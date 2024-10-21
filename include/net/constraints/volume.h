#pragma once

#ifdef HAS_DL_MODULE
#ifndef TWIN_VOLUME_H
#define TWIN_VOLUME_H

#include "twinDefines.h"
#include "net/constraints/constraint.h"
#include "util/ray.h"
#include "util/bvh.h"
#include <cppgl.h>

TWIN_NAMESPACE_BEGIN

    class TorchVolumeConstraintImpl : public TorchConstraintImpl {
    public:
        explicit TorchVolumeConstraintImpl(std::string name);

        std::atomic<float> m_density;

    private:
        torch::Tensor compute_impl(ConstraintArgument &ca) override;
    };

    using TorchVolumeConstraint = cppgl::NamedHandle<TorchVolumeConstraintImpl>;


    class VolumeConstraintImpl : public ConstraintImpl {
    public:
        VolumeConstraintImpl();

        explicit VolumeConstraintImpl(const std::string &cname);

        VolumeConstraintImpl(const std::string &cname, float mass);

        bool drawGUIShort() override;

        void drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const twin::Ray &ray, cppgl::vec2 relative_pixel,                         bool pressed) override;

        void Finalize() override;

        void createDrawelement(const cppgl::Drawelement &dre) override;

        void Serialize(nlohmann::json &json) const override;

        void Deserialize(const nlohmann::json &json, std::string cname) override;

        bool UsesTarget() override;

    protected:
        TorchConstraint PrepareOptimImpl(ConstraintArgument &ca) override;

        TorchConstraint GetConstraint() override;

        void ExchangeData(TorchVolumeConstraint con);

    private:
        float m_density;

        static int counter;

        TorchVolumeConstraint m_torch_constraint;
    };

    using VolumeConstraint = cppgl::NamedHandle<VolumeConstraintImpl>;


TWIN_NAMESPACE_END

#endif //TWIN_VOLUME_H
#endif