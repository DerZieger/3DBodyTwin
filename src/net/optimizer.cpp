#ifdef HAS_DL_MODULE

#include "net/optimizer.h"
#include "gui.h"
#include <torch/optim.h>
#include <memory>

int twin::Optimizer::c_count = 0;
bool twin::Optimizer::c_gui = false;

twin::Optimizer::Optimizer(std::string name, std::shared_ptr<OptimizationParams> params, twin::Network net, bool use_lbfgs) : name(std::move(name)), m_net(std::move(net)), m_prepare(false), m_setup(false), m_update(false), m_full_pose(torch::zeros(1)), m_zin(torch::zeros(1)), m_trans(torch::zeros(1)), m_last_pose(m_full_pose.clone()), m_last_zin(m_zin.clone()), m_last_trans(m_trans.clone()), m_torch_constraints(std::vector<TorchConstraint>()), m_optparams(std::move(params)), m_terminate(false),
                                                                                                                              m_pause_indicator(false), m_current_loss(0), m_current_learning_rate(0), m_lbfgs(use_lbfgs), m_id(c_count++) {
    cppgl::gui_add_callback("optimizer" + std::to_string(m_id), twin::GuiCallbackWrapper<twin::Optimizer>, this);
    cppgl::gui_add_callback("optimizer", drawClassGUI);
}

twin::Optimizer::~Optimizer() {
    cppgl::gui_remove_callback("optimizer" + std::to_string(m_id));
    StopConcurrentOptimizer();
}

void twin::Optimizer::SetOptparams(std::shared_ptr<OptimizationParams> params) {
    m_optparams = std::move(params);
}

void twin::Optimizer::Prepare() {
    std::lock_guard<std::recursive_mutex> lock(m_synchronization);
    NetworkParameters inputs = m_next_parameters;

    torch::Device device = m_net->GetDevice();
    torch::Tensor tmpb = inputs->GetBeta();
    m_beta = tmpb.to(device);
    if (m_optparams->optimize_beta) {
        m_beta.requires_grad_(true);
    }
    m_net->m_vposer->to(device);

    m_full_pose = torch::zeros(1);
    m_trans = torch::zeros(1);
    m_zin = torch::zeros(1);
    using namespace torch::indexing;
    m_trans = inputs->GetTrans().to(device).requires_grad_(true);
    m_full_pose = inputs->GetPose().to(device);

    if (m_optparams->use_vposer) {
        if (m_optparams->use_vposer_rep) {//This only uses the zin for vposer from the gui for the initialisation step of the optimizer
            m_zin = inputs->GetZin();
            m_zin = m_zin.to(device).requires_grad_(true);
            m_full_pose = m_net->m_vposer->Decode(m_zin, m_full_pose, m_net->m_sex == Gender::RightHand || m_net->m_sex == Gender::LeftHand).clone().detach();
        } else {
            m_zin = m_net->m_vposer->Encode(m_full_pose).loc.clone().detach().requires_grad_(true);
            m_full_pose = m_net->m_vposer->Decode(m_zin, m_full_pose, m_net->m_sex == Gender::RightHand || m_net->m_sex == Gender::LeftHand).clone().detach();
        }
    }
    m_full_pose = m_full_pose.requires_grad_(true);

    m_last_grad = m_beta.grad();
    m_last_beta = m_beta.clone();
    m_last_pose = m_full_pose.clone();
    m_last_zin = m_zin.clone();
    m_last_trans = m_trans.clone();

    SetupOptimizer();
    UpdateConstraints();
    m_prepare.store(false, std::memory_order_relaxed);
}

void twin::Optimizer::SetupOptimizer() {
    std::lock_guard<std::recursive_mutex> lock(m_synchronization);

    std::vector<torch::Tensor> vars;
    if (m_optparams->optimize_beta) {
        m_beta.requires_grad_(true);
        vars.push_back(m_beta);
    }
    if (m_optparams->optimize_pose) {
        vars.push_back(m_trans);
        if (m_optparams->use_vposer) {
            vars.push_back(m_zin);
        }
        vars.push_back(m_full_pose);
    }

    PrepareConstraints();

    for (TorchConstraint &ps: m_torch_constraints) {
        if (ps->hasOptimizableParams()) {
            for (torch::Tensor &t: ps->OptimizableParams())
                vars.push_back(t.requires_grad_(true));
        }
    }

    if (m_lbfgs) {
        torch::optim::LBFGSOptions options;
        options.set_lr(1);
        options.max_iter(20);
        options.tolerance_change(1E-5);
        options.history_size(100);
        options.line_search_fn("strong_wolfe");
        m_optim = std::make_unique<torch::optim::LBFGS>(vars, options);
    } else {
        torch::optim::AdamOptions options;
        options.lr(m_optparams->lr);
        options.weight_decay(0.01);
        m_optim = std::make_unique<torch::optim::Adam>(vars, options);
    }
    m_scheduler = std::make_unique<ReduceLROnPlateau>(*m_optim);

    m_setup.store(false, std::memory_order_relaxed);
}

