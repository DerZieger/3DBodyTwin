#ifdef HAS_DL_MODULE

#include "net/network.h"

#include <utility>

#include "util/loadUV.h"
#include "gui.h"
#include "net/constraints/distance.h"
#include "net/constraints/convex_circumference.h"

int twin::NetworkImpl::c_count = 0;

twin::NetworkImpl::NetworkImpl(const std::string &cname, std::string path, Gender s) : m_gui(false), name(cname + std::to_string(c_count)), m_path(std::move(path)), m_vposer(name + "_vp", "data/vposer.npz"), m_uvs({}), m_uv_ids({}), m_render_muscles(false), m_render_shaped(false), m_use_uvs(false), m_sex(s), m_id(c_count++) {}

bool twin::NetworkImpl::HasUVs() const {
    return !m_uvs.empty();
}

bool twin::NetworkImpl::UseUVs() const {
    return HasUVs() && m_use_uvs;
}


void twin::NetworkImpl::Inference(twin::NetworkParameters &net_param) {
    auto [vertices, indices, Js, OJs, vertices_shaped, normals, normals_shaped] = InferenceHelper(net_param);
    CreateRenderObjects(std::move(vertices), std::move(indices), Js, OJs, std::move(vertices_shaped), std::move(normals), std::move(normals_shaped));
}

void twin::NetworkImpl::SaveObj(twin::NetworkParameters &net_param, const std::string &path) {
    std::filesystem::create_directories(std::filesystem::path(path).remove_filename());
    auto [vertices, indices, Js, OJs, vertices_shaped, normals, normals_shaped] = InferenceHelper(net_param);
    cppgl::Drawelement mesh, mesh_shaped;
    CreateMesh(mesh, mesh_shaped, std::move(vertices), std::move(indices), Js, OJs, std::move(vertices_shaped), std::move(normals), std::move(normals_shaped));
    cppgl::save_mesh_cpu(mesh->mesh, path, "");
}

void twin::NetworkImpl::drawGUI() {
    ImGui::PushID("NetworkGui");
    ImGui::PushID(m_id);
    ImGui::Indent(2);
    if (ImGui::CollapsingHeader("Render Options##")) {
        ImGui::Indent(2);
        ImGui::PushID("Network");
        auto check = [&ro = m_render_option](const std::string &str, RenderOptions::RenderChoice option) {
            bool rs = ro.choice & option;
            ImGui::Checkbox(_labelPrefix(str).c_str(), &rs);
            ro.choice = RenderOptions::RenderChoice(rs ? ro.choice | option : ro.choice & ~option);
        };
        int type = static_cast<int>(m_sex);
        const char *model_type[] = {"Neutral", "Female", "Male", "LeftHand", "RightHand"};
        if (ImGui::Combo("Model", &type, model_type, 5)) {
            LoadGender(m_path, Gender(type));
        }
        check("Render", RenderOptions::RenderChoice::RenderNormal);
        check("Render Transparent", RenderOptions::RenderChoice::RenderTransparent);
        check("Render Joints", RenderOptions::RenderChoice::RenderJoints);
        check("Render Skeleton", RenderOptions::RenderChoice::RenderSkeleton);
        if (ImGui::SliderFloat4(_labelPrefix("Color").c_str(), m_render_option.color.data(), 0, 1)) {
            SetColor({}, m_render_option.color);
        }
        ImGui::Checkbox("Use UV", &m_use_uvs);
        ImGui::Checkbox("Muscles", &m_render_muscles);
        ImGui::Checkbox("Render only shaped", &m_render_shaped);
        ImGui::PopID();
        ImGui::Unindent(2);
    }
    ImGui::Unindent(2);
    ImGui::PopID();
    ImGui::PopID();
}

void twin::NetworkImpl::SetModel(const cppgl::mat4 &model) {
    m_obj->set_model_transform(model);
    m_obj_joints->set_model_transform(model);
    m_obj_skeleton->set_model_transform(model);
}

void twin::NetworkImpl::SetTranslation(const cppgl::vec3 &translate) {
    m_obj->set_model_translation(translate);
    m_obj_joints->set_model_translation(translate);
    m_obj_skeleton->set_model_translation(translate);
}

