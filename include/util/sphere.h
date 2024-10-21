#pragma once

#ifndef TWIN_SPHERE_H
#define TWIN_SPHERE_H
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
#include "util/aabb.h"

TWIN_NAMESPACE_BEGIN

    class Sphere {
    public:
        cppgl::vec3 pos;
        float r;


        Sphere() { static_assert(sizeof(Sphere) == 16, "Sphere must be 16 byte"); }

        Sphere(const cppgl::vec3 &p, float r) : pos(p), r(r) {}


        int intersectAabb(const AABB &other) const;

        bool intersectAabb2(const AABB &other) const;

        void getMinimumAabb(AABB &box) const;

        bool contains(const cppgl::vec3 &p) const;

        bool intersect(const Sphere &other) const;

        // Signed distance to sphere surface.
        // >0 means outside
        // <0 means inside
        // =0 on the surface
        float sdf(const cppgl::vec3 &p) const;


        cppgl::vec2 projectedIntervall(const cppgl::vec3 &d) const;

        //    TriangleMesh* createMesh(int rings, int sectors);
        //    void addToBuffer(std::vector<VertexNT> &vertices, std::vector<GLuint> &indices, int rings, int sectors);

        friend std::ostream &operator<<(std::ostream &os, const Sphere &s);
    };

TWIN_NAMESPACE_END

#endif //TWIN_SPHERE_H
