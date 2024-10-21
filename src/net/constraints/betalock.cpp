#ifdef HAS_DL_MODULE

#include "net/constraints/betalock.h"

#include <utility>
#include "gui.h"

TWIN_NAMESPACE_BEGIN

    int BetaLockConstraintImpl::counter = 0;

    BetaLockConstraintImpl::BetaLockConstraintImpl() : ConstraintImpl("beta_" + std::to_string(counter++)), m_beta_sigma(3) {}

    BetaLockConstraintImpl::BetaLockConstraintImpl(const std::string &cname) : ConstraintImpl(cname + "_beta_" + std::to_string(counter)), m_torch_constraint(TorchBetaLockConstraint(cname + "_beta_" + std::to_string(counter++) + "_torch")), m_beta_sigma(3) {}

    bool BetaLockConstraintImpl::drawGUIShort() {
        ExchangeData(m_torch_constraint);
        ImGui::PushID((std::string("beta") + std::to_string(m_id)).c_str());
        ImGui::Text("Betas: Current  %f  Loss: %f ", m_value, m_loss);
        ImGui::PopID();
        return false;
    }

    void
    BetaLockConstraintImpl::drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const Ray &ray, cppgl::vec2 relative_pixel, bool pressed) {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchBetaLockConstraint(name + "_beta_" + std::to_string(m_id) + "_torch");
        }
        ExchangeData(m_torch_constraint);

        ImGui::PushID((std::string("beta") + std::to_string(m_id)).c_str());
        ImGui::InputText(_labelPrefix("Name:").c_str(), &name);
        if (ImGui::InputFloat(_labelPrefix("Weight:").c_str(), &m_weight)) {
            m_torch_constraint->m_weight.store(m_weight, std::memory_order_relaxed);
        }
        if (ImGui::InputFloat(_labelPrefix("Sigma:").c_str(), &m_beta_sigma)) {
            m_torch_constraint->m_beta_sigma.store(m_beta_sigma, std::memory_order_relaxed);
        }
        ImGui::Text("Betas: Current  %f  Loss: %f ", m_value, m_loss);
        ImGui::PopID();
    }

    void BetaLockConstraintImpl::Finalize() {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchBetaLockConstraint(name + "_beta_" + std::to_string(m_id) + "_torch");
        }
    }

    void BetaLockConstraintImpl::createDrawelement(const cppgl::Drawelement &dre) {}

    void BetaLockConstraintImpl::Serialize(nlohmann::json &json) const {
        auto &json2 = json[name];
        json2["type"] = "Beta";
        json2["betaSigma"] = m_beta_sigma;
    }

    void BetaLockConstraintImpl::Deserialize(const nlohmann::json &json, std::string cname) {
        name = cname;
        m_beta_sigma = json[cname]["betaSigma"];
        if (!m_torch_constraint) {
            m_torch_constraint = TorchBetaLockConstraint(name + "_distance_" + std::to_string(m_id) + "_torch");
        }
        ExchangeData(m_torch_constraint);
    }

    TorchConstraint BetaLockConstraintImpl::PrepareOptimImpl(ConstraintArgument &ca) {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchBetaLockConstraint(name + "_distance_" + std::to_string(m_id) + "_torch");
        }
        m_torch_constraint->m_beta_sigma.store(m_beta_sigma, std::memory_order_relaxed);
        return m_torch_constraint;
    }

    TorchConstraint BetaLockConstraintImpl::GetConstraint() {
        return m_torch_constraint;
    }

    TorchBetaLockConstraintImpl::TorchBetaLockConstraintImpl(std::string name) : TorchConstraintImpl(std::move(name)), m_beta_sigma(3) {}

    torch::Tensor TorchBetaLockConstraintImpl::compute_impl(twin::ConstraintArgument &ca) {

        torch::Tensor mean = torch::mean(torch::exp(torch::pow(ca.beta / m_beta_sigma.load(std::memory_order_relaxed), 2)), 1);

        torch::Tensor loss = m_weight.load(std::memory_order_relaxed) * mean;


        m_loss.store(loss.item().toFloat(), std::memory_order_relaxed);
        return loss;
    }

TWIN_NAMESPACE_END

#endif