void twin::NetworkImpl::SetRotation(const cppgl::vec3 &rotation) {
    cppgl::mat3 rot;
    float sx = std::sin(rotation.x());
    float cx = std::cos(rotation.x());
    float sy = std::sin(rotation.y());
    float cy = std::cos(rotation.y());
    float sz = std::sin(rotation.z());
    float cz = std::cos(rotation.z());
    rot(0, 0) = cz * cy;
    rot(0, 1) = cz * sy * sx - sz * cx;
    rot(0, 2) = cz * sy * cx + sz * sx;
    rot(1, 0) = sz * cy;
    rot(1, 1) = sz * sy * sx + cz * cx;
    rot(1, 2) = sz * sy * cx - cz * sx;
    rot(2, 0) = -sy;
    rot(2, 1) = cy * sx;
    rot(2, 2) = cy * cx;
    m_obj->set_model_rotation(rot);
    m_obj_joints->set_model_rotation(rot);
    m_obj_skeleton->set_model_rotation(rot);
}

void twin::NetworkImpl::SetColor(const std::vector<int> &indices, const cppgl::vec4 &color) {
    if (m_obj) {
        std::vector<cppgl::vec4> col(m_obj->mesh->num_vertices, m_render_option.color);
        for (int ind: indices) {
            if (ind * 3 + 2 < static_cast<int>(m_obj->mesh->num_indices)) {
                col.at(m_obj->mesh->geometry->get_index(ind * 3)) = cppgl::vec4(1, 0, 0, 1);
                col.at(m_obj->mesh->geometry->get_index(ind * 3 + 1)) = cppgl::vec4(1, 0, 0, 1);
                col.at(m_obj->mesh->geometry->get_index(ind * 3 + 2)) = cppgl::vec4(1, 0, 0, 1);
            }
        }
        m_obj->mesh->update_vertex_buffer(2, col.data());
    }

}

void twin::NetworkImpl::SetColor(const std::vector<cppgl::vec3> &color) {
    /*if (m_mesh_.position.size()) {
        m_mesh_.SetVertexColor(vec4(1, 1, 1, 1));
        for (int i = 0; i != color.size(); ++i)
            if (i < m_mesh_.NumVertices()) m_mesh_.color[i] = make_vec4(color[i], 0.f);
    }
    m_obj_.asset = std::make_shared<ColoredAsset>(m_mesh_);*/
}

void twin::NetworkImpl::Render(cppgl::mat4 model) {
    RenderOptions &ro = m_render_option;
    if (ro.choice & RenderOptions::RenderJoints) {
        m_obj_joints->bind();
        m_obj_joints->draw();
        m_obj_joints->unbind();
    }
    if (ro.choice & RenderOptions::RenderChoice::RenderSkeleton) {
        m_obj_skeleton->bind();
        m_obj_skeleton->draw();
        m_obj_skeleton->unbind();
    }
    if (ro.choice & RenderOptions::RenderChoice::RenderTransparent) {

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        if (model != cppgl::mat4::Identity()) {
            cppgl::mat4 m = m_obj->model;
            m_obj->set_model_transform(model * m);
            m_obj->bind();
            m_obj->draw();
            m_obj->unbind();
            m_obj->set_model_transform(m);
        } else {
            m_obj->bind();
            m_obj->draw();
            m_obj->unbind();
        }

        glDisable(GL_BLEND);
    } else if (ro.choice & RenderOptions::RenderNormal) {
        if (model != cppgl::mat4::Identity()) {
            cppgl::mat4 m = m_obj->model;
            m_obj->set_model_transform(model * m);
            m_obj->bind();
            m_obj->draw();
            m_obj->unbind();
            m_obj->set_model_transform(m);
        } else {
            m_obj->bind();
            m_obj->draw();
            m_obj->unbind();
        }
    }

}

void twin::NetworkImpl::SetConstraints(twin::Constraints constraints) {
    m_constraints = std::move(constraints);
}

