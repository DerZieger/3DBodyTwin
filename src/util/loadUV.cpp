#include "util/loadUV.h"
#include <fstream>

twin::LoadUV::LoadUV(const std::string &path) {
    std::ifstream in(path);
    std::string line;

    enum {
        START, UVS, INDICES
    } mode = START;

    unsigned long int uvs, faces;
    while (std::getline(in, line)) {
        unsigned long int idx = line.find_first_not_of(' ');
        if (idx >= line.size()) {
            continue;
        }
        if (line[idx] == '#') {
            continue;
        }

        std::istringstream iss(line);
        if (mode == START) {
            iss >> uvs >> faces;
            m_uvs.reserve(uvs);
            m_faces.reserve(faces);
            mode = UVS;
        } else if (mode == UVS) {
            cppgl::vec2 uv;
            iss >> uv[0] >> uv[1];
            m_uvs.push_back(uv);

            if (m_uvs.size() == uvs) {
                mode = INDICES;
            }
        } else if (mode == INDICES) {
            cppgl::ivec3 ind;
            iss >> ind[0] >> ind[1] >> ind[2];
            ind -= cppgl::ivec3(1, 1, 1);
            m_faces.push_back(ind);
        }
    }
}
