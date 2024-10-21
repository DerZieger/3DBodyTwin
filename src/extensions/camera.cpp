#include "extensions/camera.h"
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

    CameraImpl::CameraImpl(const std::string &name) : cppgl::CameraImpl(name) {}

    cppgl::vec3 CameraImpl::ImageToNormalized(cppgl::vec2 ip, float depth, int w, int h) const {
        ip[0] /= static_cast<float>(w);
        ip[1] /= static_cast<float>(h);
        ip = (ip - cppgl::make_vec2(0.5f)) * 2.0f;
        cppgl::vec3 p = cppgl::make_vec3(ip, depth);
        return p;
    }

    cppgl::vec2 CameraImpl::NormalizedToImage(const cppgl::vec3 &normalizedPosition, int w, int h) const {
        cppgl::vec2 ip = cppgl::make_vec2(normalizedPosition);
        ip = ip * 0.5f + cppgl::make_vec2(0.5f);
        ip[0] *= static_cast<float>(w);
        ip[1] *= static_cast<float>(h);
        return ip;
    }

    cppgl::vec3 CameraImpl::NormalizedToView(const cppgl::vec3 &normalizedPosition) const {
        cppgl::vec4 p = cppgl::inverse(proj) * cppgl::make_vec4(normalizedPosition, 1);
        p /= p[3];
        return cppgl::make_vec3(p);
    }

    cppgl::vec3 CameraImpl::ViewToWorld(const cppgl::vec3 &viewPosition) const {
        cppgl::vec4 p = cppgl::inverse(view) * cppgl::make_vec4(viewPosition, 1);
        return cppgl::make_vec3(p);
    }

    Ray CameraImpl::PixelRay(cppgl::vec2 pixel, int w, int h, bool flip_y) {
        if (flip_y) {
            pixel.y() = static_cast<float>(h) - pixel.y();
        }

        cppgl::vec3 p = ViewToWorld(NormalizedToView(ImageToNormalized(pixel, 1, w, h)));
        Ray r((p - pos).normalized(), pos);
        return r;
    }

TWIN_NAMESPACE_END