std::vector<twin::Triangle> twin::NetworkImpl::getTriangleSoup() const {
    std::vector<twin::Triangle> tris(std::floor(m_obj->mesh->num_indices / 3));

#pragma omp parallel for default(none) shared(m_obj, tris)
    for (unsigned long int i = 0; i < m_obj->mesh->num_indices; i += 3) {
        tris.at(std::floor(i / 3)) = twin::Triangle(
                m_obj->mesh->geometry->get_position(m_obj->mesh->geometry->get_index(i)),
                m_obj->mesh->geometry->get_position(m_obj->mesh->geometry->get_index(i + 1)),
                m_obj->mesh->geometry->get_position(m_obj->mesh->geometry->get_index(i + 2)));
    }
    return tris;
}

bool twin::NetworkImpl::VposerPossible() const {
    return m_sex == Gender::Neutral || m_sex == Gender::Female || m_sex == Gender::Male || m_sex == Gender::RightHand;
}

void twin::NetworkImpl::CreateRenderObjects(std::vector<cppgl::vec3> &&vertices, std::vector<int64_t> &&indices, const std::vector<cppgl::vec3> &Js, const std::vector<cppgl::vec3> &OJs, std::vector<cppgl::vec3> &&vertices_shaped, std::vector<cppgl::vec3> &&normals, std::vector<cppgl::vec3> &&normals_shaped) {
    CreateMesh(m_mesh, m_mesh_shaped, std::forward<std::vector<cppgl::vec3> &&>(vertices), std::forward<std::vector<int64_t> &&>(indices), Js, OJs, std::forward<std::vector<cppgl::vec3> &&>(vertices_shaped), std::forward<std::vector<cppgl::vec3> &&>(normals), std::forward<std::vector<cppgl::vec3> &&>(normals_shaped));

    if (m_render_shaped) {
        m_obj = m_mesh_shaped;
    } else {
        m_obj = m_mesh;
    }


    if (UseUVs()) {
        CreateMeshTextured(m_obj);//sets the uv coordinates in place
    }


    if (!m_obj_joints) {
        if (!cppgl::Material::valid("joint")) {
            cppgl::Material jointMat("joint");
            jointMat->vec4_map["color"] = cppgl::vec4(0, 1, 0, 1);
        }
        if (!cppgl::Shader::valid("pointCloudUni")) {
            cppgl::Shader s3("pointCloudUni", "shader/pointCloudUni.vert", "shader/pointCloud.frag");
        }

        cppgl::Geometry geo("inference_pc_geo_" + name, Js);
        cppgl::Mesh meshs("inference_pc_geo_" + name, geo, cppgl::Material::find("joint"), GL_DYNAMIC_DRAW);
        meshs->primitive_type = GL_POINTS;
        geo->register_mesh(meshs);
        m_obj_joints = cppgl::Drawelement("inference_mesh_joints_" + name, cppgl::Shader::find("pointCloudUni"), meshs);
        m_obj_joints->add_pre_draw_func("pointsize", []() {
            glPointSize(5);
        });
    } else {
        if (m_render_option.choice & RenderOptions::RenderChoice::RenderJoints) {
            m_obj_joints->mesh->update_vertex_buffer(0, Js.data());
        }
    }
    if (!m_obj_skeleton) {
        std::vector<uint> ind;
        for (uint i = 1; i < Parents().size(); ++i) {//Start at one to skip invalid "bone"
            ind.push_back(i);
            ind.push_back(Parents().at(i));
        }
        if (!cppgl::Material::valid("line")) {
            cppgl::Material lineMat("line");
            lineMat->vec4_map["color"] = cppgl::vec4(0, 0, 1, 1);
        }
        if (!cppgl::Shader::valid("pointCloudUni")) {
            cppgl::Shader s3("pointCloudUni", "shader/pointCloudUni.vert", "shader/pointCloud.frag");
        }
        cppgl::Geometry geo("network_skel_geo_" + name, Js, ind);
        cppgl::Mesh m("netword_skel_mesh_" + name, geo, cppgl::Material::find("line"), GL_DYNAMIC_DRAW);
        geo->register_mesh(m);
        m->primitive_type = GL_LINES;
        m_obj_skeleton = cppgl::Drawelement("inference_mesh_skeleton_" + name, cppgl::Shader::find("pointCloudUni"), m);
        m_obj_skeleton->add_pre_draw_func("linesize", []() { glLineWidth(5); });
    } else {
        if (m_render_option.choice & RenderOptions::RenderChoice::RenderSkeleton) {
            m_obj_skeleton->mesh->update_vertex_buffer(0, Js.data());
        }
    }
}

