#pragma once

#ifdef HAS_DL_MODULE
#ifndef TWIN_DISTANCE_H
#define TWIN_DISTANCE_H

#include "twinDefines.h"
#include "net/constraints/constraint.h"
#include <cppgl.h>

//Distance constraints are calculated on the tpose not the posed mesh

TWIN_NAMESPACE_BEGIN
    class TorchDistanceConstraintImpl : public TorchConstraintImpl {
    public:
        explicit TorchDistanceConstraintImpl(std::string name);

        torch::Tensor m_index;
        torch::Tensor m_bary;
        torch::Tensor m_axis;

    private:
        torch::Tensor compute_impl(ConstraintArgument &ca) override;
    };

    using TorchDistanceConstraint = cppgl::NamedHandle<TorchDistanceConstraintImpl>;

    class DistanceConstraintImpl : public ConstraintImpl {
    public:
        DistanceConstraintImpl();

        explicit DistanceConstraintImpl(const std::string &cname);

        DistanceConstraintImpl(const std::string &cname, const cppgl::Drawelement &mesh, int face_a, int face_b, float distance, cppgl::ivec3 axis = cppgl::ivec3(1, 1, 1), cppgl::vec3 barycenter_a = cppgl::vec3(1. / 3, 1. / 3, 1. / 3), cppgl::vec3 barycenter_b = cppgl::vec3(1. / 3, 1. / 3, 1. / 3));

        bool drawGUIShort() override;

        void drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const twin::Ray &ray, cppgl::vec2 relative_pixel, bool pressed) override;

        void Finalize() override;

        void createDrawelement(const cppgl::Drawelement &dre) override;

        void Serialize(nlohmann::json &json) const override;

        void Deserialize(const nlohmann::json &json, std::string cname) override;

        bool UsesTarget() override;

        cppgl::ivec3 m_vertices[2];
        cppgl::vec3 m_barycentric[2];
    protected:
        TorchConstraint PrepareOptimImpl(ConstraintArgument &ca) override;

        TorchConstraint GetConstraint() override;

    private:
        bool m_axis[3]{};
        int m_faces[2]{};
        std::vector<cppgl::vec3> m_points;

        static int counter;

        TorchDistanceConstraint m_torch_constraint;
    };

    using DistanceConstraint = cppgl::NamedHandle<DistanceConstraintImpl>;

TWIN_NAMESPACE_END

#endif //TWIN_DISTANCE_H
#endif