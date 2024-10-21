#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_SCHEDULER_H
#define TWIN_SCHEDULER_H

#include "twinDefines.h"
#include <torch/optim/schedulers/lr_scheduler.h>

TWIN_NAMESPACE_BEGIN
    class ReduceLROnPlateau : public torch::optim::LRScheduler {
    public:
        ReduceLROnPlateau(torch::optim::Optimizer &optimizer, const double gamma = 0.1, int patience = 25, float threshold = 1e-3);

        void step(float loss);

        void reset();

        double get_last_lr() const;

        float get_last_lrf() const;

    protected:
        std::vector<double> get_lrs() override;

        const double m_gamma;
        const int m_patience;
        const float m_threshold;
        int m_number_bad_iters;
        float m_best;
        double m_last_lr;
        bool m_reduce_next;
    };

TWIN_NAMESPACE_END
#endif //TWIN_SCHEDULER_H
#endif