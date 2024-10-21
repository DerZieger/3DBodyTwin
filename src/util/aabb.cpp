#include "util/intersection.h"
#include "util/aabb.h"
#include <limits>
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

twin::AABB::AABB() : min(cppgl::make_vec3(std::numeric_limits<float>::max())), max(cppgl::make_vec3(std::numeric_limits<float>::min())) {}

twin::AABB::AABB(cppgl::vec3 min, cppgl::vec3 max) : min(std::move(min)), max(std::move(max)) {}

cppgl::vec3 twin::AABB::getPosition() const {
    return 0.5f * (min + max);
}

bool twin::AABB::isInside(const cppgl::vec3 &pos) const {
    return (min(0) <= pos(0) && pos(0) <= max(0)) && (min(1) <= pos(1) && pos(1) <= max(1)) && (min(2) <= pos(2) && pos(2) <= max(2));
}

bool twin::AABB::isInside2d(const cppgl::vec2 &pos) const {
    return (min(0) <= pos(0) && pos(0) <= max(0)) && (min(1) <= pos(1) && pos(1) <= max(1));
}

void twin::AABB::growBox(const cppgl::vec3 &v) {
    min(0) = min(0) < v(0) ? min(0) : v(0);
    min(1) = min(1) < v(1) ? min(1) : v(1);
    min(2) = min(2) < v(2) ? min(2) : v(2);
    max(0) = max(0) > v(0) ? max(0) : v(0);
    max(1) = max(1) > v(1) ? max(1) : v(1);
    max(2) = max(2) > v(2) ? max(2) : v(2);
}

void twin::AABB::growBox(const twin::AABB &aabb) {
    this->growBox(aabb.min);
    this->growBox(aabb.max);
}

void twin::AABB::growBox(const twin::Triangle &tri) {
    this->growBox(tri.a);
    this->growBox(tri.b);
    this->growBox(tri.c);
}

void twin::AABB::invalidate() {
    min = cppgl::make_vec3(std::numeric_limits<float>::max());
    max = cppgl::make_vec3(std::numeric_limits<float>::min());
}

int twin::AABB::biggestSpan() const {
    double x = max(0) - min(0);
    double y = max(1) - min(1);
    double z = max(2) - min(2);
    if (x < z && x < y) {
        return 0;
    } else if (y < x && y < z) {
        return 1;
    } else {
        return 2;
    }
}

cppgl::vec3 twin::AABB::cornerPoint(int cornerIndex) const {
    switch (cornerIndex) {
        default:
        case 0:
            return cppgl::vec3(min[0], min[1], min[2]);
        case 1:
            return cppgl::vec3(min[0], min[1], max[2]);
        case 2:
            return cppgl::vec3(min[0], max[1], max[2]);
        case 3:
            return cppgl::vec3(min[0], max[1], min[2]);
        case 4:
            return cppgl::vec3(max[0], min[1], min[2]);
        case 5:
            return cppgl::vec3(max[0], min[1], max[2]);
        case 6:
            return cppgl::vec3(max[0], max[1], max[2]);
        case 7:
            return cppgl::vec3(max[0], max[1], min[2]);
    }
}


TWIN_NAMESPACE_BEGIN
    std::ostream &operator<<(std::ostream &os, const AABB &aabb) {
        os << "[AABB] Min: " << aabb.min.transpose() << " Max: " << aabb.max.transpose();
        return os;
    }

    bool AABB::rayHit(const Ray &ray, float &t) const {
        return Intersection::RayAABB(ray, *this, t);
    }
TWIN_NAMESPACE_END