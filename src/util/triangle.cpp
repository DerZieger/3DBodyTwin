#include "util/intersection.h"
#include "util/triangle.h"
#include <glog/logging.h>
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

    Triangle::Triangle() : Triangle(cppgl::make_vec3(0.f), cppgl::make_vec3(0.f), cppgl::make_vec3(0.f), 0) {}

    Triangle::Triangle(const cppgl::vec3 &a, const cppgl::vec3 &b, const cppgl::vec3 &c) : Triangle(a, b, c, 0) {}

    Triangle::Triangle(cppgl::vec3 a, cppgl::vec3 b, cppgl::vec3 c, int id) : a(std::move(a)), b(std::move(b)), c(std::move(c)), id(id) {
        area = 0.5 * ((this->b - this->a).cross(this->c - this->a)).norm();
    }

    cppgl::vec3 Triangle::center() const {
        return (a + b + c) * 1.f / 3.f;
    }

    double Triangle::angleAtCorner(int i) const {
        return acos(cosAngleAtCorner(i));
    }

    double Triangle::cosAngleAtCorner(int i) const {
        cppgl::vec3 center = a;
        cppgl::vec3 left = b;
        cppgl::vec3 right = c;

        switch (i) {
            case 0:
                center = a;
                left = b;
                right = c;
                break;
            case 1:
                center = b;
                left = c;
                right = a;
                break;
            case 2:
                center = c;
                left = a;
                right = b;
                break;
            default:
                LOG(FATAL) << "Select a valid corner for cosAngleAtCorner" << std::endl;
        }
        return (cppgl::vec3(left - center).normalized()).dot(cppgl::vec3(right - center).normalized());
    }

    bool Triangle::isDegenerate() const {
        for (int i = 0; i < 3; ++i) {
            double an = cosAngleAtCorner(i);
            if (an <= -1 || an >= 1)
                return true;
        }
        return false;
    }

    cppgl::vec3 Triangle::normal() const {
        return ((b - a).cross(c - a)).normalized();
    }

    bool Triangle::isOnTriangle(const cppgl::vec2 &p) const {

        const cppgl::vec3 bary = baryCoords(p);
        return 0 <= bary(0) && bary(0) <= 1 && 0 <= bary(1) && bary(1) <= 1 && 0 <= bary(2) && bary(2) <= 1;
    }

    cppgl::vec3 Triangle::baryCoords(const cppgl::vec2 &p) const {
        double nennerInv = 1.0 / ((b(0) - a(0)) * (c(1) - b(1)) - (b(1) - a(1)) * (c(0) - b(0)));
        double m1 = ((b(0) - p(0)) * (c(1) - p(1)) - (b(1) - p(1)) * (c(0) - p(0))) * nennerInv;
        double m2 = ((c(0) - p(0)) * (a(1) - p(1)) - (c(1) - p(1)) * (a(0) - p(0))) * nennerInv;
        double m3 = ((a(0) - p(0)) * (b(1) - p(1)) - (a(1) - p(1)) * (b(0) - p(0))) * nennerInv;

        return cppgl::vec3(static_cast<float>(m1), static_cast<float>(m2), static_cast<float>(m3)) / (m1 + m2 + m3);
    }

    cppgl::vec3 Triangle::baryToCart(const cppgl::vec3 &p) const {
        return baryToCart(p(0), p(1), p(2));
    }

    cppgl::vec3 Triangle::baryToCart(const double &m1, const double &m2, const double &m3) const {
        double invSum = 1.0 / (m1 + m2 + m3);
        double x = (m1 * a(0) + m2 * b(0) + m3 * c(0)) * invSum;
        double y = (m1 * a(1) + m2 * b(1) + m3 * c(1)) * invSum;
        double z = (m1 * a(2) + m2 * b(2) + m3 * c(2)) * invSum;
        return cppgl::vec3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
    }


    std::ostream &operator<<(std::ostream &os, const Triangle &t) {
        os << "[Triangle] A: " << t.a.transpose() << " B: " << t.b.transpose() << " C: " << t.c.transpose() << " Area: " << t.area;
        return os;
    }

    Intersection::RayTriangleIntersection Triangle::rayHit(const Ray &ray, float epsilon) const {
        return Intersection::RayTriangle(*this, ray, epsilon);
    }
TWIN_NAMESPACE_END