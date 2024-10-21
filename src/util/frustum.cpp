#include "util/frustum.h"
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

    Frustum::Frustum(const cppgl::mat4 &model, float fovy, float aspect, float zNear, float zFar, bool negativ_z, bool negative_y) {
        using namespace cppgl;
        float tang = static_cast<float>(tan(fovy * 0.5));

        vec3 position = make_vec3(model.col(3));

        vec3 right = make_vec3(model.col(0));
        vec3 up = make_vec3(model.col(1));
        vec3 dir = make_vec3(model.col(2));

        if (negative_y) {
            up = -up;
        }


        if (negativ_z) {
            dir = -dir;
        }

        vec3 nearplanepos = position + dir * zNear;
        vec3 farplanepos = position + dir * zFar;

        // near plane
        planes[0] = Plane(nearplanepos, -dir);
        // far plane
        planes[1] = Plane(farplanepos, dir);


        float nh = zNear * tang;
        float nw = nh * aspect;
        float fh = zFar * tang;
        float fw = fh * aspect;

        // calcuate 4 corners of nearplane
        vertices[0] = nearplanepos + nh * up - nw * right;
        vertices[1] = nearplanepos + nh * up + nw * right;
        vertices[2] = nearplanepos - nh * up - nw * right;
        vertices[3] = nearplanepos - nh * up + nw * right;
        // calcuate 4 corners of farplane
        vertices[4] = farplanepos + fh * up - fw * right;
        vertices[5] = farplanepos + fh * up + fw * right;
        vertices[6] = farplanepos - fh * up - fw * right;
        vertices[7] = farplanepos - fh * up + fw * right;

        // side planes
        planes[2] = Plane(position, vertices[1], vertices[0]);  // top
        planes[3] = Plane(position, vertices[2], vertices[3]);  // bottom
        planes[4] = Plane(position, vertices[0], vertices[2]);  // left
        planes[5] = Plane(position, vertices[3], vertices[1]);  // right


        //    vec3 fbr = farplanepos - fh * up + fw * right;
        //    vec3 fbr = farplanepos - fh * up;
        vec3 fbr = vertices[4];
        vec3 sphereMid = (nearplanepos + farplanepos) * 0.5f;
        float r = distance(fbr, sphereMid);

        boundingSphere.r = r;
        boundingSphere.pos = sphereMid;
    }

    Frustum Frustum::SubFrustum(float left_begin, float right_end, float top_begin, float bot_end) const {
        Frustum ret = *this;
        // far plane

        // move vertices along horizontal line
        cppgl::vec3 verts[] = {
                vertices[0] * (1 - left_begin) + vertices[1] * left_begin,
                vertices[2] * (1 - left_begin) + vertices[3] * left_begin,
                vertices[0] * (1 - right_end) + vertices[1] * right_end,
                vertices[2] * (1 - right_end) + vertices[3] * right_end,
                vertices[4] * (1 - left_begin) + vertices[5] * left_begin,
                vertices[6] * (1 - left_begin) + vertices[7] * left_begin,
                vertices[4] * (1 - right_end) + vertices[5] * right_end,
                vertices[6] * (1 - right_end) + vertices[7] * right_end,
        };

        // move vertices along vertical line
        ret.vertices[0] = verts[0] * (1 - top_begin) + verts[1] * top_begin;
        ret.vertices[1] = verts[2] * (1 - top_begin) + verts[3] * top_begin;
        ret.vertices[2] = verts[0] * (1 - bot_end) + verts[1] * bot_end;
        ret.vertices[3] = verts[2] * (1 - bot_end) + verts[3] * bot_end;
        ret.vertices[4] = verts[4] * (1 - top_begin) + verts[5] * top_begin;
        ret.vertices[5] = verts[6] * (1 - top_begin) + verts[7] * top_begin;
        ret.vertices[6] = verts[4] * (1 - bot_end) + verts[5] * bot_end;
        ret.vertices[7] = verts[6] * (1 - bot_end) + verts[7] * bot_end;

        ret.planes[2] = Plane(ret.vertices[0], ret.vertices[5], ret.vertices[4]);  // top
        ret.planes[3] = Plane(ret.vertices[3], ret.vertices[6], ret.vertices[7]);  // bottom
        ret.planes[4] = Plane(ret.vertices[2], ret.vertices[4], ret.vertices[6]);  // left
        ret.planes[5] = Plane(ret.vertices[1], ret.vertices[7], ret.vertices[5]);  // right

        cppgl::vec3 fbr = ret.vertices[4];
        cppgl::vec3 sphereMid = 0.25 * (ret.vertices[0] + ret.vertices[3] + ret.vertices[5] + ret.vertices[7]);
        float r = cppgl::distance(fbr, sphereMid);

        ret.boundingSphere.r = r;
        ret.boundingSphere.pos = sphereMid;

        return ret;
    }

    void Frustum::computePlanesFromVertices() {
        // side planes
        planes[0] = Plane(vertices[0], vertices[2], vertices[1]);  // near
        planes[1] = Plane(vertices[4], vertices[5], vertices[7]);  // far

        planes[2] = Plane(vertices[0], vertices[1], vertices[4]);  // top
        planes[3] = Plane(vertices[2], vertices[6], vertices[3]);  // bottom
        planes[4] = Plane(vertices[0], vertices[4], vertices[2]);  // left
        planes[5] = Plane(vertices[1], vertices[3], vertices[7]);  // right
    }

    void Frustum::computePlanesFromMatrices(const cppgl::mat4 &proj, const cppgl::mat4 &view) {
        cppgl::vec3 pointsClipSpace[] = {cppgl::vec3(-1, 1, -1), cppgl::vec3(1, 1, -1), cppgl::vec3(-1, -1, -1), cppgl::vec3(1, -1, -1), cppgl::vec3(-1, 1, 1), cppgl::vec3(1, 1, 1), cppgl::vec3(-1, -1, 1), cppgl::vec3(1, -1, 1)};

        cppgl::mat4 m = cppgl::inverse(cppgl::mat4(proj * view));
        for (int i = 0; i < 8; ++i) {
            cppgl::vec4 p = m * cppgl::make_vec4(pointsClipSpace[i], 1);
            p = p / p[3];
            vertices[i] = cppgl::make_vec3(p);
        }

        // side planes
        planes[0] = Plane(vertices[0], vertices[2], vertices[1]);  // near
        planes[1] = Plane(vertices[4], vertices[5], vertices[7]);  // far

        planes[2] = Plane(vertices[0], vertices[1], vertices[4]);  // top
        planes[3] = Plane(vertices[2], vertices[6], vertices[3]);  // bottom
        planes[4] = Plane(vertices[0], vertices[4], vertices[2]);  // left
        planes[5] = Plane(vertices[1], vertices[3], vertices[7]);  // right
    }

    Frustum::IntersectionResult Frustum::pointInFrustum(const cppgl::vec3 &p) const {
        for (int i = 0; i < 6; i++) {
            if (planes[i].distance(p) > 0) return OUTSIDE;
        }
        return INSIDE;
    }

    Frustum::IntersectionResult Frustum::sphereInFrustum(const Sphere &s) const {
        IntersectionResult result = INSIDE;
        float distance;

        for (int i = 0; i < 6; i++) {
            distance = planes[i].distance(s.pos);
            if (distance >= s.r) {
                return OUTSIDE;
            } else if (distance > -s.r)
                result = INTERSECT;
        }
        return result;
    }

    Frustum::IntersectionResult Frustum::pointInSphereFrustum(const cppgl::vec3 &p) const {
        if (boundingSphere.contains(p)) {
            return INSIDE;
        } else {
            return OUTSIDE;
        }
    }

    Frustum::IntersectionResult Frustum::sphereInSphereFrustum(const Sphere &s) const {
        if (boundingSphere.intersect(s)) {
            return INSIDE;
        } else {
            return OUTSIDE;
        }
    }

    std::array<Triangle, 12> Frustum::ToTriangleList() const {
        std::array<Triangle, 12> result;

        // near
        result[0] = Triangle(vertices[0], vertices[3], vertices[1]);
        result[1] = Triangle(vertices[0], vertices[2], vertices[3]);
        // far
        result[2] = Triangle(vertices[4], vertices[5], vertices[7]);
        result[3] = Triangle(vertices[4], vertices[7], vertices[6]);
        // top
        result[4] = Triangle(vertices[0], vertices[5], vertices[4]);
        result[5] = Triangle(vertices[0], vertices[1], vertices[5]);
        // bottom
        result[6] = Triangle(vertices[2], vertices[6], vertices[7]);
        result[7] = Triangle(vertices[3], vertices[2], vertices[7]);
        // left
        result[8] = Triangle(vertices[0], vertices[4], vertices[6]);
        result[9] = Triangle(vertices[2], vertices[0], vertices[6]);
        // right
        result[10] = Triangle(vertices[1], vertices[7], vertices[5]);
        result[11] = Triangle(vertices[1], vertices[3], vertices[7]);

        return result;
    }

    cppgl::vec2 Frustum::projectedIntervall(const cppgl::vec3 &d) const {
        cppgl::vec2 ret(1000000, -1000000);
        for (int i = 0; i < 8; ++i) {
            float t = cppgl::dot(d, vertices[i]);
            ret[0] = std::min(ret[0], t);
            ret[1] = std::max(ret[1], t);
        }
        return ret;
    }

    int Frustum::sideOfPlane(const Plane &plane) const {
        int positive = 0, negative = 0;
        for (int i = 0; i < 8; ++i) {
            float t = plane.distance(vertices[i]);
            if (t > 0)
                positive++;
            else if (t < 0)
                negative++;
            if (positive && negative) return 0;
        }
        return (positive) ? 1 : -1;
    }

    std::pair<cppgl::vec3, cppgl::vec3> Frustum::getEdge(int i) const {
        using namespace cppgl;
        switch (i) {
            case 0:
                return std::pair<vec3, vec3>(vertices[0], vertices[4]); // nTL - fTL
            case 1:
                return std::pair<vec3, vec3>(vertices[1], vertices[5]); // nTR - fTR
            case 2:
                return std::pair<vec3, vec3>(vertices[2], vertices[6]); // nBL - fBL
            case 3:
                return std::pair<vec3, vec3>(vertices[3], vertices[7]); // nBR - fBR
            case 4:
                return std::pair<vec3, vec3>(vertices[0], vertices[1]); // nTL - nTR
            case 5:
                return std::pair<vec3, vec3>(vertices[0], vertices[2]); // nTL - nBL
            case 6:
                return std::pair<vec3, vec3>(vertices[3], vertices[2]); // nBR - nBL
            case 7:
                return std::pair<vec3, vec3>(vertices[3], vertices[1]); // nBR - nTR
            case 8:
                return std::pair<vec3, vec3>(vertices[4], vertices[5]); // fTL - fTR
            case 9:
                return std::pair<vec3, vec3>(vertices[4], vertices[6]); // fTL - fBL
            case 10:
                return std::pair<vec3, vec3>(vertices[7], vertices[6]); // fBR - fBL
            case 11:
                return std::pair<vec3, vec3>(vertices[7], vertices[5]); // fBR - fTR
            default:
                std::cerr << "Camera::getEdge" << std::endl;
                return std::pair<vec3, vec3>();
        }
    }

    bool Frustum::intersectSAT(const Frustum &other) const {
        // check planes of this camera
        for (int i = 0; i < 6; ++i) {
            if (other.sideOfPlane(planes[i]) > 0) {  // other is entirely on positive side
                return false;
            }
        }

        // check planes of other camera
        for (int i = 0; i < 6; ++i) {
            if (this->sideOfPlane(other.planes[i]) > 0) {  // this is entirely on positive side
                return false;
            }
        }


        // test cross product of pairs of edges, one from each polyhedron
        // since the overlap of the projected intervall is checked parallel edges doesn't have to be tested
        // -> 6 edges for each frustum
        for (int i = 0; i < 6; ++i) {
            std::pair<cppgl::vec3, cppgl::vec3> e1 = this->getEdge(i);
            for (int j = 0; j < 6; ++j) {
                std::pair<cppgl::vec3, cppgl::vec3> e2 = other.getEdge(j);
                cppgl::vec3 d = cppgl::cross(e1.first - e1.second, e2.first - e2.second);

                cppgl::vec2 i1 = this->projectedIntervall(d);
                cppgl::vec2 i2 = other.projectedIntervall(d);

                if (i1[0] > i2[1] || i1[1] < i2[0]) return false;
            }
        }

        return true;
    }

    bool Frustum::intersectSAT(const Sphere &s) const {
        using namespace cppgl;
        for (int i = 0; i < 6; ++i) {
            if (planes[i].distance(s.pos) >= s.r) {
                return false;
            }
        }

        for (int i = 0; i < 8; ++i) {
            const vec3 &v = vertices[i];
            vec3 d = (v - s.pos).normalized();
            vec2 i1 = this->projectedIntervall(d);
            vec2 i2 = s.projectedIntervall(d);
            if (i1[0] > i2[1] || i1[1] < i2[0])
                return false;
        }

        for (int i = 0; i < 12; ++i) {
            std::pair<vec3, vec3> edge = this->getEdge(i);
            vec3 A = edge.first;
            vec3 B = edge.second;
            vec3 AP = (s.pos - A);
            vec3 AB = (B - A);
            vec3 closestOnEdge = A + dot(AP, AB) / dot(AB, AB) * AB;

            vec3 d = (closestOnEdge - s.pos).normalized();
            vec2 i1 = this->projectedIntervall(d);
            vec2 i2 = s.projectedIntervall(d);
            if (i1[0] > i2[1] || i1[1] < i2[0])
                return false;
        }

        return true;
    }

    std::ostream &operator<<(std::ostream &os, const Frustum &frustum) {
        os << "[Frustum]" << std::endl;
        for (const Plane &p: frustum.planes) {
            os << p << std::endl;
        }
        for (const cppgl::vec3 &v: frustum.vertices) {
            os << v.transpose() << std::endl;
        }
        return os;
    }

TWIN_NAMESPACE_END

