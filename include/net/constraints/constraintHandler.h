#pragma once

#ifdef HAS_DL_MODULE
#ifndef TWIN_CONSTRAINTHANDLER_H
#define TWIN_CONSTRAINTHANDLER_H

#include "twinDefines.h"
#include <cppgl.h>
#include "net/network.h"
#include "net/constraints/constraints.h"
#include "util/bvh.h"
#include "util/ray.h"
#include "osim.h"
#include "c3d.h"
#include "jsonMarkers.h"

TWIN_NAMESPACE_BEGIN
    class ConstraintHandler {
    public:
        explicit ConstraintHandler(std::string config = std::string("data/default_constraints.json"));

        void Clear();

        void
        Refresh(std::vector<std::shared_ptr<Osim>> &osim, std::vector<std::shared_ptr<C3D>> &c3d, std::vector<std::shared_ptr<JMarker>> &mjs, const cppgl::mat4 &model, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped);

        void RenderImGui(Network &net, std::vector<std::shared_ptr<Osim>> &osim, std::vector<std::shared_ptr<C3D>> &c3d, std::vector<std::shared_ptr<JMarker>> &mjs, Constraints &constraints, cppgl::mat4 model, BVH &bvh, const Ray &ray, const cppgl::vec2 &local_pixel, bool pressed, bool has_changed, int &zeroFrame, int &maxFramerate);

        void AddNewDefaultConstraint(Constraint con);

        void RemoveDefaultConstraint(std::string con);

        void Render(cppgl::mat4 model = cppgl::mat4::Identity());

        void RecomputeRendering(const cppgl::Drawelement &mesh, bool mesh_changed);

        void SetFrame(int frame = -1);

        static void drawClassGUI();

        int getFramerate() const;


        // id -> tuple containing <active, rendering, con>
        std::map<int, std::tuple<bool, bool, Constraint>> m_con_map;
        nlohmann::json m_json;
    private:
        std::string m_path;
        int m_configuring;
        bool m_resetCol;
        std::shared_ptr<ConstraintImpl> m_new_constraint;
        int m_frame;
        int m_id;
        int m_selection;
        static int c_count;
        static bool c_gui;

        void Load();

        void Save();


        // name -> id
        std::map<std::string, int> m_id_map;
        // id -> name
        std::map<int, std::string> m_name_map;
    };

TWIN_NAMESPACE_END
#endif //TWIN_CONSTRAINTHANDLER_H
#endif