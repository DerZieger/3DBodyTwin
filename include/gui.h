#pragma once
#ifndef TWIN_GUI_H
#define TWIN_GUI_H

#include "twinDefines.h"
#include "cppgl.h"

TWIN_NAMESPACE_BEGIN

// Puts the imgui text on the left side of the input field
// https://github.com/libigl/libigl/issues/1300
    inline std::string _labelPrefix(const std::string &label1) {
        const char *label = label1.c_str();
        float width = ImGui::CalcItemWidth();

        float x = ImGui::GetCursorPosX();
        ImGui::Text("%s", label);
        ImGui::SameLine();
        ImGui::SetCursorPosX(x + width * 0.5f + ImGui::GetStyle().ItemInnerSpacing.x);
        ImGui::SetNextItemWidth(-1);

        std::string labelID = "##";
        labelID += label;

        return labelID;
    }

// Wrapper functions for membercallback function for gui
// WARNING: callfunction must be named drawGUI to work and has to be public
    template<typename T>
    inline void GuiCallbackWrapper(void *context) {
        static_cast<T *>(context)->drawGUI();
    }

    bool SliderReset(const std::string &str, float *data, float min, float max, bool &ret);

TWIN_NAMESPACE_END

#include <imgui/imgui_internal.h>

namespace ImGui {
    inline bool CustomSliderFloat2(const char *label, void *v0, void *v1, float v_min, float v_max, const char *format = "%.3f", ImGuiSliderFlags flags = 0) {
        ImGuiWindow *window = ImGui::GetCurrentWindow();
        if (window->SkipItems)
            return false;

        ImGuiContext &g = *ImGui::GetCurrentContext();
        bool value_changed = false;
        BeginGroup();
        PushID(label);
        PushMultiItemsWidths(2, CalcItemWidth());

        PushID(0);
        value_changed |= SliderScalar("", ImGuiDataType_Float, v0, &v_min, &v_max, format, flags);
        PopID();
        PopItemWidth();
        PushID(1);
        SameLine(0, g.Style.ItemInnerSpacing.x);
        value_changed |= SliderScalar("", ImGuiDataType_Float, v1, &v_min, &v_max, format, flags);
        PopID();
        PopItemWidth();
        PopID();

        const char *label_end = FindRenderedTextEnd(label);
        if (label != label_end) {
            SameLine(0, g.Style.ItemInnerSpacing.x);
            TextEx(label, label_end);
        }

        EndGroup();
        return value_changed;
    };
}

#endif