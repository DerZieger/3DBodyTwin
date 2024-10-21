#pragma once
#ifndef TWIN_PLANE_H
#define TWIN_PLANE_H

#include "twinDefines.h"
#include <cppgl.h>
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
TWIN_NAMESPACE_BEGIN


/**
 * Implicit representation of a plane.
 * This class is save for use in CUDA kernels.
 *
 * x * n - d = 0
 *
 * With:
 * x: a point
 * n: the plane normal
 * d: the distance to the origin (projected to n)
 */
    class Plane {
    public:
        cppgl::vec3 m_normal;
        float m_d;

        Plane();

        Plane(const cppgl::vec3 &point, const cppgl::vec3 &normal);

        /**
         *  Uses first point as plane point and computes normal via cross product.
         *  Similar to triangles the points should be ordered counter clock wise to give a positive normal.
         */
        Plane(const cppgl::vec3 &p1, const cppgl::vec3 &p2, const cppgl::vec3 &p3);

        /**
         * Returns the plane with inverted normal and offset.
         */
        Plane invert() const;

        /**
         * (Signed) Distance from the point 'p' to the plane.
         */
        float distance(const cppgl::vec3 &p) const;

        /**
         * The overlapping distance between a sphere and this plane.
         * Negative if the sphere does NOT intersect the plane.
         */
        float sphereOverlap(const cppgl::vec3 &c, float r) const;

        /**
         * The intersecting circle of a sphere on this plane.
         * Sphere center is projected on the plane and the radius is calculated.
         */
        std::pair<cppgl::vec3, float> intersectingCircle(const cppgl::vec3 &c, float r) const;

        /**
         * Returns the point on the plane which is closest to the given point p.
         */
        cppgl::vec3 closestPointOnPlane(const cppgl::vec3 &p) const;

        /**
         * Returns the point on the plane which is closest to the origin.
         */
        cppgl::vec3 getPoint() const;


        friend std::ostream &operator<<(std::ostream &os, const Plane &plane);
    };

TWIN_NAMESPACE_END

#endif //TWIN_PLANE_H
