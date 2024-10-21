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
#ifndef TWIN_AABB_H
#define TWIN_AABB_H

#include "twinDefines.h"
#include "util/triangle.h"
#include <cppgl.h>
#include "util/ray.h"

TWIN_NAMESPACE_BEGIN

    class AABB {
    public:
        cppgl::vec3 min;
        cppgl::vec3 max;


        AABB();

        AABB(cppgl::vec3 min, cppgl::vec3 max);

        /**
        *@return the middle of the aabb
        */
        cppgl::vec3 getPosition() const;

        /**
        *@return true if point is inside the aabb
        */
        bool isInside(const cppgl::vec3 &pos) const;

        /**
        *@return  true if point is inside the aabb, but the third dimension is ignor
        */
        bool isInside2d(const cppgl::vec2 &pos) const;

        /**
        * Increase the size of the aabb to fit param v
        *
        *@param v point in space
        */
        void growBox(const cppgl::vec3 &v);

        /**
        * Increase the size of the aabb to fit the other aabb
        *
        *@param aabb another aabb
        */
        void growBox(const AABB &aabb);

        /**
        * Increase the size of the aabb to fit the triangle
        *
        *@param tri triangle
        */
        void growBox(const twin::Triangle &tri);

        /**
        *invalidate the aabb
        */
        void invalidate();

        /**
        *@return  dimension of the aabb with biggestSpan
        */
        int biggestSpan() const;

        cppgl::vec3 cornerPoint(int cornerIndex) const;

        friend std::ostream &operator<<(std::ostream &os, const AABB &dt);

        bool rayHit(const Ray &ray, float &t) const;
    };


TWIN_NAMESPACE_END

#endif //TWIN_AABB_H
