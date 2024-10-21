#include "gui.h"

TWIN_NAMESPACE_BEGIN

    bool SliderReset(const std::string &str, float *data, float min, float max, bool &ret) {
        ImGui::PushID(str.c_str());
        if (ImGui::Button("Reset##")) {
            ret = true;
            data[0] = 0;
            data[1] = 0;
            data[2] = 0;
        }
        ImGui::SameLine();
        if (ImGui::SliderFloat3(_labelPrefix(str).c_str(), data, min, max)) ret = true;


        ImGui::PopID();
        return ret;
    }

TWIN_NAMESPACE_END