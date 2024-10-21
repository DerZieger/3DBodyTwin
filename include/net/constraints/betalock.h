#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_BETALOCK_H
#define TWIN_BETALOCK_H

#include "twinDefines.h"
#include "net/constraints/constraint.h"
#include <cppgl.h>

TWIN_NAMESPACE_BEGIN


    class TorchBetaLockConstraintImpl : public TorchConstraintImpl {
    public:
        explicit TorchBetaLockConstraintImpl(std::string name);

        std::atomic<float> m_beta_sigma;

    private:
        torch::Tensor compute_impl(ConstraintArgument &ca) override;
    };

    using TorchBetaLockConstraint = cppgl::NamedHandle<TorchBetaLockConstraintImpl>;

    class BetaLockConstraintImpl : public ConstraintImpl {
    public:
        BetaLockConstraintImpl();

        explicit BetaLockConstraintImpl(const std::string &cname);

        bool drawGUIShort() override;

        void drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const twin::Ray &ray, cppgl::vec2 relative_pixel, bool pressed) override;

        void Finalize() override;

        void createDrawelement(const cppgl::Drawelement &dre) override;

        void Serialize(nlohmann::json &json) const override;

        void Deserialize(const nlohmann::json &json, std::string cname) override;

    protected:
        TorchConstraint PrepareOptimImpl(ConstraintArgument &ca) override;

        TorchConstraint GetConstraint() override;

    private:
        TorchBetaLockConstraint m_torch_constraint;
        float m_beta_sigma;

        static int counter;
    };

    using BetaLockConstraint = cppgl::NamedHandle<BetaLockConstraintImpl>;

TWIN_NAMESPACE_END

#endif //TWIN_BETALOCK_H
#endif