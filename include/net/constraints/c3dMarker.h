#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_C3DMARKER_H
#define TWIN_C3DMARKER_H

#include "twinDefines.h"
#include <cppgl.h>
#include "markers.h"
#include "net/constraints/constraint.h"
#include "util/bvh.h"
#include "util/ray.h"
#include "net/constraints/marker.h"
#include "c3d.h"

TWIN_NAMESPACE_BEGIN

    class C3DMarkerConstraintImpl : public MarkerConstraintBaseImpl {
    public:
        C3DMarkerConstraintImpl();

        explicit C3DMarkerConstraintImpl(std::shared_ptr<C3D> c3d);

        explicit C3DMarkerConstraintImpl(const std::string &cname);

        C3DMarkerConstraintImpl(const std::string &cname, std::shared_ptr<C3D> c3d);

        void Serialize(nlohmann::json &json) const override;

        void Deserialize(const nlohmann::json &json, std::string cname) override;

        bool isReferenceSet() const override;

        void referenceGUI(std::vector<std::shared_ptr<C3D>> &vC3D);

        void drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const twin::Ray &ray, cppgl::vec2 relative_pixel, bool pressed) override;

        void Finalize() override;

        int getZeroFrame() const override;

        int getFramerate() const override;

        std::shared_ptr<C3D> m_c3d;

        void UpdateMarkerData() override;

    protected:
        TorchConstraint PrepareOptimImpl(ConstraintArgument &ca) override;
    };

    using C3DMarkerConstraint = cppgl::NamedHandle<C3DMarkerConstraintImpl>;

TWIN_NAMESPACE_END

#endif //TWIN_C3DMARKER_H
#endif