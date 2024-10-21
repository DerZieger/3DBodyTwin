#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_OSIMMARKER_H
#define TWIN_OSIMMARKER_H

#include "twinDefines.h"
#include <cppgl.h>
#include "markers.h"
#include "net/constraints/constraint.h"
#include "util/bvh.h"
#include "util/ray.h"
#include "osim.h"
#include "net/constraints/marker.h"

TWIN_NAMESPACE_BEGIN

    class OsimMarkerConstraintImpl : public MarkerConstraintBaseImpl {
    public:
        OsimMarkerConstraintImpl();

        explicit OsimMarkerConstraintImpl(std::shared_ptr<Osim> osim);

        explicit OsimMarkerConstraintImpl(const std::string &cname);

        OsimMarkerConstraintImpl(const std::string &cname, std::shared_ptr<Osim> osim);

        void Serialize(nlohmann::json &json) const override;

        void Deserialize(const nlohmann::json &json, std::string cname) override;

        bool isReferenceSet() const override;

        void referenceGUI(std::vector<std::shared_ptr<Osim>> &vOsim);

        void drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const twin::Ray &ray, cppgl::vec2 relative_pixel, bool pressed) override;

        void Finalize() override;

        int getZeroFrame() const override;

        int getFramerate() const override;

        void UpdateMarkerData() override;

        std::shared_ptr<Osim> m_osim;
    protected:
        TorchConstraint PrepareOptimImpl(ConstraintArgument &ca) override;

    };

    using OsimMarkerConstraint = cppgl::NamedHandle<OsimMarkerConstraintImpl>;

TWIN_NAMESPACE_END

#endif //TWIN_OSIMMARKER_H
#endif