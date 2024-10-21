#pragma once
#ifndef TWIN_INTERSECTION_H
#define TWIN_INTERSECTION_H

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
    class Ray;

    class Triangle;

    class Plane;

    class AABB;

    class Sphere;
    namespace Intersection {
        /**
         * Intersection of a ray with a triangle.
         * There are either no interesection or exactly one at 't'.
         * 'back' is true, if the triangle was hit from behind (counter clockwise ordering)
         */
        class RayTriangleIntersection {
        public:
            bool valid = false;
            float t = std::numeric_limits<float>().infinity();  // position on ray
            bool backFace;
            int triangleIndex;  // usefull for raytracers
            cppgl::vec3 barycenter;

            bool operator<(const RayTriangleIntersection &other) { return t < other.t; }

            explicit operator bool() const { return valid; }
        };

        RayTriangleIntersection RayTriangle(const Triangle &tri, const Ray &ray, float epsilon = 0.00001f);

        RayTriangleIntersection RayTriangle(const cppgl::vec3 &direction, const cppgl::vec3 &origin, const cppgl::vec3 &A, const cppgl::vec3 &B, const cppgl::vec3 &C, float epsilon = 0.00001);

        bool RayPlane(const Ray &r, const Plane &p, float &t);

        bool RayAABB(const cppgl::vec3 &origin, const cppgl::vec3 &direction, const cppgl::vec3 &boxmin, const cppgl::vec3 &boxmax, float &t);

        bool RayAABB(const Ray &r, const AABB &bb, float &t);

        /**
         * Intersection of 2 planes.
         * 2 general planes intersect in a line given by outDir and outPoint, unless they are parallel.
         * Returns false if the planes are parallel.
         */
        bool PlanePlane(const Plane &p1, const Plane &p2, Ray &outRay);

        /**
         * Intersection of a ray with a sphere.
         * There are either 2 intersections or 0, given by the return value.
         * t2 is always greater or equal to t1
         */
        bool RaySphere(const cppgl::vec3 &rayOrigin, const cppgl::vec3 &rayDir, const cppgl::vec3 &spherePos, float sphereRadius, float &t1, float &t2);

        bool RaySphere(const Ray &ray, const Sphere &sphere, float &t1, float &t2);

        bool SphereSphere(const cppgl::vec3 &c1, float r1, const cppgl::vec3 &c2, float r2);

        bool SphereSphere(const Sphere &s1, const Sphere &s2);

        bool AABBAABB(const cppgl::vec3 &min1, const cppgl::vec3 &max1, const cppgl::vec3 &min2, const cppgl::vec3 &max2);

        bool AABBAABB(const AABB &bb1, const AABB &bb2);

    }

TWIN_NAMESPACE_END

#endif //TWIN_INTERSECTION_H
