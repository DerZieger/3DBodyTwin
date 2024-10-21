#pragma once

#ifndef TWIN_SELECTION_H
#define TWIN_SELECTION_H

#include "twinDefines.h"
#include <cppgl.h>
#include "util/frustum.h"
#include "util/ray.h"

TWIN_NAMESPACE_BEGIN

    class RectangleSelection {
    public:
        RectangleSelection();

        void ResetSelection();

        std::vector<int> Select(const cppgl::Drawelement &mesh, const Ray &ray, const cppgl::vec2 &pixel_percent);

        void Render() const;

    private:
        cppgl::vec2 m_corners[2];
        Frustum m_frustum;
        cppgl::Drawelement m_quad;
        static int counter;
    };


TWIN_NAMESPACE_END

#endif //TWIN_SELECTION_H
