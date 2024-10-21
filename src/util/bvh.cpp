#include "util/bvh.h"
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

    BVH::BVH(int leafs, float bvh_eps) : BVH(std::vector<Triangle>(), leafs, bvh_eps) {}

    BVH::BVH(const std::vector<Triangle> &inTris, int leafs, float bvh_eps) : nodes(std::vector<BVHNode>()), next_tris(std::vector<Triangle>()), leafTris(leafs), bvh_epsilon(bvh_eps) {
        tris.reserve(inTris.size());
        for (size_t i = 0; i < inTris.size(); ++i) {
            tris.emplace_back(inTris.at(i), i);
        }
    }

    AABB BVH::computeBox(int start, int end) const {
        AABB box;
        box.invalidate();
        for (int i = start; i < end; ++i) {
            auto &t = tris.at(i).first;
            box.growBox(t.a);
            box.growBox(t.b);
            box.growBox(t.c);
        }

        box.min -= cppgl::vec3(bvh_epsilon, bvh_epsilon, bvh_epsilon);
        box.max += cppgl::vec3(bvh_epsilon, bvh_epsilon, bvh_epsilon);
        return box;
    }

    void BVH::construct() {
        nodes.clear();
        nodes.reserve(tris.size());
        construct(0, static_cast<int>(tris.size()));
    }

    void BVH::construct(std::vector<Triangle> &inTris) {
        tris.clear();
        tris.reserve(inTris.size());
        for (size_t i = 0; i < inTris.size(); ++i) {
            tris.emplace_back(inTris.at(i), i);
        }
        nodes.clear();
        nodes.reserve(tris.size());
        construct(0, static_cast<int>(tris.size()));
    }

    int BVH::construct(int start, int end) {
        int nodeid = static_cast<int>(nodes.size());
        nodes.emplace_back();
        BVHNode &node = nodes.back();

        node.box = computeBox(start, end);

        if (end - start <= leafTris) {
            // leaf node
            node.isLeaf = true;
            node.left = start;
            node.right = end;
        } else {
            node.isLeaf = false;
            int axis = node.box.biggestSpan();
            std::sort(tris.begin() + start, tris.begin() + end, Axis(axis));

            int mid = (start + end) / 2;

            int l = construct(start, mid);
            int r = construct(mid, end);

            // reload node, because the reference from above might be broken
            BVHNode &node2 = nodes.at(nodeid);
            node2.left = l;
            node2.right = r;
        }

        return nodeid;
    }

    void BVH::setNextTris(const std::vector<Triangle> &triangles) {
        next_tris = triangles;
    }

    void BVH::setNextTris(const std::vector<Triangle> &&triangles) {
        next_tris = triangles;
    }

    void BVH::Update() {
        if (next_tris.empty())return;
        construct(next_tris);
        next_tris.clear();
    }

    std::ostream &operator<<(std::ostream &os, const BVH &dt) {
        os << "[BVH] Trianglecount: " << dt.tris.size() << " Nodecount " << dt.nodes.size() << " leaves: " << dt.leafTris;
        return os;
    }

    Intersection::RayTriangleIntersection BVH::getClosest(const Ray &ray) {
        Intersection::RayTriangleIntersection result;
        if (!nodes.empty())
            getClosest(0, ray, result);
        return result;
    }

    void BVH::getClosest(uint32_t node, const Ray &ray, Intersection::RayTriangleIntersection &result) const {
        if (node >= nodes.size())return;

        const BVHNode &n = nodes.at(node);

        float aabbT;

        // The ray missed the box
        if (!n.box.rayHit(ray, aabbT)) return;

        // The node is further than the closest hit
        if (aabbT > result.t) return;

        if (!n.isLeaf) {
            getClosest(n.left, ray, result);
            getClosest(n.right, ray, result);
        } else {
            // Leaf node -> intersect with triangles
            for (uint32_t i = n.left; i < n.right; ++i) {
                Intersection::RayTriangleIntersection inter = tris.at(i).first.rayHit(ray);
                if (inter && inter < result) {
                    inter.triangleIndex = tris.at(i).second;
                    result = inter;
                }
            }
        }
    }


TWIN_NAMESPACE_END