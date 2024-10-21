#ifdef HAS_DL_MODULE

#include "net/scheduler.h"


twin::ReduceLROnPlateau::ReduceLROnPlateau(torch::optim::Optimizer &optimizer, const double gamma, int patience, float threshold) : LRScheduler(optimizer), m_gamma(gamma), m_patience(patience), m_threshold(threshold), m_number_bad_iters(0), m_best(1E38f), m_last_lr(0), m_reduce_next(false) {}

void twin::ReduceLROnPlateau::step(float loss) {
    torch::optim::LRScheduler::step();

    if (loss < m_best * (1 - m_threshold)) {
        m_best = loss;
        m_number_bad_iters = 0;
    } else {
        ++m_number_bad_iters;
    }
    if (m_number_bad_iters > m_patience) {
        m_reduce_next = true;
        m_number_bad_iters = 0;
    }
}

void twin::ReduceLROnPlateau::reset() {
    m_number_bad_iters = 0;
    m_best = 1E38f;
    m_reduce_next = false;
    m_last_lr = 0;
}

double twin::ReduceLROnPlateau::get_last_lr() const {
    return m_last_lr;
}

float twin::ReduceLROnPlateau::get_last_lrf() const {
    return static_cast<float>(m_last_lr);
}

std::vector<double> twin::ReduceLROnPlateau::get_lrs() {

    std::vector<double> ret = get_current_lrs();
    m_last_lr = ret[0];
    if (m_reduce_next) {
        m_reduce_next = false;
        for (double &v: ret) v *= m_gamma;
    }
    return ret;

}

#endif