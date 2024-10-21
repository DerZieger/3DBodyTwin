#include "util/plane.h"
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
    Plane::Plane() : m_normal(cppgl::vec3(0, 1, 0)), m_d(0) {
        static_assert(sizeof(Plane) == 4 * sizeof(float), "Wrong plane size!");
    }

    Plane::Plane(const cppgl::vec3 &point, const cppgl::vec3 &normal) {
        m_normal = normal.normalized();
        m_d = point.dot(m_normal);
    }

    Plane::Plane(const cppgl::vec3 &p1, const cppgl::vec3 &p2, const cppgl::vec3 &p3) {
        m_normal = cppgl::cross(p2 - p1, p3 - p1);
        m_normal = m_normal.normalized();
        m_d = p1.dot(m_normal);
    }

    Plane Plane::invert() const {
        Plane invPlane;
        invPlane.m_normal = -this->m_normal;
        invPlane.m_d = -this->m_d;
        return invPlane;
    }

    float Plane::distance(const cppgl::vec3 &p) const {
        return cppgl::dot(p, m_normal) - m_d;
    }

    float Plane::sphereOverlap(const cppgl::vec3 &c, float r) const {
        return r - distance(c);
    }

    std::pair<cppgl::vec3, float> Plane::intersectingCircle(const cppgl::vec3 &c, float r) const {
        float dis = distance(c);
        float radius = static_cast<float>(sqrt(std::max(r * r - dis * dis, 0.0f)));
        cppgl::vec3 center = c - dis * m_normal;
        return {center, radius};
    }

    cppgl::vec3 Plane::closestPointOnPlane(const cppgl::vec3 &p) const {
        float dis = distance(p);
        return p - dis * m_normal;
    }

    cppgl::vec3 Plane::getPoint() const {
        return m_normal * m_d;
    }

    std::ostream &operator<<(std::ostream &os, const Plane &plane) {
        os << "[Plane] normal: " << plane.m_normal.transpose() << " distance: " << plane.m_d;
        return os;
    }

TWIN_NAMESPACE_END