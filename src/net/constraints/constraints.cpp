#ifdef HAS_DL_MODULE

#include "net/constraints/constraints.h"

int twin::ConstraintsImpl::c_counter = 0;

twin::ConstraintsImpl::ConstraintsImpl(const std::string& na) : name(na + std::to_string(c_counter)), m_id(c_counter++) {}

void twin::ConstraintsImpl::Updated() {
    m_change = Change::None;
}

void twin::ConstraintsImpl::Update(twin::ConstraintsImpl::Change change) {
    if (change < m_change)
        m_change = change;
}

void twin::ConstraintsImpl::Remove(long unsigned int index) {
    std::lock_guard<std::recursive_mutex> lock = Lock();
    if (index >= m_constraints.size()) return;
    Constraint con = *(m_constraints.begin() + static_cast<long>(index));
    Update(con->Deregister(name));
    m_constraints.erase(m_constraints.begin() + static_cast<long>(index));
}

void twin::ConstraintsImpl::Remove(const std::string &cname) {
    std::lock_guard<std::recursive_mutex> lock = Lock();
    for (long unsigned int i = 0; i < m_constraints.size(); ++i) {
        if (m_constraints.at(i)->name == cname) {
            Remove(i);
            break;
        }
    }
}

void twin::ConstraintsImpl::Add(twin::Constraint con) {
    std::lock_guard<std::recursive_mutex> lock = Lock();
    m_constraints.push_back(con);
    Update(con->Register(name));//Use the name to find named handle in constraint
}

void twin::ConstraintsImpl::Clear() {
    std::lock_guard<std::recursive_mutex> lock = Lock();
    m_constraints.clear();
    Updated();
}

std::lock_guard<std::recursive_mutex> twin::ConstraintsImpl::Lock() {
    return std::lock_guard(m_lock);
}

std::vector<twin::Constraint> &twin::ConstraintsImpl::All() {
    return m_constraints;
}

const std::vector<twin::Constraint> &twin::ConstraintsImpl::All() const {
    return m_constraints;
}

twin::ConstraintsImpl::Change twin::ConstraintsImpl::GetChange() {
    return m_change;
}

bool twin::ConstraintsImpl::NeedsUpdate() {
    return m_change == Change::None;
}

void twin::ConstraintsImpl::render() const {
    for (const Constraint &con: m_constraints) {
        con->render();
    }
}

#endif