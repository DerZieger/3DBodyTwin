#ifdef HAS_DL_MODULE

#include "net/constraints/distance.h"

#include <utility>
#include "gui.h"

int twin::DistanceConstraintImpl::counter = 0;

twin::DistanceConstraintImpl::DistanceConstraintImpl() : ConstraintImpl("distance_" + std::to_string(counter++)) {
    m_faces[0] = -1;
    m_faces[1] = -1;
    m_axis[0] = true;
    m_axis[1] = true;
    m_axis[2] = true;
    m_barycentric[0] = cppgl::vec3(1. / 3, 1. / 3, 1. / 3);
    m_barycentric[1] = cppgl::vec3(1. / 3, 1. / 3, 1. / 3);
    m_vertices[0] = cppgl::ivec3(-1, -1, -1);
    m_vertices[1] = cppgl::ivec3(-1, -1, -1);
    m_points.resize(2);
}

twin::DistanceConstraintImpl::DistanceConstraintImpl(const std::string &cname) : ConstraintImpl(cname + "_distance_" + std::to_string(counter)), m_torch_constraint(TorchDistanceConstraint(cname + "_distance_" + std::to_string(counter++) + "_torch")) {
    m_faces[0] = -1;
    m_faces[1] = -1;
    m_axis[0] = true;
    m_axis[1] = true;
    m_axis[2] = true;
    m_barycentric[0] = cppgl::vec3(1. / 3, 1. / 3, 1. / 3);
    m_barycentric[1] = cppgl::vec3(1. / 3, 1. / 3, 1. / 3);
    m_vertices[0] = cppgl::ivec3(-1, -1, -1);
    m_vertices[1] = cppgl::ivec3(-1, -1, -1);
    m_points.resize(2);
};

twin::DistanceConstraintImpl::DistanceConstraintImpl(const std::string &cname, const cppgl::Drawelement &mesh, int face_a, int face_b, float distance, cppgl::ivec3 axis, cppgl::vec3 barycenter_a, cppgl::vec3 barycenter_b) : ConstraintImpl(cname + "_distance_" + std::to_string(counter)), m_torch_constraint(TorchDistanceConstraint(cname + "_distance_" + std::to_string(counter++) + "_torch")) {
    m_target = distance;
    m_faces[0] = face_a;
    m_faces[1] = face_b;

    m_barycentric[0] = std::move(barycenter_a);
    m_barycentric[1] = std::move(barycenter_b);

    m_axis[0] = axis[0];
    m_axis[1] = axis[1];
    m_axis[2] = axis[2];

    m_vertices[0] = cppgl::ivec3(static_cast<int>(mesh->mesh->geometry->get_index(face_a * 3)), static_cast<int>(mesh->mesh->geometry->get_index(face_a * 3 + 1)), static_cast<int>(mesh->mesh->geometry->get_index(face_a * 3 + 2)));
    m_vertices[1] = cppgl::ivec3(static_cast<int>(mesh->mesh->geometry->get_index(face_b * 3)), static_cast<int>(mesh->mesh->geometry->get_index(face_b * 3 + 1)), static_cast<int>(mesh->mesh->geometry->get_index(face_b * 3 + 2)));
    m_points.resize(2);
}

bool twin::DistanceConstraintImpl::drawGUIShort() {
    ImGui::PushID(name.c_str());
    ExchangeData(m_torch_constraint);
    ImGui::Text("Value: %.3f, Loss: %.3f", m_value, m_loss);
    ImGui::PopID();
    return false;
}

