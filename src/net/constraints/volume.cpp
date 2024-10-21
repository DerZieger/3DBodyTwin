#ifdef HAS_DL_MODULE

#include "net/constraints/volume.h"

#include <utility>
#include "gui.h"

TWIN_NAMESPACE_BEGIN

    int VolumeConstraintImpl::counter = 0;

    VolumeConstraintImpl::VolumeConstraintImpl() : ConstraintImpl("volume_" + std::to_string(counter++)), m_density(1024.f) {}

    VolumeConstraintImpl::VolumeConstraintImpl(const std::string &cname) : ConstraintImpl(cname + "_volume_" + std::to_string(counter)), m_density(1024.f), m_torch_constraint(TorchVolumeConstraint(cname + "_volume_" + std::to_string(counter++) + "_torch")) {}

    VolumeConstraintImpl::VolumeConstraintImpl(const std::string &cname, float mass) : ConstraintImpl(cname + "_volume_" + std::to_string(counter)), m_density(985.f), m_torch_constraint(TorchVolumeConstraint(cname + "_volume_" + std::to_string(counter++) + "_torch")) {
        m_target = mass / m_density;
    }

    bool VolumeConstraintImpl::drawGUIShort() {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchVolumeConstraint(name + "_volume_" + std::to_string(m_id) + "_torch");
        }
        ExchangeData(m_torch_constraint);
        ImGui::Text("V: %.3f, L: %.3f", m_value, m_loss);
        return false;
    }

    void VolumeConstraintImpl::drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const Ray &ray, cppgl::vec2 relative_pixel, bool pressed) {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchVolumeConstraint(name + "_volume_" + std::to_string(m_id) + "_torch");
        }
        ExchangeData(m_torch_constraint);
        ImGui::PushID((std::string("volume") + std::to_string(m_id)).c_str());
        ImGui::InputText(_labelPrefix("Name:").c_str(), &name);
        float mass = m_target * m_density;
        ImGui::InputFloat(_labelPrefix("Mass[kg]:").c_str(), &mass);
        m_target = mass / m_density;
        ImGui::InputFloat(_labelPrefix("Density[kg/m^3]:").c_str(), &m_density);
        ImGui::InputFloat(_labelPrefix("Weight:").c_str(), &m_weight);
        ImGui::Text("Volume: %f", m_target);

        ImGui::Text("Current Value: %f (%f), Loss: %f ", m_value, m_target, m_loss);
        ImGui::Text("Current Mass: %f (%f)", m_value * m_density, m_target * m_density);
        ImGui::PopID();
        ExchangeData(m_torch_constraint);
    }

    void VolumeConstraintImpl::Finalize() {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchVolumeConstraint(name + "_volume_" + std::to_string(m_id) + "_torch");
        }
    }

    void VolumeConstraintImpl::createDrawelement(const cppgl::Drawelement &dre) {}

    void VolumeConstraintImpl::Serialize(nlohmann::json &json) const {
        auto &json2 = json[name];
        json2["type"] = "Volume";
        json2["density"] = m_density;
        json2["target"] = m_target;
        json2["weight"] = m_weight;
    }

    void VolumeConstraintImpl::Deserialize(const nlohmann::json &json, std::string cname) {
        name = cname;
        m_density = json[name]["density"];
        m_target = json[name]["target"];
        m_weight = json[name]["weight"];
        if (!m_torch_constraint) {
            m_torch_constraint = TorchVolumeConstraint(name + "_volume_" + std::to_string(m_id) + "_torch");
        }
        ExchangeData(m_torch_constraint);
    }

    bool VolumeConstraintImpl::UsesTarget() {
        if (ImGui::SliderFloat("", &m_target, 0.0, 0.2))
            SetChange(ConstraintImpl::Change::ParametersReconfigure);
        return true;
    }

    TorchConstraint VolumeConstraintImpl::PrepareOptimImpl(ConstraintArgument &ca) {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchVolumeConstraint(name + "_volume_" + std::to_string(m_id) + "_torch");
        }
        m_torch_constraint->m_density.store(m_density, std::memory_order_relaxed);
        return m_torch_constraint;
    }

    TorchConstraint VolumeConstraintImpl::GetConstraint() {
        return m_torch_constraint;
    }

    void VolumeConstraintImpl::ExchangeData(TorchVolumeConstraint con) {
        ConstraintImpl::ExchangeData(std::move(con));
        m_torch_constraint->m_density.store(m_density, std::memory_order_relaxed);
    }

    TorchVolumeConstraintImpl::TorchVolumeConstraintImpl(std::string name) : TorchConstraintImpl(std::move(name)), m_density(985.f) {}

    torch::Tensor TorchVolumeConstraintImpl::compute_impl(ConstraintArgument &ca) {
        using namespace torch::indexing;
        // verts: BxFx3x3
        Slice s = Slice();
        torch::Tensor verts = ca.v_shaped.index({s, ca.faces});
        torch::Tensor volumes = 1.f / 6.f *
                                (-verts.index({s, s, 2, 0}) * verts.index({s, s, 1, 1}) * verts.index({s, s, 0, 2}) +
                                 verts.index({s, s, 1, 0}) * verts.index({s, s, 2, 1}) * verts.index({s, s, 0, 2}) +
                                 verts.index({s, s, 2, 0}) * verts.index({s, s, 0, 1}) * verts.index({s, s, 1, 2}) -
                                 verts.index({s, s, 0, 0}) * verts.index({s, s, 2, 1}) * verts.index({s, s, 1, 2}) -
                                 verts.index({s, s, 1, 0}) * verts.index({s, s, 0, 1}) * verts.index({s, s, 2, 2}) +
                                 verts.index({s, s, 0, 0}) * verts.index({s, s, 1, 1}) * verts.index({s, s, 2, 2}));
        torch::Tensor volume_batch = torch::sum(volumes, -1);
        torch::Tensor loss = m_weight.load(std::memory_order_relaxed) * torch::abs(volume_batch - m_target.load(std::memory_order_relaxed)).index({0});

        m_loss.store(loss.item().toFloat(), std::memory_order_relaxed);
        m_value.store(volume_batch.item().toFloat(), std::memory_order_relaxed);
        return loss;
    }


TWIN_NAMESPACE_END

#endif