void twin::NetworkImpl::CreateMesh(cppgl::Drawelement &mesh, cppgl::Drawelement &mesh_shaped, std::vector<cppgl::vec3> &&vertices, std::vector<int64_t> &&indices, const std::vector<cppgl::vec3> &Js, const std::vector<cppgl::vec3> &OJs, std::vector<cppgl::vec3> &&vertices_shaped, std::vector<cppgl::vec3> &&normals_verts, std::vector<cppgl::vec3> &&normals_shaped) {
    if (!mesh) {
        std::vector<uint32_t> ind, shap;
        ind.reserve(indices.size());
        shap.reserve(indices.size());
        for (unsigned long int i = 0; i < indices.size(); ++i) {
            shap.push_back(indices.at(i));
            ind.push_back(indices.at(i));
        }
        std::vector<cppgl::vec4> col;
        col.reserve(vertices.size());
        for (long unsigned int i = 0; i < vertices.size(); ++i) {
            col.emplace_back(0.8, 0.8, 0.8, 1);
        }
        cppgl::Geometry geo("network_geo_" + name, vertices, ind, normals_verts);
        geo->add_attribute_vec4("color", col.data(), col.size());
        cppgl::Geometry geoshap("network_shaped_geo_" + name, vertices_shaped, shap, normals_shaped);
        geoshap->add_attribute_vec4("color", col.data(), col.size());
        if (!cppgl::Material::valid("network_mat")) {
            cppgl::Material netMat("network_mat");
            netMat->vec3_map["pivot"] = cppgl::vec3::Zero();
            netMat->vec4_map["color"] = cppgl::vec4(0.8, 0.8, 0.8, 1);
            netMat->vec4_map["lightDir"] = cppgl::vec4(-1, -1, -1, 0).normalized();
        }
        if (!cppgl::Shader::valid("vertColor")) {
            cppgl::Shader s2("vertColor", "shader/MVP_norm_col_pivot.vert", "shader/intensityDiffuse.frag");
        }
        mesh = cppgl::Drawelement("network_de_" + name, cppgl::Shader::find("vertColor"), cppgl::Mesh("network_mesh_" + name, geo, cppgl::Material::find("network_mat"), GL_DYNAMIC_DRAW));
        mesh_shaped = cppgl::Drawelement("network_shaped_de_" + name, cppgl::Shader::find("vertColor"), cppgl::Mesh("network_shaped_mesh_" + name, geoshap, cppgl::Material::find("network_mat"), GL_DYNAMIC_DRAW));
        geo->register_mesh(cppgl::Mesh::find("network_mesh_" + name));
        geoshap->register_mesh(cppgl::Mesh::find("network_shaped_mesh_" + name));
    } else {
        if (m_render_shaped) {
            mesh_shaped->mesh->update_vertex_buffer(0, vertices_shaped.data());
#pragma omp parallel for default(none) shared(mesh_shaped, vertices_shaped)
            for (unsigned long int i = 0; i < mesh_shaped->mesh->geometry->positions_size(); ++i) {
                mesh_shaped->mesh->geometry->set_position(i, vertices_shaped.at(i));
            }
            mesh_shaped->mesh->update_vertex_buffer(1, normals_shaped.data());
#pragma omp parallel for default(none) shared(mesh_shaped, normals_shaped)
            for (unsigned long int i = 0; i < mesh_shaped->mesh->geometry->normals_size(); ++i) {
                mesh_shaped->mesh->geometry->set_normal(i, normals_shaped.at(i));
            }
        } else {
            mesh->mesh->update_vertex_buffer(0, vertices.data());
#pragma omp parallel for default(none) shared(mesh, vertices)
            for (unsigned long int i = 0; i < mesh->mesh->geometry->positions_size(); ++i) {
                mesh->mesh->geometry->set_position(i, vertices.at(i));
            }
            mesh->mesh->update_vertex_buffer(1, normals_verts.data());
#pragma omp parallel for default(none) shared(mesh, normals_verts)
            for (unsigned long int i = 0; i < mesh->mesh->geometry->normals_size(); ++i) {
                mesh->mesh->geometry->set_normal(i, normals_verts.at(i));
            }
        }
    }
}

