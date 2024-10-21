#ifdef HAS_DL_MODULE

#include "net/supr/supr.h"
#include <iostream>
#include "glog/logging.h"
#include "gui.h"
#include <nlohmann/json.hpp>

bool twin::SUPRImpl::c_gui = false;

twin::SUPRImpl::SUPRImpl(const std::string &cname, const std::string &pathname, Gender s) : NetworkImpl(cname, pathname, s),m_num_verts(0),m_constrained(false),m_constposs(false),m_numpose(0) {

    try {
        params = SuprParameters(cname);
        LoadGender(pathname, s);
        cppgl::gui_add_callback("supr" + std::to_string(m_id), twin::GuiCallbackWrapper<twin::SUPRImpl>, this);
        cppgl::gui_add_callback("supr", drawClassGUI);
    } catch (const c10::Error &e) {
        LOG(FATAL) << "error loading the model: " << pathname << " with filename \"supr_{gender}\".npz" << std::endl;
    }
}

twin::SUPRImpl::~SUPRImpl(){
    cppgl::gui_remove_callback("supr" + std::to_string(m_id));
}


std::tuple<torch::Tensor, torch::Tensor,torch::Tensor>twin::SUPRImpl::VPosed(torch::Tensor full_pose, torch::Tensor trans, torch::Tensor beta) {
    using namespace torch::indexing;
    SUPRInferred inf=sp->forward(full_pose,beta,trans);

    return {inf.v_final, inf.J_transformed,inf.v_shaped};
}

torch::Device twin::SUPRImpl::GetDevice() const{
    return torch::kCUDA;
}

torch::Device twin::SUPRImpl::GetHost() const{
    return torch::kCPU;
}

torch::Tensor twin::SUPRImpl::VTemplate() {
    return m_bufs["v_template"];
}

torch::Tensor twin::SUPRImpl::Shapedirs() {
    return m_bufs["shapedirs"];
}

torch::Tensor twin::SUPRImpl::Faces() {
    return m_bufs["faces"];
}

torch::Tensor twin::SUPRImpl::JReg() {
    return m_bufs["J_regressor"];
}

torch::Tensor twin::SUPRImpl::Kintree() {
    return m_bufs["kintree_table"];
}

torch::Tensor twin::SUPRImpl::Weights() {
    return m_bufs["weights"];
}

torch::Tensor twin::SUPRImpl::Posedirs() {
    return m_bufs["posedirs"];
}

std::vector<long> twin::SUPRImpl::Parents() {
    return sp->getParentVec();
}


void twin::SUPRImpl::Inference() {
    NetworkImpl::Inference(params);
}


void twin::SUPRImpl::drawGUI() {
    if (c_gui) {
        if (ImGui::Begin(std::string("SUPR").c_str(), &c_gui)) {
            ImGui::PushID("Supr");
            ImGui::PushID(m_id);
            if (ImGui::CollapsingHeader(std::to_string(m_id).c_str())) {
                NetworkImpl::drawGUI();
                ImGui::Indent(2);
                if (m_constposs) {
                    if (ImGui::Checkbox("Use constrained model", &m_constrained)) {
                        LoadGender(m_path, m_sex, !m_constrained);
                    }
                }
                ImGui::Unindent(2);
                params->drawGUI();
            }
            ImGui::PopID();
            ImGui::PopID();
        }
        ImGui::End();
    }

}

void twin::SUPRImpl::Inference(twin::NetworkParameters &p) {
    if (dynamic_cast<twin::SuprParametersImpl *>(p.ptr.get()) != nullptr) {
        NetworkImpl::Inference(p);
    } else {
        LOG(FATAL) << "Wrong networkparameter type given to inference" << std::endl;
    }
}

void twin::SUPRImpl::LoadGender(const std::string &path, twin::Gender s) {
    LoadGender(path, s, true);
}

void twin::SUPRImpl::LoadGender(const std::string &path, twin::Gender s, bool forceUnconstrained) {
    m_obj.free();
    m_mesh.free();
    m_mesh_shaped.free();
    m_obj_skeleton.free();
    m_obj_joints.free();
    if (s < 3) {
        params->setType(ModelType::human);
    } else if (s > 5) {
        params->setType(ModelType::head);
    } else {
        params->setType(ModelType::hand);
    }
    std::string g;
    if (s == Gender::Neutral) {
        m_constrained = std::filesystem::exists(path + "/supr_neutral_constrained.json");
        g = m_constrained ? "/supr_neutral_constrained.npz" : "/supr_neutral.npz";
    } else if (s == Gender::Male) {
        m_constrained = std::filesystem::exists(path + "/supr_male_constrained.json");
        g = m_constrained ? "/supr_male_constrained.npz" : "/supr_male.npz";
    } else if (s == Gender::Female) {
        m_constrained = std::filesystem::exists(path + "/supr_female_constrained.json");
        g = m_constrained ? "/supr_female_constrained.npz" : "/supr_female.npz";
    } else if (s == Gender::LeftHand) {
        m_constrained = std::filesystem::exists(path + "/supr_hand_left_constrained.json");
        g = m_constrained ? "/supr_hand_left_constrained.npz" : "/supr_hand_left.npz";
    } else if (s == Gender::RightHand) {
        m_constrained = std::filesystem::exists(path + "/supr_hand_right.json");
        g = m_constrained ? "/supr_hand_right_constrained.npz" : "/supr_hand_right.npz";
    }
    m_constposs = m_constrained;
    if (forceUnconstrained) {
        m_constrained = false;
    }
    params->setConstrained(m_constrained);
    m_sex = s;
    m_filename = path + g;
    sp=std::make_shared<SUPRCPP>(m_filename,params->BetaEntries(),m_constrained);
    sp->to(torch::kCUDA);
    m_bufs=sp->named_buffers();
    params->init();
}

void twin::SUPRImpl::drawClassGUI() {
    if (cppgl::Context::show_gui) {
        if (ImGui::BeginMainMenuBar()) {
            ImGui::PushID("Supr");
            ImGui::Separator();
            ImGui::Checkbox("SUPR", &c_gui);
            ImGui::PopID();
            ImGui::EndMainMenuBar();
        }
    }
}

bool twin::SUPRImpl::VposerPossible() const {
    return (NetworkImpl::VposerPossible()) && !m_constrained;
}

void twin::SUPRImpl::to(const torch::Device &d){
    sp->to(d);
}

#endif