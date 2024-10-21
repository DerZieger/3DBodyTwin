#pragma once

#ifndef TWIN_TWINVIEWER_H
#define TWIN_TWINVIEWER_H

#include "twinDefines.h"
#include "twinParams.h"
#include <cppgl.h>
#include "osim.h"
#include "net/supr/supr.h"
#include "net/optimizer.h"
#include "util/interaction.h"
#include "net/constraints/constraintHandler.h"
#include "c3d.h"
#include "jsonMarkers.h"

TWIN_NAMESPACE_BEGIN

    enum DatasetType : int {
        NONE = -1,
        DT_C3D = 0,
        DT_OSIM = 1,
        DT_JSON = 2
    };

    template<typename T>
    inline void MouseButtonCallbackWrapper(int button, int action, int mods, void *context) {
        static_cast<T *>(context)->mouse_button_callback(button, action, mods);
    }

    template<typename T>
    inline void KeyboardCallbackWrapper(int key, int scancode, int action, int mods, void *context) {
        static_cast<T *>(context)->keyboard_callback(key, scancode, action, mods);
    }

    template<typename T>
    inline void ResizeCallbackWrapper(int w, int h, void *context) {
        static_cast<T *>(context)->resize_callback(w, h);
    }

    template<typename T>
    inline void MouseScrollCallbackWrapper(double xoffset, double yoffset, void *context) {
        static_cast<T *>(context)->mouse_scroll_callback(xoffset, yoffset);
    }

    template<typename T>
    inline void MousePosCallbackWrapper(double xpos, double ypos, void *context) {
        static_cast<T *>(context)->mouse_pos_callback(xpos, ypos);
    }

#ifdef HAS_DL_MODULE
    struct OptimizerSet {
        std::shared_ptr<Optimizer> opti;
        ConstraintHandler consHand;
        std::shared_ptr<OptimizationParams> optP;
        Network netz;
        NetworkParameters netzP;
        BVH bvh;
        cppgl::vec3 trans;
        cppgl::vec3 pivot;
        cppgl::vec3 rot;
        int zeroFrame;
    };
#endif

    class TwinViewer {
    public:
        TwinViewer(int argc, char *argv[]);

        ~TwinViewer();

        void setupWindow();

        twinParams parseCmd(int argc, char *argv[]);

        void writeConfig();

        void drawGUI();

        static void drawClassGUI();

        static void drawConfigGUI();

        void run();

        void mouse_button_callback(int button, int action, int mods);

        void keyboard_callback(int key, int scancode, int action, int mods);

        void resize_callback(int w, int h);

        void mouse_scroll_callback(double xoffset, double yoffset);

        void mouse_pos_callback(double xpo, double ypos);

    private:

        twinParams m_params;

        Interaction m_interact;

        std::vector<std::shared_ptr<Osim>> m_osim;

        std::vector<std::shared_ptr<C3D>> m_c3d;

        std::vector<std::shared_ptr<JMarker>> m_mjs;

        static bool c_guiconf;

        int m_frame;
        int m_prevFrame;

        bool m_record;
        int m_recordframe;

        bool m_autoplay;

        bool m_syncplay;
        double m_virtTime;
        int m_maxFramerate;

        int m_frameadv;

        int m_minFrame;
        int m_maxFrame;

        DatasetType m_dataset;

        std::string m_osimstr, m_motstr, m_stostr, m_scastr;
        bool m_mot, m_sca, m_sto;

        bool m_zeroed;

        bool m_swap;

        static bool c_gui;

        cppgl::Shader blit;
        cppgl::Framebuffer fbo;

        void refreshFrame();

        void addOptimizerset();

        void addC3D(const std::string &path);

        void addJMarker(const std::string &path);

        void addOsim(const std::string &filepath_osim, const std::string &filepath_mot = "", const std::string &filepath_sto = "", const std::string &filepath_scale = "");

        void datasetGUI();

#ifdef HAS_DL_MODULE
        std::vector<OptimizerSet> m_optis;
#endif

    };

TWIN_NAMESPACE_END

#endif //TWIN_TWINVIEWER_H
