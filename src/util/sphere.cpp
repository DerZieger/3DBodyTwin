#include "util/sphere.h"
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

    int Sphere::intersectAabb(const twin::AABB &other) const {
        if (!intersectAabb2(other)) return 0;

        for (int i = 0; i < 8; i++) {
            if (!contains(other.cornerPoint(i))) return 1;
        }
        return 2;
    }

    bool Sphere::intersectAabb2(const AABB &other) const {
        float s, d = 0;

        // find the square of the distance
        // from the sphere to the box
        for (long i = 0; i < 3; i++) {
            if (pos[i] < other.min[i]) {
                s = pos[i] - other.min[i];
                d += s * s;
            } else if (pos[i] > other.max[i]) {
                s = pos[i] - other.max[i];
                d += s * s;
            }
        }
        return d <= r * r;
    }

    void Sphere::getMinimumAabb(AABB &box) const {
        cppgl::vec3 rad(r + 1, r + 1, r + 1);
        box.min = pos - rad;
        box.max = pos + rad;
    }

    bool Sphere::contains(const cppgl::vec3 &p) const {
        return cppgl::length(p - pos) < r;
    }

    bool Sphere::intersect(const Sphere &other) const {
        return cppgl::distance(other.pos, pos) < r + other.r;
    }

    float Sphere::sdf(const cppgl::vec3 &p) const {
        return (p - pos).norm() - r;
    }

    cppgl::vec2 Sphere::projectedIntervall(const cppgl::vec3 &d) const {
        cppgl::vec2 ret;
        float t = cppgl::dot(d, pos);
        ret[0] = t - r;
        ret[1] = t + r;
        return ret;
    }

    std::ostream &operator<<(std::ostream &os, const twin::Sphere &s) {
        os << "[Sphere] Center: " << s.pos << " radius: " << s.r;
        return os;
    }

TWIN_NAMESPACE_END