cppgl::Drawelement twin::NetworkImpl::CreateMeshTextured(cppgl::Drawelement &mesh) {
    if (mesh->mesh->geometry->positions_size() == mesh->mesh->geometry->texcoords_size()) {
#pragma omp parallel for default(none) shared(mesh, m_uv_ids, m_uvs)
        for (unsigned long int i = 0; i < mesh->mesh->geometry->positions_size(); ++i) {
            mesh->mesh->geometry->set_texcoord(i, m_uvs.at(m_uv_ids.at(std::floor(static_cast<double >(i) / 3.))[i % 3]));
        }

    }

    return mesh;
}

std::tuple<std::vector<cppgl::vec3>, std::vector<int64_t>, std::vector<cppgl::vec3>, std::vector<cppgl::vec3>, std::vector<cppgl::vec3>, std::vector<cppgl::vec3>, std::vector<cppgl::vec3>> twin::NetworkImpl::InferenceHelper(twin::NetworkParameters &net_params) {
    torch::Tensor trans = net_params->GetTrans().to(torch::kCUDA);
    torch::Tensor beta = net_params->GetBeta().to(torch::kCUDA);

    torch::Tensor full_pose = net_params->GetPose().to(torch::kCUDA);
    if (net_params->m_use_vposer) {
        m_vposer->to(torch::kCUDA);
        torch::Tensor zin = net_params->GetZin().to(torch::kCUDA);
        full_pose = m_vposer->Decode(zin, full_pose, m_sex == Gender::RightHand || m_sex == Gender::LeftHand);
        net_params->PutPose(full_pose);
    }

    auto [verts, J_transformed, v_shaped] = VPosed(full_pose, trans, beta);


    auto normals = [](const torch::Tensor &vertices, const torch::Tensor &faces) {
        using namespace torch::indexing;
        torch::Tensor normals = torch::zeros_like(vertices);
        torch::Tensor vertices_faces = vertices.index({Slice(), faces});
        torch::Tensor faces_normals = torch::cross(vertices_faces.index({Slice(), Slice(), 2}) - vertices_faces.index({Slice(), Slice(), 1}), vertices_faces.index({Slice(), Slice(), 0}) - vertices_faces.index({Slice(), Slice(), 1}), 2);

        normals.index_add_(1, faces.index({Slice(), 0}), faces_normals);
        normals.index_add_(1, faces.index({Slice(), 1}), faces_normals);
        normals.index_add_(1, faces.index({Slice(), 2}), faces_normals);

        // return normals;
        return torch::nn::functional::normalize(normals, torch::nn::functional::NormalizeFuncOptions().eps(1e-6).dim(2));
    };

    torch::Tensor norms = normals(verts, Faces()).to(torch::kCPU);
    torch::Tensor norms_shaped = normals(v_shaped, Faces()).to(torch::kCPU);

    // Execute the model and turn its output into a tensor.
    torch::Tensor v_shaped_h = (v_shaped).to(torch::kCPU);
    torch::Tensor v_posed_shaped = (verts).to(torch::kCPU);
    torch::Tensor joints = J_transformed.to(torch::kCPU).contiguous();
    torch::Tensor faces = Faces().to(torch::kCPU);
    std::vector<cppgl::vec3> vertices((cppgl::vec3 *) v_posed_shaped.data_ptr<float>(), (cppgl::vec3 *) (v_posed_shaped.data_ptr<float>() + v_posed_shaped.numel()));
    std::vector<int64_t> indices(faces.data_ptr<int64_t>(), faces.data_ptr<int64_t>() + faces.numel());
    std::vector<cppgl::vec3> Js((cppgl::vec3 *) joints.data_ptr<float>(), (cppgl::vec3 *) (joints.data_ptr<float>() + joints.numel()));
    std::vector<cppgl::vec3> vertices_shaped((cppgl::vec3 *) v_shaped_h.data_ptr<float>(), (cppgl::vec3 *) (v_shaped_h.data_ptr<float>() + v_shaped_h.numel()));

    std::vector<cppgl::vec3> h_normals((cppgl::vec3 *) norms.data_ptr<float>(), (cppgl::vec3 *) (norms.data_ptr<float>() + norms.numel()));
    std::vector<cppgl::vec3> h_normals_shaped((cppgl::vec3 *) norms_shaped.data_ptr<float>(), (cppgl::vec3 *) (norms_shaped.data_ptr<float>() + norms_shaped.numel()));
    return std::make_tuple(std::move(vertices), std::move(indices), Js, Js, std::move(vertices_shaped), std::move(h_normals), std::move(h_normals_shaped));
}