void twin::DistanceConstraintImpl::drawGUIEdit(twin::BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const twin::Ray &ray, cppgl::vec2 relative_pixel, bool pressed) {
    ImGui::PushID((std::string("distance") + std::to_string(m_id)).c_str());
    bool has_first = m_vertices[0][0] != -1;
    bool has_second = m_vertices[1][0] != -1;
    if (pressed && (!has_first || !has_second)) {
        bvh.Update();
        Intersection::RayTriangleIntersection intersection = bvh.getClosest(ray);
        if (intersection.valid && !intersection.backFace && intersection.triangleIndex != m_faces[!has_first]) {
            m_faces[has_first] = intersection.triangleIndex;
            m_barycentric[has_first] = intersection.barycenter;
            m_vertices[has_first] = cppgl::ivec3(static_cast<int>(mesh->mesh->geometry->get_index(intersection.triangleIndex * 3)), static_cast<int>(mesh->mesh->geometry->get_index(intersection.triangleIndex * 3 + 1)), static_cast<int>(mesh->mesh->geometry->get_index(intersection.triangleIndex * 3 + 2)));

            SetChange(Change::ParametersReconfigure);
            m_recompute_rendering = true;
        }
    }
    if (!m_torch_constraint) {
        m_torch_constraint = TorchDistanceConstraint(name + "_distance_" + std::to_string(m_id) + "_torch");
    }
    ExchangeData(m_torch_constraint);
    ImGui::InputText(_labelPrefix("Name:").c_str(), &name);
    if (ImGui::InputFloat(_labelPrefix("Distance in m:").c_str(), &m_target)) {
        m_torch_constraint->m_target.store(m_target, std::memory_order_relaxed);
        SetChange(ConstraintImpl::Change::ParametersReconfigure);
    }
    if (ImGui::InputFloat(_labelPrefix("Weight").c_str(), &m_weight)) {
        m_torch_constraint->m_weight.store(m_weight, std::memory_order_relaxed);
        SetChange(ConstraintImpl::Change::ParametersReconfigure);
    }
    ImGui::PushID("P1");
    ImGui::Text("Point 1:");
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        m_vertices[0] = cppgl::ivec3{-1, -1, -1};
        m_barycentric[0] = cppgl::vec3{0, 0, 0};
        m_faces[0] = -1;
        m_recompute_rendering = true;
    }
    ImGui::Indent(4);
    ImGui::Text("Face %i (%i, %i, %i) (%f, %f, %f)", m_faces[0], m_vertices[0][0], m_vertices[0][1], m_vertices[0][2], m_barycentric[0][0], m_barycentric[0][1], m_barycentric[0][2]);
    ImGui::Unindent(4);
    ImGui::PopID();
    ImGui::PushID("P2");
    ImGui::Text("Point 2:");
    ImGui::SameLine();
    if (ImGui::Button("Reset")) {
        m_vertices[1] = cppgl::ivec3{-1, -1, -1};
        m_barycentric[1] = cppgl::vec3{0, 0, 0};
        m_faces[1] = -1;
        m_recompute_rendering = true;
    }

    ImGui::Indent(4);
    ImGui::Text("Face %i (%i, %i, %i) (%f, %f, %f)", m_faces[1], m_vertices[1][0], m_vertices[1][1], m_vertices[1][2], m_barycentric[0][0], m_barycentric[0][1], m_barycentric[0][2]);
    ImGui::Unindent(4);
    ImGui::PopID();
    ImGui::PushID("X");
    ImGui::Text("X");
    ImGui::SameLine();
    ImGui::Checkbox("##", &m_axis[0]);
    ImGui::SameLine();
    ImGui::PopID();
    ImGui::PushID("Y");
    ImGui::Text("Y");
    ImGui::SameLine();
    ImGui::Checkbox("##", &m_axis[1]);
    ImGui::SameLine();
    ImGui::PopID();
    ImGui::PushID("Z");
    ImGui::Text("Z");
    ImGui::SameLine();
    ImGui::Checkbox("##", &m_axis[2]);
    ImGui::PopID();
    ImGui::Text("Current Value: %f (%f), Loss: %f ", m_value, m_target, m_loss);

    ImGui::PopID();
}

void twin::DistanceConstraintImpl::Finalize() {
    if (!m_torch_constraint) {
        m_torch_constraint = TorchDistanceConstraint(name + "_distance_" + std::to_string(m_id) + "_torch");
    }
}