void twin::Optimizer::UpdateConstraints() {
    std::lock_guard<std::recursive_mutex> lock(m_synchronization);

    if (m_lbfgs) {
        m_optim->state().clear();
    } else if (auto *opt = dynamic_cast<torch::optim::Adam *>(m_optim.get())) {
        for (torch::optim::OptimizerParamGroup &option: opt->param_groups())
            option.options().set_lr(m_optparams->lr);
    }

    m_scheduler->reset();

    PrepareConstraints();

    m_update.store(false, std::memory_order_relaxed);
}

twin::ConstraintArgument twin::Optimizer::GetArgument() {
    torch::Device device = m_net->GetDevice();
    return ConstraintArgument{
            .device   = device,
            .beta     = m_beta,
            .theta    = m_full_pose,
            .v_shaped = m_net->VTemplate(),
            .v_posed  = m_net->VTemplate(),
            .faces    = m_net->Faces(),
            .parents  = m_net->Kintree(),
    };
}

void twin::Optimizer::PrepareConstraints() {
    m_torch_constraints.clear();
    twin::ConstraintArgument ca = GetArgument();
    if (m_net->m_constraints) {
        std::lock_guard<std::recursive_mutex> lock = m_net->m_constraints->Lock();
        for (Constraint &c: m_net->m_constraints->All()) {
            m_torch_constraints.push_back(c->PrepareOptim(ca));
        }
    }
}

torch::Tensor twin::Optimizer::step(const std::shared_ptr<OptimizationParams> &params) {
    if (m_torch_constraints.empty()) return torch::tensor(-1.f);

    using namespace torch::indexing;

    torch::Tensor loss = torch::zeros(1).to(torch::kFloat32).to(m_beta.device());


    ConstraintArgument ca = GetArgument();
    torch::Tensor new_pose = m_full_pose;
    if (params->use_vposer) {
        new_pose = m_net->m_vposer->Decode(m_zin, m_full_pose, m_net->m_sex == Gender::RightHand || m_net->m_sex == Gender::LeftHand);
        ca.theta = new_pose;
    }
    auto [verts, J_transformed, v_shaped] = m_net->VPosed(new_pose, m_trans, m_beta);

    ca.v_shaped = v_shaped;
    ca.v_posed = verts;
    ca.joints_posed = J_transformed;
    for (TorchConstraint &c: m_torch_constraints) loss += c->compute(ca);
    {
        std::lock_guard<std::mutex> guard(m_results_lock);
        //m_last_grad = m_beta.grad();
        m_last_beta = m_beta.detach();//copy_(m_beta.detach());
        m_last_pose = m_full_pose.detach();//.copy_(m_full_pose.detach());
        m_last_zin = m_zin.detach();//.copy_(m_zin.detach());
        m_last_trans = m_trans.detach();//.copy_(m_trans.detach());
    }
    return loss;
}

void twin::Optimizer::GetResults(twin::NetworkParameters &netparams) {
    std::lock_guard<std::mutex> guard(m_results_lock);

    if (m_optparams->optimize_beta) {
        if (m_last_grad.defined()) {

            torch::Tensor hgrad = m_last_grad.to(m_net->GetHost());
            netparams->m_beta_grad = std::vector<float>(hgrad.data_ptr<float>(), hgrad.data_ptr<float>() + hgrad.numel());

        }
        if (m_last_beta.defined()) {
            netparams->PutBeta(m_last_beta.detach());
        }
    }

    if (m_optparams->optimize_pose) {
        if (m_last_zin.defined()) {
            if (m_optparams->use_vposer) {
                m_last_pose = m_net->m_vposer->Decode(m_last_zin, m_last_pose, m_net->m_sex == Gender::RightHand || m_net->m_sex == Gender::LeftHand).clone().detach();
                netparams->PutZin(m_last_zin.detach());
            }
            netparams->PutPose(m_last_pose.detach());
            netparams->PutTrans(m_last_trans.detach());
            netparams->m_pose_loss = 10;  // pose_loss;
        }
    }

}

