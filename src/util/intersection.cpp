#include "util/intersection.h"
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
#include <cmath>
#include "util/ray.h"
#include "util/triangle.h"
#include "util/plane.h"
#include "util/aabb.h"
#include "util/sphere.h"


TWIN_NAMESPACE_BEGIN
    namespace Intersection {
        RayTriangleIntersection RayTriangle(const Triangle &tri, const Ray &ray, float epsilon) {
            return RayTriangle(ray.direction, ray.origin, tri.a, tri.b, tri.c, epsilon);
        }

        RayTriangleIntersection RayTriangle(const cppgl::vec3 &direction, const cppgl::vec3 &origin, const cppgl::vec3 &A, const cppgl::vec3 &B, const cppgl::vec3 &C, float epsilon) {
            RayTriangleIntersection inter;


            cppgl::vec3 e1, e2;  // Edge1, Edge2
            cppgl::vec3 P, Q, T;
            float det, inv_det, u, v;
            float t;

            // Find vectors for two edges sharing V1
            e1 = B - A;
            e2 = C - A;

            // culling
            cppgl::vec3 n = cppgl::cross(e1, e2);
            inter.backFace = cppgl::dot(direction, n) > 0;

            // Begin calculating determinant - also used to calculate u parameter
            P = cppgl::cross(direction, e2);
            // if determinant is near zero, ray lies in plane of triangle
            det = cppgl::dot(e1, P);

            // NOT CULLING
            if (det > -epsilon && det < epsilon) return inter;
            inv_det = 1.f / det;

            // calculate distance from V1 to ray origin
            T = origin - A;

            // Calculate u parameter and test bound
            u = cppgl::dot(T, P) * inv_det;
            // The intersection lies outside of the triangle
            if (u < 0.f || u > 1.f) return inter;

            // Prepare to test v parameter
            Q = cppgl::cross(T, e1);

            // Calculate V parameter and test bound
            v = cppgl::dot(direction, Q) * inv_det;

            // The intersection lies outside of the triangle
            if (v < 0.f || u + v > 1.f) return inter;

            t = cppgl::dot(e2, Q) * inv_det;

            if (t > epsilon) {
                inter.barycenter = cppgl::vec3(1 - u - v, u, v);
                inter.valid = true;
                inter.t = t;
                return inter;
            }

            return inter;
        }

        bool RayPlane(const Ray &r, const Plane &p, float &t) {
            const cppgl::vec3 &direction = r.direction;
            const cppgl::vec3 &origin = r.origin;

            const cppgl::vec3 &N = p.m_normal;
            const cppgl::vec3 &P = p.getPoint();


            const float EPSILON = 0.000001;

            float denom = cppgl::dot(N, direction);

            // Check if ray is parallel to the plane
            if (abs(denom) > EPSILON) {
                t = cppgl::dot(P - origin, N) / denom;
                if (t >= 0) {
                    return true;
                }
            }
            return false;
        }

        bool RayAABB(const cppgl::vec3 &origin, const cppgl::vec3 &direction, const cppgl::vec3 &boxmin, const cppgl::vec3 &boxmax, float &t) {
            using std::max;
            using std::min;
            cppgl::vec3 inv_dir = 1.0 / direction.array();

            // lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
            // r.org is origin of ray
            cppgl::vec3 t_min;
            t_min(0) = (boxmin[0] - origin[0]) * inv_dir[0];
            t_min(1) = (boxmin[1] - origin[1]) * inv_dir[1];
            t_min(2) = (boxmin[2] - origin[2]) * inv_dir[2];
            t_min = (boxmin - origin).array() * inv_dir.array();

            cppgl::vec3 t_max;
            t_max(0) = (boxmax[0] - origin[0]) * inv_dir[0];
            t_max(1) = (boxmax[1] - origin[1]) * inv_dir[1];
            t_max(2) = (boxmax[2] - origin[2]) * inv_dir[2];
            t_max = (boxmax - origin).array() * inv_dir.array();

            // float tmin = std::max(max(min(t_min(0), t_max(0)), std::min(t_min(1), t_max(1))), std::min(t_min(2), t_max(2)));
            // float tmax = std::min(min(max(t_min(0), t_max(0)), std::max(t_min(1), t_max(1))), std::max(t_min(2), t_max(2)));

            float tmin = t_min.array().min(t_max.array()).maxCoeff();
            float tmax = t_min.array().max(t_max.array()).minCoeff();

            // if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
            if (tmax < 0) {
                t = tmax;
                return false;
            }

            // if tmin > tmax, ray doesn't intersect AABB
            if (tmin > tmax) {
                t = tmax;
                return false;
            }

            t = tmin;
            return true;
        }

        bool RayAABB(const Ray &r, const AABB &bb, float &t) {
            return RayAABB(r.origin, r.direction, bb.min, bb.max, t);
        }


        bool PlanePlane(const Plane &p1, const Plane &p2, Ray &outRay) {
            // similar to here
            // https://stackoverflow.com/questions/6408670/line-of-intersection-between-two-planes
            cppgl::vec3 p3_normal = cppgl::cross(p1.m_normal, p2.m_normal);
            float det = cppgl::dot(p3_normal, p3_normal);
            if (det < cppgl::epsilon<float>()) return false;
            cppgl::vec3 outPoint = ((cppgl::cross(p3_normal, p2.m_normal) * p1.m_d) + (cppgl::cross(p1.m_normal, p3_normal) * p2.m_d)) / det;
            outRay = Ray(p3_normal, outPoint);
            return true;
        }

        bool RaySphere(const cppgl::vec3 &rayOrigin, const cppgl::vec3 &rayDir, const cppgl::vec3 &spherePos, float sphereRadius, float &t1, float &t2) {
            cppgl::vec3 L = rayOrigin - spherePos;
            float a = cppgl::dot(rayDir, rayDir);
            float b = 2 * cppgl::dot(rayDir, L);
            float c = cppgl::dot(L, L) - sphereRadius * sphereRadius;
            float D = b * b + (-4.0f) * a * c;

            // rays misses sphere
            if (D < 0) return false;


            if (D == 0) {
                // ray touches sphere
                t1 = t2 = -0.5f * b / a;
            } else {
                // ray interescts sphere
                t1 = -0.5f * (b + std::sqrt(D)) / a;
                t2 = -0.5f * (b - std::sqrt(D)) / a;
            }

            if (t1 > t2) {
                auto tmp = t1;
                t1 = t2;
                t2 = tmp;
                //        std::swap(t1, t2);
            }
            return true;
        }

        bool RaySphere(const Ray &ray, const Sphere &sphere, float &t1, float &t2) {
            return RaySphere(ray.origin, ray.direction, sphere.pos, sphere.r, t1, t2);
        }

        bool SphereSphere(const cppgl::vec3 &c1, float r1, const cppgl::vec3 &c2, float r2) {
            return cppgl::distance(c1, c2) < r1 + r2;
        }

        bool SphereSphere(const Sphere &s1, const Sphere &s2) {
            return SphereSphere(s1.pos, s1.r, s2.pos, s2.r);
        }

        bool AABBAABB(const cppgl::vec3 &min1, const cppgl::vec3 &max1, const cppgl::vec3 &min2, const cppgl::vec3 &max2) {
            if (min1[0] >= max2[0] || max1[0] <= min2[0]) return false;
            if (min1[1] >= max2[1] || max1[1] <= min2[1]) return false;
            if (min1[2] >= max2[2] || max1[2] <= min2[2]) return false;
            return true;
        }

        bool AABBAABB(const AABB &bb1, const AABB &bb2) {
            return AABBAABB(bb1.min, bb1.max, bb2.min, bb2.max);
        }
    }

TWIN_NAMESPACE_END