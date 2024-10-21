#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_CONSTRAINT_H
#define TWIN_CONSTRAINT_H

#include "twinDefines.h"
#include <cppgl.h>
#include <nlohmann/json.hpp>
#include <torch/types.h>
#include "util/ray.h"
#include "util/bvh.h"

TWIN_NAMESPACE_BEGIN

    struct ConstraintArgument {
        torch::Device device;


        torch::Tensor beta;//shape parameters
        torch::Tensor theta;//full pose parameters
        torch::Tensor v_shaped;//shaped vertices
        torch::Tensor joints_shaped;// joint positions after shape
        torch::Tensor v_posed;//vertices shaped and posed
        torch::Tensor joints_posed;//joint positions after shape and pose

        torch::Tensor faces;
        torch::Tensor parents;
    };

    class TorchConstraintImpl {
    public:
        std::string name;

        explicit TorchConstraintImpl(std::string name);

        virtual ~TorchConstraintImpl();

        torch::Tensor compute(ConstraintArgument &ca);

        virtual bool hasOptimizableParams() const;

        virtual std::vector<torch::Tensor> OptimizableParams();

        std::atomic<float> m_weight;
        std::atomic<float> m_loss;
        std::atomic<float> m_value;
        std::atomic<float> m_target;
        std::mutex m_lock;

    private:
        static inline int id_counter = 0;

        virtual torch::Tensor compute_impl(ConstraintArgument &) = 0;
    };

    using TorchConstraint = cppgl::NamedHandle<TorchConstraintImpl>;

    class ConstraintsImpl;

    using Constraints = cppgl::NamedHandle<ConstraintsImpl>;

    class ConstraintImpl {
    public:
        std::string name;
        enum class Change : uint8_t {
            ParametersRestart = 0,
            ParametersReconfigure = 1,
            None = 2,
        };

        ConstraintImpl();

        explicit ConstraintImpl(std::string name);

        virtual ~ConstraintImpl();

        void UpdateReferencePose(int frameid);

        virtual bool drawGUIShort() = 0;

        virtual void drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const twin::Ray &ray, cppgl::vec2 relative_pixel, bool pressed) = 0;

        void SetModel(cppgl::mat4 model);

        virtual void Serialize(nlohmann::json &json) const = 0;

        virtual void Deserialize(const nlohmann::json &json, std::string name) = 0;

        TorchConstraint PrepareOptim(ConstraintArgument &ca);

        virtual void Finalize() {};

        virtual void createDrawelement(const cppgl::Drawelement &dre) = 0;

        // Observer Stuff
        Change Register(const std::string &name);

        Change Deregister(const std::string &name);

        virtual void render() const;

        cppgl::mat4 m_model;

        const int m_id;

        bool NeedRecomputeRenderObjects() const;

        virtual bool UsesTarget();

        virtual bool isReferenceSet() const;

        virtual int getZeroFrame() const;//Returns first frame with event RHS RTS for selected dataset

        virtual int getFramerate() const;

        float m_target;

        cppgl::Drawelement m_consRepr;

    protected:
        virtual Change OnFrameChange();

        virtual Change OnRegistrationChange();

        virtual TorchConstraint GetConstraint() = 0;

        virtual TorchConstraint PrepareOptimImpl(ConstraintArgument &ca) = 0;

        void ExchangeData(TorchConstraint con);

        bool m_recompute_rendering;

        float m_weight;
        float m_loss;
        float m_value;

        // if any changes in the constraints lead to a recomputation of the torchconstraint, set it here
        void SetChange(Change change);

        void ClearChange();

        Change GetChange() const;

        bool HasChange() const;

        int m_frameid, m_last_prepared_frameid;

    private:
        void Notify(Change c) const;

        static inline int id_counter = 0;

        Change m_change;
        std::vector<std::weak_ptr<ConstraintsImpl>> m_constraints;
    };


    using Constraint = cppgl::NamedHandle<ConstraintImpl>;

TWIN_NAMESPACE_END
#endif //TWIN_CONSTRAINT_H
#endif