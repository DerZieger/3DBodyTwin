#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_MARKER_H
#define TWIN_MARKER_H

#include "twinDefines.h"
#include <cppgl.h>
#include "markers.h"
#include "net/constraints/constraint.h"
#include "util/bvh.h"
#include "util/ray.h"

TWIN_NAMESPACE_BEGIN

    class TorchMarkerConstraintImpl : public TorchConstraintImpl {
    public:
        explicit TorchMarkerConstraintImpl(std::string name);

        torch::Tensor m_positions;
        torch::Tensor m_faces, m_barycenter, m_dis, m_vertices;
        torch::Tensor m_residuals, m_current_positions;


        std::atomic<bool> m_optimize_barycenter;
        std::atomic<bool> m_switch_optimize_barycenter;

        bool hasOptimizableParams() const override;

        std::vector<torch::Tensor> OptimizableParams() override;

    private:
        torch::Tensor compute_impl(ConstraintArgument &ca) override;
    };

    using TorchMarkerConstraint = cppgl::NamedHandle<TorchMarkerConstraintImpl>;

    class MarkerConstraintBaseImpl : public ConstraintImpl {
    public:
        MarkerConstraintBaseImpl();

        explicit MarkerConstraintBaseImpl(const std::string &cname);

        bool drawGUIShort() override;

        virtual void drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const twin::Ray &ray, cppgl::vec2 relative_pixel, bool pressed) override;

        virtual void Finalize() override;

        void createDrawelement(const cppgl::Drawelement &dre) override;

        virtual void Serialize(nlohmann::json &json) const = 0;

        virtual void Deserialize(const nlohmann::json &json, std::string cname) = 0;

        virtual bool isReferenceSet() const override;

        virtual void UpdateMarkerData() = 0;

        std::map<std::string, MarkerData> m_markers;
    protected:
        virtual TorchConstraint PrepareOptimImpl(ConstraintArgument &ca) = 0;

        TorchConstraint GetConstraint() override;

        Change OnFrameChange() override;

        TorchMarkerConstraint m_torch_constraint;

        std::vector<std::string> m_unused_markers;
    private:

        std::string m_current_selection;

        int m_selected_unused_markers;

        bool m_render_texts;

        bool m_optimize_marker_positions;

        static int counter;
    };


    void to_json(nlohmann::json &j, const MarkerData &d);

    void from_json(const nlohmann::json &j, MarkerData &d);

TWIN_NAMESPACE_END

#endif //TWIN_MARKER_H
#endif