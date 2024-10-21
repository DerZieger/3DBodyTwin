#pragma once

#ifndef TWIN_FRUSTUM_H
#define TWIN_FRUSTUM_H
// ---------------------------------------------------------------------------
// Code adapted from:
// https://github.com/darglein/saiga
//    MIT License
//
//    Copyright (c) 2021 Darius Rückert <darius.rueckert@fau.de>
//
//    Permission is hereby granted, free of charge, to any person obtaining a copy
//            of this software and associated documentation files (the "Software"), to deal
//    in the Software without restriction, including without limitation the rights
//            to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//            copies of the Software, and to permit persons to whom the Software is
//    furnished to do so, subject to the following conditions:
//
//    The above copyright notice and this permission notice shall be included in all
//            copies or substantial portions of the Software.
//
//    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//            AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//    SOFTWARE.
// ---------------------------------------------------------------------------
#include "twinDefines.h"
#include <cppgl.h>
#include "util/plane.h"
#include "util/triangle.h"
#include "util/sphere.h"

TWIN_NAMESPACE_BEGIN


    class Frustum {
    public:
        enum IntersectionResult {
            OUTSIDE = 0,
            INSIDE,
            INTERSECT
        };

        // corners of the truncated pyramid
        // Ordered like this:
        //
        // Near Plane:  Far Plane:
        // 0 -- 1       4 -- 5
        // |    |       |    |
        // 2 -- 3       6 -- 7
        //
        //
        //
        //
        //
        std::array<cppgl::vec3, 8> vertices;

        // Ordered like this:
        // near, far, top, bottom, left, right
        std::array<Plane, 6> planes;

        Sphere boundingSphere;  // for fast frustum culling


        Frustum() = default;

        Frustum(const cppgl::mat4 &model, float fovy, float aspect, float zNear, float zFar, bool negativ_z = true, bool negative_y = false);

        Frustum SubFrustum(float left_begin, float right_end, float top_begin, float bot_end) const;

        void computePlanesFromVertices();

        void computePlanesFromMatrices(const cppgl::mat4 &proj, const cppgl::mat4 &view);

        // culling stuff
        IntersectionResult pointInFrustum(const cppgl::vec3 &p) const;

        IntersectionResult sphereInFrustum(const Sphere &s) const;

        IntersectionResult pointInSphereFrustum(const cppgl::vec3 &p) const;

        IntersectionResult sphereInSphereFrustum(const Sphere &s) const;


        std::array<Triangle, 12> ToTriangleList() const;

        /**
         * Return the intervall (min,max) when all vertices of the frustum are
         * projected to the axis 'd'. To dedect an overlap in intervalls the axis
         * does not have to be normalized.
         *
         * @brief projectedIntervall
         * @param d
         * @return
         */
        cppgl::vec2 projectedIntervall(const cppgl::vec3 &d) const;

        /**
         * Returns the side of the plane on which the frustum is.
         * +1 on the positive side
         * -1 on the negative side
         * 0 the plane is intersecting the frustum
         *
         * @brief sideOfPlane
         * @param plane
         * @return
         */
        int sideOfPlane(const Plane &plane) const;


        /**
         * Returns unique edges of the frustum.
         * A frustum has 6 unique edges ( non parallel edges).
         * @brief getEdge
         * @param i has to be in range (0 ... 5)
         * @return
         */

        std::pair<cppgl::vec3, cppgl::vec3> getEdge(int i) const;

        /**
         * Exact frustum-frustum intersection with the Separating Axes Theorem (SAT).
         * This test is expensive, so it should be only used when important.
         *
         * Number of Operations:
         * 6+6=12  sideOfPlane(const Plane &plane), for testing the faces of the frustum.
         * 6*6*2=72  projectedIntervall(const vec3 &d), for testing all cross product of pairs of non parallel edges
         *
         * http://www.geometrictools.com/Documentation/MethodOfSeparatingAxes.pdf
         * @brief intersectSAT
         * @param other
         * @return
         */

        bool intersectSAT(const Frustum &other) const;


        bool intersectSAT(const Sphere &s) const;

        friend std::ostream &operator<<(std::ostream &os, const Frustum &frustum);
    };

TWIN_NAMESPACE_END

#endif //TWIN_FRUSTUM_H