void twin::Optimizer::Run() {
    while (!m_terminate.load()) {
        float loss = 1;
        m_pausesem.signal();
        if (m_prepare.load()) {
            Prepare();
        }
        if (m_setup.load()) {
            SetupOptimizer();
        }

        if (m_update.load()) {
            UpdateConstraints();
        }
        {
            auto cost = [&]() {
                m_optim->zero_grad();
                if (m_optparams->use_vposer)
                    m_net->m_vposer->zero_grad();
                torch::Tensor loss = step(m_optparams);
                if (loss.item().toFloat() == -1.0f) {//Nothing to optimize
                    return loss;
                }

                loss.backward();

                m_current_loss = loss.item().toFloat();
                m_current_learning_rate = m_scheduler->get_last_lrf();
                if (auto opt = dynamic_cast<torch::optim::LBFGS *>(m_optim.get()))
                    m_current_learning_rate = static_cast<float>(opt->param_groups().at(0).options().get_lr());
                return loss;
            };
            if (m_lbfgs) {
                t_loss = m_optim->step(cost);
                loss = t_loss.item().toFloat();
            } else {
                t_loss = cost();
                loss = t_loss.item().toFloat();
                m_optim->step();
                m_scheduler->step(loss);
            }
        }
        m_pausesem.wait();
    }
}

