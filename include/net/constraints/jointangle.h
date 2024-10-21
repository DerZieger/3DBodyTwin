#pragma once
#ifdef HAS_DL_MODULE

#ifndef TWIN_JOINTANGLE_H
#define TWIN_JOINTANGLE_H

#include "twinDefines.h"
#include "net/constraints/constraint.h"
#include <cppgl.h>


TWIN_NAMESPACE_BEGIN

    class TorchAngleConstraintImpl : public TorchConstraintImpl {
    public:
        explicit TorchAngleConstraintImpl(std::string name);

        torch::Tensor m_joints;
        torch::Tensor m_lboundaries;
        torch::Tensor m_hboundaries;
        torch::Tensor m_active;
    private:
        torch::Tensor compute_impl(ConstraintArgument &ca) override;
    };

    using TorchAngleLockConstraint = cppgl::NamedHandle<TorchAngleConstraintImpl>;

    class AngleConstraintImpl : public ConstraintImpl {
    public:
        AngleConstraintImpl();

        explicit AngleConstraintImpl(const std::string &cname);

        bool drawGUIShort() override;

        void drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const twin::Ray &ray, cppgl::vec2 relative_pixel,                         bool pressed) override;

        void Finalize() override;

        void createDrawelement(const cppgl::Drawelement &dre) override;

        void Serialize(nlohmann::json &json) const override;

        void Deserialize(const nlohmann::json &json, std::string cname) override;

    protected:
        TorchConstraint PrepareOptimImpl(ConstraintArgument &ca) override;

        TorchConstraint GetConstraint() override;

    private:
        TorchAngleLockConstraint m_torch_constraint;
        std::vector<int> m_joints;
        std::vector<int> m_active;
        std::vector<float> m_lbound;
        std::vector<float> m_hbound;


        static int counter;
    };

    using AngleConstraint = cppgl::NamedHandle<AngleConstraintImpl>;

TWIN_NAMESPACE_END

#endif //TWIN_JOINTANGLE_H
#endif