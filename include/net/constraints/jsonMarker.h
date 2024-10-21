#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_JSONMARKER_H
#define TWIN_JSONMARKER_H

#include "twinDefines.h"
#include <cppgl.h>
#include "markers.h"
#include "net/constraints/constraint.h"
#include "util/bvh.h"
#include "util/ray.h"
#include "net/constraints/marker.h"
#include "jsonMarkers.h"


TWIN_NAMESPACE_BEGIN

    class JsonMarkerConstraintImpl : public MarkerConstraintBaseImpl {
    public:
        JsonMarkerConstraintImpl();

        explicit JsonMarkerConstraintImpl(std::shared_ptr<JMarker> mjs);

        explicit JsonMarkerConstraintImpl(const std::string &cname);

        JsonMarkerConstraintImpl(const std::string &cname, std::shared_ptr<JMarker> mjs);

        void Serialize(nlohmann::json &json) const override;

        void Deserialize(const nlohmann::json &json, std::string cname) override;

        bool isReferenceSet() const override;

        void referenceGUI(std::vector<std::shared_ptr<JMarker>> &vmjs);

        void drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const twin::Ray &ray, cppgl::vec2 relative_pixel, bool pressed) override;

        void Finalize() override;

        int getZeroFrame() const override;

        int getFramerate() const override;

        std::shared_ptr<JMarker> m_json;

        void UpdateMarkerData() override;

    protected:
        TorchConstraint PrepareOptimImpl(ConstraintArgument &ca) override;

    };

    using JsonMarkerConstraint = cppgl::NamedHandle<JsonMarkerConstraintImpl>;

TWIN_NAMESPACE_END

#endif //TWIN_JSONMARKER_H
#endif