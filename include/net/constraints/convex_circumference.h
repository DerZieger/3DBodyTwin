#pragma once

#ifdef HAS_DL_MODULE
#ifndef TWIN_CONVEX_CIRCUMFERENCE_H
#define TWIN_CONVEX_CIRCUMFERENCE_H

#include "twinDefines.h"
#include <cppgl.h>
#include "net/constraints/constraint.h"
#include "util/bvh.h"
#include "util/plane.h"
#include "util/ray.h"
#include "util/selection.h"

TWIN_NAMESPACE_BEGIN

    class TorchCircumferenceConstraintImpl : public TorchConstraintImpl {
    public:
        explicit TorchCircumferenceConstraintImpl(std::string name);

        torch::Tensor m_t_index;
        torch::Tensor m_segment_index;
        torch::Tensor m_segment_alpha;
    private:
        torch::Tensor compute_impl(ConstraintArgument &ca) override;
    };

    using TorchCircumferenceConstraint = cppgl::NamedHandle<TorchCircumferenceConstraintImpl>;


    class CircumferenceConstraintImpl : public ConstraintImpl {
    public:
        CircumferenceConstraintImpl();

        explicit CircumferenceConstraintImpl(const std::string &cname);

        CircumferenceConstraintImpl(const std::string &cname, float dot, cppgl::vec3 normal, float circumference);

        bool drawGUIShort() override;

        void drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const twin::Ray &ray, cppgl::vec2 relative_pixel, bool pressed) override;

        void Finalize() override;

        void createDrawelement(const cppgl::Drawelement &dre) override;

        void Serialize(nlohmann::json &json) const override;

        void Deserialize(const nlohmann::json &json, std::string cname) override;

        bool UsesTarget() override;

        std::vector<int> m_faces;//vector of triangle indices that mark the start of a triangle in index buffer of a mesh (max_size: numindices/3)

        struct IntersectionPoint {
            int v0, v1;
            float alpha;
            cppgl::vec3 point;
            cppgl::vec2 rotated;
            int tri;
        };

        static std::vector<CircumferenceConstraintImpl::IntersectionPoint> GiftWrapping(std::vector<CircumferenceConstraintImpl::IntersectionPoint> &points, const cppgl::vec2 &center);

        std::vector<int32_t> m_indices;

        std::vector<float> m_alphas;

    protected:
        TorchConstraint PrepareOptimImpl(ConstraintArgument &ca) override;

        TorchConstraint GetConstraint() override;

    private:
        struct Triangle {
            cppgl::vec3 v[3] = {};
            int id[3] = {};
        };

        /**
         *   @brief Intersects the object with the given plane
         *   @return the points on which the plane cuts the triangles, the first element contains the leftmost point on the
         * plane by x coordinate the center of the line segments.
         */
        std::pair<std::vector<CircumferenceConstraintImpl::IntersectionPoint>, cppgl::vec2> TrianglePlaneIntersection(const std::vector<CircumferenceConstraintImpl::Triangle> &triangles, const Plane &plane) const;

        void TrianglesToIndices(const std::vector<CircumferenceConstraintImpl::Triangle> &triangles);

        cppgl::vec3 m_normal;

        float m_dot;

        RectangleSelection m_selection;

        int m_pressed_counter;

        static int counter;

        TorchCircumferenceConstraint m_torch_constraint;
    };

    using CircumferenceConstraint = cppgl::NamedHandle<CircumferenceConstraintImpl>;

TWIN_NAMESPACE_END

#endif //TWIN_CONVEX_CIRCUMFERENCE_H
#endif