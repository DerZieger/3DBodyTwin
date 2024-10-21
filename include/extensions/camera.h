#pragma once

#ifndef TWIN_CAMERA_H
#define TWIN_CAMERA_H
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
#include "twinDefines.h"
#include <cppgl.h>
#include "util/ray.h"

TWIN_NAMESPACE_BEGIN

    class CameraImpl : public cppgl::CameraImpl {
    public:
        explicit CameraImpl(const std::string &name);

        cppgl::vec3 ImageToNormalized(cppgl::vec2 ip, float depth, int w, int h) const;

        cppgl::vec2 NormalizedToImage(const cppgl::vec3 &normalizedPosition, int w, int h) const;

        cppgl::vec3 NormalizedToView(const cppgl::vec3 &normalizedPosition) const;

        cppgl::vec3 ViewToWorld(const cppgl::vec3 &viewPosition) const;

        Ray PixelRay(cppgl::vec2 pixel, int w, int h, bool flip_y);
    };

    using Camera = cppgl::NamedHandle<CameraImpl>;
TWIN_NAMESPACE_END

#endif //TWIN_CAMERA_H
