#pragma once
#ifndef TWIN_LOADUV_H
#define TWIN_LOADUV_H
#include "twinDefines.h"
#include <cppgl.h>

TWIN_NAMESPACE_BEGIN

class LoadUV{
public:
    LoadUV(const std::string& path);
    std::vector<cppgl::vec2> m_uvs;
    std::vector<cppgl::ivec3> m_faces;
};

TWIN_NAMESPACE_END
#endif //TWIN_LOADUV_H