void twin::Optimizer::OptimizeOneShot() {
    int number_iterations = 0;
    cppgl::Timer timer;
    int iter = 0;
    float prev_loss = 1000;
    float my_loss = prev_loss - 1;
    for (iter = 0; iter < m_optparams->minimum_iterations || (iter < m_optparams->iterations && timer.look() < m_optparams->time_budget_seconds * 1'000 && (abs(prev_loss - my_loss) > m_optparams->tolerance_threshold || prev_loss > 2) && prev_loss > m_optparams->loss_threshold); ++iter, timer.begin(), ++number_iterations) {
        float loss = 1;
        m_pausesem.signal();
        if (m_prepare.load()) Prepare();
        if (m_setup.load()) SetupOptimizer();
        if (m_update.load()) UpdateConstraints();
        {
            auto cost = [&]() {
                m_optim->zero_grad();
                torch::Tensor loss = step(m_optparams);
                if (loss.item().toFloat() == -1.0f) {
                    return loss;
                }

                loss.backward();
                m_current_loss = loss.item().toFloat();
                m_current_learning_rate = m_scheduler->get_last_lrf();
                if (auto opt = dynamic_cast<torch::optim::LBFGS *>(m_optim.get()))
                    m_current_learning_rate = static_cast<float>(opt->param_groups().at(0).options().get_lr());
                return loss;
            };
            if (m_lbfgs) {
                loss = m_optim->step(cost).item().toFloat();
            } else {
                loss = cost().item().toFloat();
                m_optim->step();
                m_scheduler->step(loss);
            }
        }
    }
}

void twin::Optimizer::Terminate() {
    m_terminate.store(true);
    Unpause();
}

void twin::Optimizer::Pause() {
    if (m_optim) {
        m_pausesem.wait();
        m_pause_indicator.store(true);
    }
}

void twin::Optimizer::Unpause() {
    if (m_optim) {
        m_pausesem.signal();
        m_pause_indicator.store(false);
    }
}

void twin::Optimizer::QueuePrepare(twin::NetworkParameters params) {
    std::lock_guard<std::recursive_mutex> lock(m_synchronization);
    m_prepare.store(true, std::memory_order_relaxed);
    m_next_parameters = std::move(params);
}

void twin::Optimizer::QueueSetup() {
    m_setup.store(true, std::memory_order_relaxed);
}

void twin::Optimizer::QueueUpdate() {
    m_update.store(true, std::memory_order_relaxed);
}

void twin::Optimizer::StartConcurrentOptimizer() {
    if (m_optim) {
        StopConcurrentOptimizer();
    }
    if (!m_net->VposerPossible()) {
        m_optparams->use_vposer = false;
        m_optparams->use_vposer_rep = false;
    }
    m_lbfgs = m_optparams->use_lbfgs;
    QueuePrepare(m_next_parameters);
    Prepare();
    m_terminate.store(false);
    m_concurrent_optimizer = std::make_unique<std::thread>([this]() { Run(); });
}

void twin::Optimizer::StopConcurrentOptimizer() {
    Terminate();
    if (m_optim) {
        if (m_concurrent_optimizer)
            m_concurrent_optimizer->join();
        m_optim.reset();
        m_concurrent_optimizer.reset();
    }
    m_current_learning_rate = 0;
    m_current_loss = 0;
}

void
twin::Optimizer::UpdateInputs(twin::NetworkParameters inputs, std::shared_ptr<OptimizationParams> params, bool forceSet) {
    if (m_optim || forceSet) {
        SetOptparams(std::move(params));
        QueuePrepare(std::move(inputs));
    }
}

void twin::Optimizer::UpdateResetConstraints() {
    if (!m_optim) {
        return;
    }
    ConstraintImpl::Change change = m_net->m_constraints->GetChange();
    if (change == ConstraintImpl::Change::ParametersRestart) {
        QueueSetup();
    } else if (change == ConstraintImpl::Change::ParametersReconfigure) {
        QueueUpdate();
    }
    m_net->m_constraints->Updated();
}

void twin::Optimizer::UpdateParams(twin::NetworkParameters &inputs) {
    if (m_optim) {
        GetResults(inputs);
    }
}

bool twin::Optimizer::Update(twin::NetworkParameters &params) {
    if (m_optim && !m_pause_indicator) {
        UpdateResetConstraints();
        UpdateParams(params);
        return true;
    }
    return false;
}

void twin::Optimizer::drawGUI() {
    if (c_gui) {
        if (ImGui::Begin(std::string("Optimizer##").c_str(), &c_gui)) {
            ImGui::PushID("Optimizer");
            ImGui::PushID(m_id);
            if (ImGui::CollapsingHeader(std::to_string(m_id).c_str())) {
                ImGui::Indent(3);
                bool resetup_optimizer = false;

                if (ImGui::CollapsingHeader("Optimization Parameter##")) {
                    ImGui::Indent(2);
                    ImGui::Text("Loss %f ", m_optparams->last_loss);
                    ImGui::SameLine();
                    if (ImGui::Button("Reset")) {
                        m_optparams->last_loss = 1E10;
                    }

                    ImGui::InputInt("Minimum Iterations", &m_optparams->minimum_iterations, 1, 20);
                    ImGui::InputFloat("LR", &m_optparams->lr);
                    resetup_optimizer |= ImGui::Checkbox("Optimize Pose", &m_optparams->optimize_pose);
                    resetup_optimizer |= ImGui::Checkbox("Optimize Beta", &m_optparams->optimize_beta);
                    ImGui::Checkbox("Use Vposer", &m_optparams->use_vposer);

                    ImGui::Checkbox("Use VposerRep", &m_optparams->use_vposer_rep);


                    if (ImGui::CollapsingHeader("Budget")) {
                        ImGui::InputFloat("Tol", &m_optparams->tolerance_threshold);
                        ImGui::InputFloat("Time", &m_optparams->time_budget_seconds);
                        ImGui::InputFloat("Loss", &m_optparams->loss_threshold);
                        ImGui::InputInt("Iter", &m_optparams->iterations);
                    }
                    ImGui::Unindent(2);
                }


                if (ImGui::CollapsingHeader("Concurrent optimizer")) {
                    ImGui::Indent(2);
                    if (m_optim) {
                        if (resetup_optimizer) UpdateResetConstraints();

                        if (!m_pause_indicator) {
                            if (ImGui::Button("Pause")) Pause();
                        } else {
                            if (ImGui::Button("Unpause")) Unpause();
                        }
                        if (ImGui::Button("Terminate"))
                            StopConcurrentOptimizer();
                        else {
                            if (!m_pause_indicator) {
                                m_losses.push_back(m_current_loss);
                                m_learning_rates.push_back(m_current_learning_rate);
                                if (m_losses.size() > 100) {
                                    m_losses.erase(m_losses.begin());
                                    m_learning_rates.erase(m_learning_rates.begin());
                                }
                                int offset = 0;
                                ImGui::PlotLines("Loss", m_losses.data(), static_cast<int>(m_losses.size()), offset, nullptr, 0, 5, ImVec2(0, 80.0f));
                                ImGui::PlotLines("LR", m_learning_rates.data(), static_cast<int>(m_learning_rates.size()), offset, nullptr, 0, 1E-2, ImVec2(0, 80.0f));
                            }
                        }
                    } else {
                        int optimizer = m_optparams->use_lbfgs ? 0 : 1;
                        ImGui::Combo("Optimizer", &optimizer, "LBFGS\000Adam\000");
                        m_optparams->use_lbfgs = optimizer == 0 ? 1 : 0;
                        if (ImGui::Button("StartConcurrentOptimizer"))
                            StartConcurrentOptimizer();
                    }
                    ImGui::Unindent(2);
                }
                ImGui::Unindent(3);
            }
            ImGui::PopID();
            ImGui::PopID();
        }
        ImGui::End();
    }
}

void twin::Optimizer::drawClassGUI() {
    if (cppgl::Context::show_gui) {
        if (ImGui::BeginMainMenuBar()) {
            ImGui::PushID("Optimizer");
            ImGui::Separator();
            ImGui::Checkbox("Optimizer", &c_gui);
            ImGui::PopID();
            ImGui::EndMainMenuBar();
        }
    }
}

#endif