void twin::DistanceConstraintImpl::createDrawelement(const cppgl::Drawelement &dre) {
    if (m_vertices[0][0] == -1 && m_faces[0] != -1) {
        m_points.at(0) = dre->mesh->geometry->get_position(dre->mesh->geometry->get_index(m_faces[0] * 3)) * m_barycentric[0][0] + dre->mesh->geometry->get_position(dre->mesh->geometry->get_index(m_faces[0] * 3 + 1)) * m_barycentric[0][1] + dre->mesh->geometry->get_position(dre->mesh->geometry->get_index(m_faces[0] * 3 + 2)) * m_barycentric[0][2];
    } else if (m_vertices[0][0] != -1) {
        m_points.at(0) = dre->mesh->geometry->get_position(m_vertices[0][0]) * m_barycentric[0][0] + dre->mesh->geometry->get_position(m_vertices[0][1]) * m_barycentric[0][1] + dre->mesh->geometry->get_position(m_vertices[0][2]) * m_barycentric[0][2];
    }
    if (m_vertices[1][0] == -1 && m_faces[1] != -1) {
        m_points.at(1) = dre->mesh->geometry->get_position(dre->mesh->geometry->get_index(m_faces[1] * 3)) * m_barycentric[1][0] + dre->mesh->geometry->get_position(dre->mesh->geometry->get_index(m_faces[1] * 3 + 1)) * m_barycentric[1][1] + dre->mesh->geometry->get_position(dre->mesh->geometry->get_index(m_faces[1] * 3 + 2)) * m_barycentric[1][2];
    } else if (m_vertices[1][0] != -1) {
        m_points.at(1) = dre->mesh->geometry->get_position(m_vertices[1][0]) * m_barycentric[1][0] + dre->mesh->geometry->get_position(m_vertices[1][1]) * m_barycentric[1][1] + dre->mesh->geometry->get_position(m_vertices[1][2]) * m_barycentric[1][2];
    }


    if (m_vertices[0][0] == -1 && m_faces[0] != -1) {
        m_vertices[0][0] = static_cast<int>(dre->mesh->geometry->get_index(m_faces[0] * 3));
        m_vertices[0][1] = static_cast<int>(dre->mesh->geometry->get_index(m_faces[0] * 3 + 1));
        m_vertices[0][2] = static_cast<int>(dre->mesh->geometry->get_index(m_faces[0] * 3 + 2));
    }
    if (m_vertices[1][0] == -1 && m_faces[1] != -1) {
        m_vertices[1][0] = static_cast<int>(dre->mesh->geometry->get_index(m_faces[1] * 3));
        m_vertices[1][1] = static_cast<int>(dre->mesh->geometry->get_index(m_faces[1] * 3 + 1));
        m_vertices[1][2] = static_cast<int>(dre->mesh->geometry->get_index(m_faces[1] * 3 + 2));
    }


    if (!m_consRepr) {
        if (!cppgl::Material::valid("marker")) {
            cppgl::Material markerMat("marker");
            markerMat->vec4_map["color"] = cppgl::vec4(1, 0, 0, 1);
        }
        if (!cppgl::Shader::valid("pointCloudUni")) {
            cppgl::Shader s3("pointCloudUni", "shader/pointCloudUni.vert", "shader/pointCloud.frag");
        }

        std::vector<uint32_t> ind = {0, 1};

        cppgl::Geometry geo(name + "_pc_geo_" + std::to_string(m_id), m_points, ind);
        cppgl::Mesh meshs(name + "_pc_mesh_" + std::to_string(m_id), geo, cppgl::Material::find("marker"), GL_DYNAMIC_DRAW);
        meshs->primitive_type = GL_POINTS;

        m_consRepr = cppgl::Drawelement(name + "_repr_ " + std::to_string(m_id), cppgl::Shader::find("pointCloudUni"), meshs);
        m_consRepr->add_pre_draw_func("pointsize", []() {
            glPointSize(5);
        });
    } else {
        m_consRepr->mesh->update_vertex_buffer(0, m_points.data());
    }
}

void twin::DistanceConstraintImpl::Serialize(nlohmann::json &json) const {
    nlohmann::json &json2 = json[name];
    json2["type"] = "Distance";
    json2["faces"] = std::vector<int>(m_faces, m_faces + 2);
    json2["axis"] = std::vector<bool>(m_axis, m_axis + 3);

    std::vector<float> b;
    for (const cppgl::vec3 &i: m_barycentric) {
        for (int j = 0; j < 3; ++j) {
            b.push_back(i[j]);
        }
    }
    json2["baryCoords"] = b;

    std::vector<int> v;
    for (const cppgl::ivec3 &vertex: m_vertices) {
        for (int j = 0; j < 3; ++j) {
            v.push_back(vertex[j]);
        }
    }
    json2["vertIds"] = v;
    json2["target"] = m_target;
    json2["weight"] = m_weight;
}

