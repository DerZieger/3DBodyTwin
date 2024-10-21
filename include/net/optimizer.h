#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_OPTIMIZER_H
#define TWIN_OPTIMIZER_H

#include "twinDefines.h"
#include <cppgl.h>
#include "net/network.h"
#include "countingSemaphore.h"
#include <torch/optim/optimizer.h>
#include "net/scheduler.h"

TWIN_NAMESPACE_BEGIN

    struct OptimizationParams {
        float loss_threshold = 1E-10;
        float tolerance_threshold = 1E-5;
        float time_budget_seconds = 1.f / 60.f;
        int minimum_iterations = 10;
        int iterations = 1000;
        float lr = 1E-3;
        bool optimize_pose = true;
        bool optimize_beta = true;
        bool optimize_markers = false;
        bool use_vposer = true;
        bool use_vposer_rep = true;

        bool use_lbfgs = false;

        bool continuous_optimization = false;

        float last_loss = 1E10;
    };

    class Optimizer {
    public:
        std::string name;

        explicit Optimizer(std::string name, std::shared_ptr<OptimizationParams> params, Network net, bool use_lbfgs = true);

        ~Optimizer();

        void SetOptparams(std::shared_ptr<OptimizationParams> params);

        void Prepare();

        void SetupOptimizer();

        void UpdateConstraints();

        ConstraintArgument GetArgument();

        void PrepareConstraints();

        torch::Tensor step(const std::shared_ptr<OptimizationParams> &params);

        void GetResults(NetworkParameters &netparams);

        void Run();

        void OptimizeOneShot();

        void Terminate();

        void Pause();

        void Unpause();

        void QueuePrepare(NetworkParameters params);

        void QueueSetup();

        void QueueUpdate();

        void StartConcurrentOptimizer();

        void UpdateInputs(NetworkParameters inputs, std::shared_ptr<OptimizationParams> params, bool forceSet = false);//Has to be used in the main loop to update the relevant information if something changed

        void UpdateResetConstraints();

        void StopConcurrentOptimizer();

        void UpdateParams(NetworkParameters &inputs);

        bool Update(NetworkParameters &params);

        void drawGUI(/*OptimizationParams &params, NetworkParameters input*/);

        Network m_net;
        BinarySemaphore m_pausesem;

        std::atomic<bool> m_prepare;
        NetworkParameters m_next_parameters;
        std::atomic<bool> m_setup;
        std::atomic<bool> m_update;

        torch::Tensor m_full_pose, m_zin, m_beta, m_trans;
        torch::Tensor m_last_grad, m_last_beta, m_last_pose, m_last_zin, m_last_trans;

        std::unique_ptr<torch::optim::Optimizer> m_optim;
        std::unique_ptr<ReduceLROnPlateau> m_scheduler;

        std::vector<TorchConstraint> m_torch_constraints;

        std::shared_ptr<OptimizationParams> m_optparams;

        std::recursive_mutex m_synchronization;
        std::mutex m_results_lock;
        std::atomic<bool> m_terminate;
        std::atomic<bool> m_pause_indicator;
        float m_current_loss;
        float m_current_learning_rate;
        bool m_lbfgs;

        torch::Tensor t_loss;

        static void drawClassGUI();

    private:
        std::unique_ptr<std::thread> m_concurrent_optimizer;

        std::vector<float> m_losses;
        std::vector<float> m_learning_rates;

        int m_id;

        static bool c_gui;
        static int c_count;
    };


TWIN_NAMESPACE_END

#endif //TWIN_OPTIMIZER_H
#endif