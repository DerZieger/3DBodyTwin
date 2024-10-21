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
#ifndef TWIN_TRIANGLE_H
#define TWIN_TRIANGLE_H

#include "twinDefines.h"
#include <cppgl.h>
#include "util/ray.h"

TWIN_NAMESPACE_BEGIN

    namespace Intersection {
        class RayTriangleIntersection;
    }


    class Triangle {
    public:
        cppgl::vec3 a, b, c;
        int id;
        double area;

        Triangle();

        Triangle(const cppgl::vec3 &a, const cppgl::vec3 &b, const cppgl::vec3 &c);

        Triangle(cppgl::vec3 a, cppgl::vec3 b, cppgl::vec3 c, int id);

        /**
        * @return center of triangle
        */
        cppgl::vec3 center() const;

        /**
        * @return angle at corner i
        */
        double angleAtCorner(int i) const;

        /**
        * @return cosine angle at corner i
        */
        double cosAngleAtCorner(int i) const;

        /**
        * @return true is triangle is degenerate
        */
        bool isDegenerate() const;

        /**
        * @return normal vector of triangle
        */
        cppgl::vec3 normal() const;

        /**
        * @return true is point is on the triangle
        */
        bool isOnTriangle(const cppgl::vec2 &p) const;

        /**
        * Calculates the barycentric coordinate for a given point ignoring the third dimension
        *
        * @param p 2d point
        * @return barycentric coordinates
        */
        cppgl::vec3 baryCoords(const cppgl::vec2 &p) const;

        /**
        * @param p barycentric coordinates
        * @return world coordinates
        */
        cppgl::vec3 baryToCart(const cppgl::vec3 &p) const;

        /**
        * @param m1 first barycentric coordinate
        * @param m2 second barycentric coordinate
        * @param m3 third barycentric coordinate
        * @return world coordinates
        */
        cppgl::vec3 baryToCart(const double &m1, const double &m2, const double &m3) const;

        friend std::ostream &operator<<(std::ostream &os, const Triangle &dt);

        Intersection::RayTriangleIntersection rayHit(const Ray &ray, float epsilon = 0.00001f) const;
    };

TWIN_NAMESPACE_END

#endif //TWIN_TRIANGLE_H