void twin::DistanceConstraintImpl::Deserialize(const nlohmann::json &json, std::string cname) {
    name = cname;
    const nlohmann::json &con = *json.find(name);

    m_target = con.find("target")->begin().value().get<float>();
    m_weight = con.find("weight")->begin().value().get<float>();
    const nlohmann::json &f = *con.find("faces");
    const nlohmann::json &a = *con.find("axis");
    const nlohmann::json &b = *con.find("baryCoords");
    const nlohmann::json &v = *con.find("vertIds");
    int i = 0;
    for (auto iter = f.begin(); iter != f.end(); ++iter, ++i) {
        m_faces[i] = iter.value().get<int>();
    }

    i = 0;
    for (auto iter = a.begin(); iter != a.end(); ++iter, ++i) {
        m_axis[i] = iter.value().get<bool>();
    }

    i = 0;
    for (auto iter = b.begin(); iter != b.end(); ++iter, ++i) {
        m_barycentric[i / 3][i % 3] = iter.value().get<float>();
    }
    i = 0;
    for (auto iter = v.begin(); iter != v.end(); ++iter, ++i) {
        m_vertices[i / 3][i % 3] = iter.value().get<int>();
    }
    if (!m_torch_constraint) {
        m_torch_constraint = TorchDistanceConstraint(name + "_distance_" + std::to_string(m_id) + "_torch");
    }
    ExchangeData(m_torch_constraint);
}

bool twin::DistanceConstraintImpl::UsesTarget() {
    if (ImGui::SliderFloat("", &m_target, 0.0, 2.0))
        SetChange(ConstraintImpl::Change::ParametersReconfigure);
    return true;
}

twin::TorchConstraint twin::DistanceConstraintImpl::PrepareOptimImpl(twin::ConstraintArgument &ca) {
    std::vector<int> idx{m_vertices[0][0], m_vertices[0][1], m_vertices[0][2], m_vertices[1][0], m_vertices[1][1], m_vertices[1][2]};
    std::vector<float> bary{m_barycentric[0][0], m_barycentric[0][1], m_barycentric[0][2], m_barycentric[1][0], m_barycentric[1][1], m_barycentric[1][2]};


    std::lock_guard guard(m_torch_constraint->m_lock);

    m_torch_constraint->m_bary = torch::from_blob(bary.data(), {2, 3}).to(ca.device);
    m_torch_constraint->m_index = torch::from_blob(idx.data(), {2, 3}, torch::kInt32).to(torch::kInt64).to(ca.device);
    m_torch_constraint->m_axis = torch::from_blob(m_axis, {3}, torch::kBool).to(torch::kFloat32).to(ca.device);
    return m_torch_constraint;
}

twin::TorchConstraint twin::DistanceConstraintImpl::GetConstraint() {
    return m_torch_constraint;
}

twin::TorchDistanceConstraintImpl::TorchDistanceConstraintImpl(std::string name) : TorchConstraintImpl(std::move(name)) {}

torch::Tensor twin::TorchDistanceConstraintImpl::compute_impl(twin::ConstraintArgument &ca) {
    using namespace torch::indexing;

    torch::Tensor vs = ca.v_shaped.index({Slice(), m_index});
    torch::Tensor points = torch::einsum("nk,bnkh,h->bnh", {m_bary, vs, m_axis});
    torch::Tensor diff = points.index({Slice(), 1}) - points.index({Slice(), 0});
    torch::Tensor res = torch::linalg_norm(diff);
    torch::Tensor loss = m_weight.load(std::memory_order_relaxed) * torch::abs(res - m_target.load(std::memory_order_relaxed));

    m_loss = loss.item().toFloat();
    m_value = res.item().toFloat();
    return loss;
}


#endif