void twin::NetworkImpl::WriteVals(std::ostream &os, twin::NetworkParameters np) {
    float mass = 0;
    float chest = 0;
    float hip = 0;
    float waist = 0;
    float height = 0;

    auto [vertices, indices, Js, OJs, vertices_shaped, normals, normals_shaped] = InferenceHelper(np);

    for (long unsigned int i = 0; i < indices.size(); i += 3) {
        mass += cppgl::dot(vertices_shaped.at(indices.at(i)), cppgl::cross(vertices_shaped.at(indices.at(i + 1)), vertices_shaped.at(indices.at(i + 2))));
    }
    mass /= 6;


    for (Constraint &c: m_constraints->All()) {
        if (c.isType<DistanceConstraintImpl>()) {
            DistanceConstraint con = std::dynamic_pointer_cast<DistanceConstraintImpl>(c.ptr);
            cppgl::vec3 v1 = vertices_shaped.at(con->m_vertices[0].x());
            cppgl::vec3 v2 = vertices_shaped.at(con->m_vertices[0].y());
            cppgl::vec3 v3 = vertices_shaped.at(con->m_vertices[0].z());
            cppgl::vec3 v4 = vertices_shaped.at(con->m_vertices[1].x());
            cppgl::vec3 v5 = vertices_shaped.at(con->m_vertices[1].y());
            cppgl::vec3 v6 = vertices_shaped.at(con->m_vertices[1].z());

            cppgl::vec3 f1 = v1 * con->m_barycentric[0].x() + v2 * con->m_barycentric[0].y() + v3 * con->m_barycentric[0].z();
            cppgl::vec3 f2 = v4 * con->m_barycentric[1].x() + v5 * con->m_barycentric[1].y() + v6 * con->m_barycentric[1].z();

            float a = f1.y();
            float b = f2.y();
            height = std::abs(a - b);
        }
        if (c.isType<CircumferenceConstraintImpl>()) {
            CircumferenceConstraint con = std::dynamic_pointer_cast<CircumferenceConstraintImpl>(c.ptr);
            float cir = 0;
            std::vector<cppgl::vec3> ps(con->m_alphas.size());
            for (long unsigned int i = 0; i < con->m_alphas.size(); ++i) {
                cppgl::vec3 v1 = vertices_shaped.at(con->m_indices.at(i * 2));
                cppgl::vec3 v2 = vertices_shaped.at(con->m_indices.at(i * 2 + 1));
                ps.at(i) = (1 - con->m_alphas.at(i)) * v1 + con->m_alphas.at(i) * v2;
            }

#pragma omp parallel for reduction(+ : cir)
            for (size_t i = 0; i < ps.size(); ++i) {
                cir += (ps.at(i) - ps.at((i + 1) % ps.size())).norm();
            }
            if (con->name == "Chest") {
                chest = cir;
            } else if (con->name == "Waist") {
                waist = cir;
            } else if (con->name == "Hip") {
                hip = cir;
            }
        }
    }


    nlohmann::json js;
    js["hip"] = hip;
    js["mass"] = mass;
    js["chest"] = chest;
    js["waist"] = waist;
    js["height"] = height;
    os << js;
}

#endif