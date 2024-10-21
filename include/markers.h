#pragma once

#ifndef TWIN_MARKERS_H
#define TWIN_MARKERS_H

#include "twinDefines.h"
#include <cppgl.h>

TWIN_NAMESPACE_BEGIN

    class MarkerData {
    public:

        MarkerData() : m_id(-1), m_fid(-1), m_baryCoords(cppgl::vec3(1. / 3, 1. / 3, 1. / 3)), m_pos(cppgl::vec3(0, 0, 0)), m_use(false), m_reference(false), m_target(cppgl::vec3(0, 0, 0)), m_dis(0.005), m_dist(1e10) {}

        int m_id;//Markerid
        int m_fid;//Faceid that is used to extract position from mesh
        cppgl::vec3 m_baryCoords;//barycentric coordinates for position calculation for faster multiple access
        cppgl::vec3 m_pos;//Calculate using vertices from m_fid and m_barycoords
        bool m_use;//Marker in use
        bool m_reference;//If marker has an assigned point on mesh
        cppgl::vec3 m_target;//Position of the actual marker
        float m_dis;//distance from skin to marker along normal
        float m_dist;
    };

TWIN_NAMESPACE_END

#endif //TWIN_MARKERS_H
