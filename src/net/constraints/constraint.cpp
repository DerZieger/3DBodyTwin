#ifdef HAS_DL_MODULE

#include "net/constraints/constraints.h"
#include "net/constraints/constraint.h"
#include <glog/logging.h>

twin::TorchConstraintImpl::TorchConstraintImpl(std::string name) : name(name + "_" + std::to_string(id_counter++)), m_weight(1.f), m_loss(1.f), m_value(0.f), m_target(0.f) {}

twin::TorchConstraintImpl::~TorchConstraintImpl() = default;

torch::Tensor twin::TorchConstraintImpl::compute(twin::ConstraintArgument &ca) {

    std::lock_guard<std::mutex> guard(m_lock);
    return compute_impl(ca);

}

bool twin::TorchConstraintImpl::hasOptimizableParams() const {
    return false;
}

std::vector<torch::Tensor> twin::TorchConstraintImpl::OptimizableParams() {
    return {};
}

twin::ConstraintImpl::ConstraintImpl() : ConstraintImpl(std::to_string(id_counter)) {}

twin::ConstraintImpl::ConstraintImpl(std::string name) : name(std::move(name)), m_model(cppgl::mat4::Zero()), m_id(++id_counter), m_target(0), m_recompute_rendering(false), m_weight(1), m_loss(1), m_value(0), m_frameid(-1), m_last_prepared_frameid(-2), m_change(Change::ParametersReconfigure) {}

twin::ConstraintImpl::~ConstraintImpl() = default;

void twin::ConstraintImpl::UpdateReferencePose(int frameid) {
    if (m_frameid != frameid) {
        m_frameid = frameid;
        SetChange(OnFrameChange());
    }
    m_frameid = frameid;
}

void twin::ConstraintImpl::SetModel(cppgl::mat4 model) {
    m_model = std::move(model);
}

twin::TorchConstraint twin::ConstraintImpl::PrepareOptim(twin::ConstraintArgument &ca) {
    if (HasChange()) {
        ClearChange();
        TorchConstraint ret = PrepareOptimImpl(ca);
        m_last_prepared_frameid = m_frameid;
        return ret;
    } else {
        return GetConstraint();
    }
}

twin::ConstraintImpl::Change twin::ConstraintImpl::Register(const std::string &cname) {
    if (Constraints::valid(cname)) {
        m_constraints.push_back(std::weak_ptr(Constraints::find(cname).ptr));
    } else {
        LOG(ERROR) << "Failed to register constraint to constraints " << name << "; " << name << " is not valid name of constraints" << std::endl;
    }
    return OnRegistrationChange();
}

twin::ConstraintImpl::Change twin::ConstraintImpl::Deregister(const std::string &cname) {

    if (Constraints::valid(cname)) {
        ConstraintsImpl *ptr = Constraints::find(cname).ptr.get();
        m_constraints.erase(std::find_if(m_constraints.begin(), m_constraints.end(), [ptr](auto c) { return c.lock().get() == ptr; }));
    } else {
        LOG(ERROR) << "Failed to deregister constraint to constraints " << name << "; " << name << " is not valid name of constraints" << std::endl;
    }
    return OnRegistrationChange();
}

bool twin::ConstraintImpl::NeedRecomputeRenderObjects() const {
    return m_recompute_rendering;
}

bool twin::ConstraintImpl::UsesTarget() {
    ImGui::Spacing();
    return false;
}

int twin::ConstraintImpl::getZeroFrame() const {
    return -1;
}

int twin::ConstraintImpl::getFramerate() const {
    return -1;
}

twin::ConstraintImpl::Change twin::ConstraintImpl::OnFrameChange() {
    return m_change;//Change::None;//Doesn't do what name implies, but can be used for that
}

twin::ConstraintImpl::Change twin::ConstraintImpl::OnRegistrationChange() {
    return Change::ParametersReconfigure;
}

void twin::ConstraintImpl::ExchangeData(twin::TorchConstraint con) {
    m_value = con->m_value.load(std::memory_order_relaxed);
    m_loss = con->m_loss.load(std::memory_order_relaxed);
    con->m_weight.store(m_weight, std::memory_order_relaxed);
    con->m_target.store(m_target, std::memory_order_relaxed);
}

void twin::ConstraintImpl::SetChange(twin::ConstraintImpl::Change change) {
    m_change = change;
    Notify(change);
}

void twin::ConstraintImpl::ClearChange() {
    m_change = Change::None;
}

twin::ConstraintImpl::Change twin::ConstraintImpl::GetChange() const {
    return m_change;
}

bool twin::ConstraintImpl::HasChange() const {
    return m_change != Change::None;
}

void twin::ConstraintImpl::Notify(twin::ConstraintImpl::Change c) const {
    for (const std::weak_ptr<ConstraintsImpl> &con: m_constraints) {
        if (std::shared_ptr<ConstraintsImpl> cc = con.lock()) {
            cc->Update(c);
        }
    }
}

void twin::ConstraintImpl::render() const {
    if (m_consRepr) {
        m_consRepr->bind();
        m_consRepr->draw();
        m_consRepr->unbind();
    }
}

bool twin::ConstraintImpl::isReferenceSet() const {
    return true;
}


#endif