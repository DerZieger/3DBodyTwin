#pragma once

#include "twinDefines.h"
#include <string>
#include "data_types.h"

TWIN_NAMESPACE_BEGIN
    class twinParams {
    public:
        twinParams() : cmdOnly(false), cfgP("."), outDir("./out"), inDir("./in"), height(1080), width(1920), title("twinVis"), interval(1), osimMod(""), osimSca(""), osimMot(""), osimSto(""), camPos(cppgl::vec3(0, 0, 0)), camDir(cppgl::vec3(1, 0, 0)), camUp(cppgl::vec3(0, 1, 0)), defCon(""), c3dP(""), suprP(""), evalS("") {};

        bool cmdOnly;
        std::string cfgP;
        std::string outDir;
        std::string inDir;
        int height;
        int width;
        std::string title;
        int interval;
        std::string osimMod;
        std::string osimSca;
        std::string osimMot;
        std::string osimSto;
        cppgl::vec3 camPos;
        cppgl::vec3 camDir;
        cppgl::vec3 camUp;
        std::string defCon;
        std::string c3dP;
        std::string suprP;
        std::string evalS;
    };

TWIN_NAMESPACE_END