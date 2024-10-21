#pragma once
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
#ifndef TWIN_BVH_H
#define TWIN_BVH_H

#include "twinDefines.h"
#include <cppgl.h>
#include "util/triangle.h"
#include "util/aabb.h"
#include "util/ray.h"
#include "util/intersection.h"

TWIN_NAMESPACE_BEGIN
/**
* BVHNode class
*/
    class BVHNode {
    public:
        bool isLeaf = false;
        AABB box;
        uint32_t left = 0;//Left child index
        uint32_t right = 0;//Right child index

    };

/**
* BVH class
*/
    class BVH {
        /**
        * Helper struct to sort trianlges along a axis
        */
        class Axis {
        public:
            explicit Axis(int a) : axis(a) {}

            inline bool operator()(const std::pair<Triangle, int> &A, const std::pair<Triangle, int> &B) const {
                const cppgl::vec3 &a = A.first.center();
                const cppgl::vec3 &b = B.first.center();
                return a(axis) < b(axis);
            }

            int axis;
        };

        std::vector<BVHNode> nodes;//all nodes in the bvh

        std::vector<Triangle> next_tris;

        int leafTris;

        float bvh_epsilon;

    public:

        std::vector<std::pair<Triangle, int>> tris;

        BVH(int leafs = 5, float bvh_eps = 0.0001f);

        BVH(const std::vector<Triangle> &inTris, int leafs = 5, float bvh_eps = 0.0001f);

        AABB computeBox(int start, int end) const;

        void construct();

        /**
        * Constructs the bvh for the given triangles
        *
        * @param tris list of triangles
        */
        void construct(std::vector<Triangle> &inTris);


        /**
        * Constructs the bvh for the section of the triangle list [start,end)
        *
        * @param tris list of triangles
        * @param start start index
        * @param end end index
        */
        int construct(int start, int end);

        void setNextTris(const std::vector<Triangle> &triangles);

        void setNextTris(const std::vector<Triangle> &&triangles);

        void Update();

        friend std::ostream &operator<<(std::ostream &os, const BVH &dt);

        Intersection::RayTriangleIntersection getClosest(const Ray &ray);

        void getClosest(uint32_t node, const Ray &ray, Intersection::RayTriangleIntersection &result) const;
    };


TWIN_NAMESPACE_END


#endif //TWIN_BVH_H
