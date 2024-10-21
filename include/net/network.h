#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_NETWORK_H
#define TWIN_NETWORK_H

#include "twinDefines.h"
#include "drawelement.h"
#include "texture.h"
#include <torch/types.h>
#include "net/vposer.h"
#include "net/constraints/constraints.h"
#include "net/netParams.h"

TWIN_NAMESPACE_BEGIN

    enum Gender : int {
        Neutral = 0,
        Female = 1,
        Male = 2,
        LeftHand = 3,
        RightHand = 4
    };

    class Optimizer;

    class NetworkImpl {
        friend class Optimizer;

    public:
        struct RenderOptions {
            enum RenderChoice : int {
                RenderNormal = 1 << 0,
                RenderTransparent = 1 << 1,
                RenderJoints = 1 << 2,
                RenderSkeleton = 1 << 3,
            } choice = RenderChoice::RenderNormal;
            cppgl::vec4 color = cppgl::vec4(0.8, 0.8, 0.8, 1);
        };
        RenderOptions m_render_option;
        cppgl::Drawelement m_mesh, m_mesh_shaped, m_obj;

        explicit NetworkImpl(const std::string &cname, std::string path, Gender s = Gender::Neutral);


        bool HasUVs() const;

        bool UseUVs() const;

        virtual ~NetworkImpl() = default;

        /**
         * Inference needs to instanciate the variables m_obj, m_obj_joints,  m_obj_skeleton, m_mesh
         */
        virtual void Inference(NetworkParameters &net_param);

        void SaveObj(NetworkParameters &net_param, const std::string &path);

        virtual void drawGUI();

        virtual void SetModel(const cppgl::mat4 &model);

        void SetTranslation(const cppgl::vec3 &translate);

        void SetRotation(const cppgl::vec3 &rotation);

        //Indices is a list triangle indices(max_size:num_indices/3) that are in index buffer of a mesh (tri_index points to for index in index buffer for start of triangle)
        virtual void SetColor(const std::vector<int> &indices, const cppgl::vec4 &color);

        virtual void SetColor(const std::vector<cppgl::vec3> &color);

        virtual void Render(cppgl::mat4 model = cppgl::mat4::Identity());

        //virtual torch::Tensor VShaped(torch::Tensor beta) = 0;

        virtual std::tuple<torch::Tensor, torch::Tensor, torch::Tensor> VPosed(torch::Tensor full_pose, torch::Tensor trans, torch::Tensor beta) = 0;


        virtual torch::Device GetDevice() const = 0;

        virtual torch::Device GetHost() const = 0;

        virtual torch::Tensor Faces() = 0;

        virtual torch::Tensor VTemplate() = 0;

        virtual torch::Tensor Kintree() = 0;

        virtual std::vector<long> Parents() = 0;

        void SetConstraints(Constraints constraints);

        virtual std::vector<twin::Triangle> getTriangleSoup() const;

        virtual void LoadGender(const std::string &path, Gender s) = 0;

        virtual bool VposerPossible() const;

        bool m_gui;

        std::string name;

        Constraints m_constraints;

        std::string m_path;

        virtual void WriteVals(std::ostream &os, twin::NetworkParameters np);

        twin::VPoser m_vposer;

    protected:

        void CreateRenderObjects(std::vector<cppgl::vec3> &&vertices, std::vector<int64_t> &&indices, const std::vector<cppgl::vec3> &Js, const std::vector<cppgl::vec3> &OJs, std::vector<cppgl::vec3> &&vertices_shaped, std::vector<cppgl::vec3> &&normals, std::vector<cppgl::vec3> &&normals_shaped);

        void CreateMesh(cppgl::Drawelement &mesh, cppgl::Drawelement &mesh_shaped, std::vector<cppgl::vec3> &&vertices, std::vector<int64_t> &&indices, const std::vector<cppgl::vec3> &Js, const std::vector<cppgl::vec3> &OJs, std::vector<cppgl::vec3> &&vertices_shaped, std::vector<cppgl::vec3> &&normals_verts, std::vector<cppgl::vec3> &&normals_shaped);

        cppgl::Drawelement CreateMeshTextured(cppgl::Drawelement &mesh);

        std::tuple<std::vector<cppgl::vec3>, std::vector<int64_t>, std::vector<cppgl::vec3>, std::vector<cppgl::vec3>, std::vector<cppgl::vec3>, std::vector<cppgl::vec3>, std::vector<cppgl::vec3>> InferenceHelper(NetworkParameters &net_params);

        cppgl::Drawelement m_obj_joints;//PointCloud
        cppgl::Drawelement m_obj_skeleton;//lines


        std::vector<cppgl::vec2> m_uvs;
        std::vector<cppgl::ivec3> m_uv_ids;

        bool m_render_muscles;
        bool m_render_shaped;
        bool m_use_uvs;
        //TemplatedShaderStorageBuffer<Muscle> opengl_muscles_;
        Gender m_sex;

        cppgl::Texture2D m_texture;

        cppgl::Texture2D GetTexture(bool regenerate);

        int m_id;

        static int c_count;
    };

    using Network = cppgl::NamedHandle<NetworkImpl>;
TWIN_NAMESPACE_END
#endif //TWIN_NETWORK_H
#endif