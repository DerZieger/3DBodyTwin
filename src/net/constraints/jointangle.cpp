#ifdef HAS_DL_MODULE

#include "net/constraints/jointangle.h"

#include <utility>
#include "gui.h"

TWIN_NAMESPACE_BEGIN

    int AngleConstraintImpl::counter = 0;

    AngleConstraintImpl::AngleConstraintImpl() : ConstraintImpl("angle_" + std::to_string(counter++)) {}

    AngleConstraintImpl::AngleConstraintImpl(const std::string &cname) : ConstraintImpl(cname + "_angle_" + std::to_string(counter)), m_torch_constraint(TorchAngleLockConstraint(cname + "_angle_" + std::to_string(counter++) + "_torch")) {}

    bool AngleConstraintImpl::drawGUIShort() {
        ExchangeData(m_torch_constraint);
        ImGui::PushID((std::string("angle") + std::to_string(m_id)).c_str());
        ImGui::Text("Loss: %f ", m_loss);
        ImGui::PopID();
        return false;
    }

    void AngleConstraintImpl::drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const Ray &ray, cppgl::vec2 relative_pixel, bool pressed) {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchAngleLockConstraint(name + "_angle_" + std::to_string(m_id) + "_torch");
        }
        bool change = false;
        ExchangeData(m_torch_constraint);

        ImGui::PushID((std::string("angle") + std::to_string(m_id)).c_str());
        ImGui::InputText(_labelPrefix("Name:").c_str(), &name);
        if (ImGui::InputFloat(_labelPrefix("Weight:").c_str(), &m_weight)) {
            m_torch_constraint->m_weight.store(m_weight, std::memory_order_relaxed);
        }

        if (ImGui::CollapsingHeader("Angles")) {
            if (ImGui::Button("Add new joint")) {
                m_joints.push_back(0);
                m_active.push_back(0);
                m_lbound.push_back(-M_PI);
                m_hbound.push_back(M_PI);
            }
            for (uint64_t i = 0; i < m_joints.size(); ++i) {
                ImGui::PushID(static_cast<int>(i));
                bool t = m_active.at(i) == 1;
                if (ImGui::Checkbox("##", &t)) {
                    m_active.at(i) = t ? 1 : 0;
                    change = true;
                }
                ImGui::SameLine();
                ImGui::Text("Joint:");
                ImGui::SameLine();
                ImGui::PushItemWidth(50);
                ImGui::InputInt(" Bounds:", &m_joints.at(i), 0);
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::PushItemWidth(250);
                change |= ImGui::CustomSliderFloat2("##1", &m_lbound.at(i), &m_hbound.at(i), -M_PI, M_PI);
                ImGui::PopItemWidth();
                ImGui::PopID();
            }
        }
        if (change)
            SetChange(Change::ParametersReconfigure);
        ImGui::Text("Angle: Loss: %f ", m_loss);
        ImGui::PopID();
    }

    void AngleConstraintImpl::Finalize() {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchAngleLockConstraint(name + "_angle_" + std::to_string(m_id) + "_torch");
        }
    }

    void AngleConstraintImpl::createDrawelement(const cppgl::Drawelement &dre) {}

    void AngleConstraintImpl::Serialize(nlohmann::json &json) const {
        nlohmann::json &json2 = json[name];
        json2["type"] = "Angle";
        json2["joints"] = m_joints;
        json2["lbounds"] = m_lbound;
        json2["upbounds"] = m_hbound;
    }

    void AngleConstraintImpl::Deserialize(const nlohmann::json &json, std::string cname) {
        name = cname;
        const nlohmann::json &con = json[name];
        m_hbound = con["upbounds"].get<std::vector<float>>();
        m_lbound = con["lbounds"].get<std::vector<float>>();
        m_joints = con["joints"].get<std::vector<int>>();
        m_active.resize(m_joints.size());
        std::fill(m_active.begin(), m_active.end(), 1);
        ExchangeData(m_torch_constraint);
    }

    TorchConstraint AngleConstraintImpl::PrepareOptimImpl(ConstraintArgument &ca) {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchAngleLockConstraint(name + "_distance_" + std::to_string(m_id) + "_torch");
        }
        std::lock_guard<std::mutex> g(m_torch_constraint->m_lock);
        m_torch_constraint->m_hboundaries = torch::from_blob(m_hbound.data(), static_cast<int>(m_hbound.size()), torch::kFloat32).to(ca.device);
        m_torch_constraint->m_lboundaries = torch::from_blob(m_lbound.data(), static_cast<int>(m_lbound.size()), torch::kFloat32).to(ca.device);
        m_torch_constraint->m_joints = torch::from_blob(m_joints.data(), static_cast<int>(m_joints.size()), torch::kInt32).to(ca.device);
        m_torch_constraint->m_active = torch::from_blob(m_active.data(), static_cast<int>(m_active.size()), torch::kInt32).to(ca.device);

        return m_torch_constraint;
    }

    TorchConstraint AngleConstraintImpl::GetConstraint() {
        return m_torch_constraint;
    }

    TorchAngleConstraintImpl::TorchAngleConstraintImpl(std::string name) : TorchConstraintImpl(std::move(name)) {}

    torch::Tensor TorchAngleConstraintImpl::compute_impl(twin::ConstraintArgument &ca) {
        using namespace torch::indexing;
        torch::Tensor js = ca.theta.index({0, m_joints});
        torch::Tensor err = torch::where(js > m_hboundaries, torch::pow(js - m_hboundaries, 4) * 2, 0);
        err = torch::where(js < m_lboundaries, torch::pow(js + m_lboundaries, 4) * 2, err);
        err *= m_active;
        torch::Tensor s = torch::sum(err);
        torch::Tensor loss = m_weight.load(std::memory_order_relaxed) * s;
        m_loss.store(loss.item().toFloat(), std::memory_order_relaxed);
        return loss;
    }

TWIN_NAMESPACE_END

#endif
