#ifdef HAS_DL_MODULE

#include "util/intersection.h"
#include "net/constraints/convex_circumference.h"

#include <utility>
#include "gui.h"

TWIN_NAMESPACE_BEGIN
    int CircumferenceConstraintImpl::counter = 0;

    CircumferenceConstraintImpl::CircumferenceConstraintImpl() : ConstraintImpl(
            "circumference_" + std::to_string(counter++)), m_normal(cppgl::vec3(0, 1, 0)), m_dot(0.f), m_pressed_counter(0) {}

    CircumferenceConstraintImpl::CircumferenceConstraintImpl(const std::string &cname) : ConstraintImpl(cname + "_circumference_" + std::to_string(counter)), m_normal(cppgl::vec3(0, 1, 0)), m_dot(0.f), m_pressed_counter(0), m_torch_constraint(TorchCircumferenceConstraint(cname + "_circumference_" + std::to_string(counter++) + "_torch")) {}

    CircumferenceConstraintImpl::CircumferenceConstraintImpl(const std::string &cname, float dot, cppgl::vec3 normal, float circumference) : ConstraintImpl(cname + "_circumference_" + std::to_string(counter)), m_normal(std::move(normal)), m_dot(dot), m_pressed_counter(0), m_torch_constraint(TorchCircumferenceConstraint(cname + "_circumference_" + std::to_string(counter++) + "_torch")) {
        m_target = circumference;
    }

    bool CircumferenceConstraintImpl::drawGUIShort() {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchCircumferenceConstraint(name + "_circumference_" + std::to_string(m_id) + "_torch");
        }
        ExchangeData(m_torch_constraint);
        ImGui::Text("V: %.3f, L: %.3f, T %.3f CT %.3f", m_value, m_loss, m_target, m_torch_constraint->m_target.load(std::memory_order_relaxed));
        return false;
    }

    std::pair<cppgl::vec3, float> ChooseLeastVarianceNormal(const cppgl::Drawelement &mesh, const std::vector<int> &faces) {
        cppgl::vec3 center = cppgl::vec3::Zero();
        for (int i: faces) {
            for (int j = 0; j != 3; ++j) {
                center += mesh->mesh->geometry->get_position(mesh->mesh->geometry->get_index(i * 3 + j));
            }
        }
        center /= static_cast<float>(faces.size()) * 3;
        cppgl::mat3 cov = cppgl::mat3::Zero();
        for (int i: faces) {
            for (int j = 0; j != 3; ++j) {
                cov += (mesh->mesh->geometry->get_position(mesh->mesh->geometry->get_index(i * 3 + j)) - center) * (mesh->mesh->geometry->get_position(mesh->mesh->geometry->get_index(i * 3 + j)) - center).transpose();
            }
        }

        Eigen::JacobiSVD svd = cov.jacobiSvd(Eigen::ComputeFullU | Eigen::ComputeFullV);

        float min_variance = std::numeric_limits<float>::max();
        cppgl::vec3 min_normal;
        for (int k = 0; k < 3; ++k) {
            float mean_dist = 0;
            cppgl::vec3 normal = svd.matrixU().col(k).normalized().eval();
            for (int i: faces) {
                for (int j = 0; j < 3; ++j) {
                    cppgl::vec3 pos = mesh->mesh->geometry->get_position(mesh->mesh->geometry->get_index(i * 3 + j));
                    cppgl::vec3 centered = pos - center;
                    double sq = centered.dot(centered) - normal.dot(centered) * normal.dot(centered);
                    mean_dist += static_cast<float>( sqrt(sq));
                }
            }
            mean_dist /= static_cast<float>(faces.size()) * 3.f;
            float mean_var = 0;
            for (int i: faces) {
                for (int j = 0; j < 3; ++j) {
                    cppgl::vec3 pos = mesh->mesh->geometry->get_position(mesh->mesh->geometry->get_index(i * 3 + j));
                    cppgl::vec3 centered = pos - center;
                    float sq = centered.dot(centered) - normal.dot(centered) * normal.dot(centered);
                    double dist = sqrt(static_cast<double>(sq));
                    mean_var += static_cast<float>((dist - mean_dist) * (dist - mean_dist));
                }
            }
            mean_var /= static_cast<float>(faces.size()) * 3.f;
            if (min_variance > mean_var) {
                min_variance = mean_var;
                min_normal = normal;
            }
        }
        return std::make_pair(min_normal, min_normal.dot(center));
    }

    void CircumferenceConstraintImpl::drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const Ray &ray, cppgl::vec2 relative_pixel, bool pressed) {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchCircumferenceConstraint(name + "_circumference_" + std::to_string(m_id) + "_torch");
        }
        ExchangeData(m_torch_constraint);
        bool update = false;
        ImGui::InputText(_labelPrefix("Name").c_str(), &name);
        if (ImGui::Button("Update By Selection")) {
            update = true;
            std::pair<cppgl::vec3, float> res = ChooseLeastVarianceNormal(mesh_shaped, m_faces);
            m_normal = res.first;
            m_dot = res.second;
            SetChange(Change::ParametersReconfigure);
        }
        update |= ImGui::InputFloat(_labelPrefix("Distance from origin:").c_str(), &m_dot);
        update |= ImGui::InputFloat3(_labelPrefix("Normal:").c_str(), m_normal.data());
        if (ImGui::InputFloat(_labelPrefix("Circumference[m]:").c_str(), &m_target)) {
            SetChange(Change::ParametersReconfigure);
        }
        if (ImGui::InputFloat(_labelPrefix("Weight:").c_str(), &m_weight)) {
            SetChange(Change::ParametersReconfigure);
        }


        if (ImGui::Button("Reset") || m_pressed_counter == 3) {
            m_selection.ResetSelection();
            m_faces.clear();
        }

        if (pressed) {
            ++m_pressed_counter;
        } else {
            m_pressed_counter = 0;
        }
        if (m_pressed_counter >= 3) {
            m_faces = m_selection.Select(mesh_shaped, ray, relative_pixel);
            update = true;
            m_selection.Render();
        }

        m_recompute_rendering |= update;
        if (m_recompute_rendering) {
            SetChange(Change::ParametersReconfigure);
        }
        ImGui::Text("Current Value: %f (%f), Loss: %f ", m_value, m_target, m_loss);
        ExchangeData(m_torch_constraint);
    }

    void CircumferenceConstraintImpl::Finalize() {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchCircumferenceConstraint(name + "_circumference_" + std::to_string(m_id) + "_torch");
        }
    }

    void CircumferenceConstraintImpl::createDrawelement(const cppgl::Drawelement &dre) {

        if (m_recompute_rendering || m_alphas.empty()) {
            std::vector<Triangle> triangles;
            auto add_triangle = [&](int id) {
                CircumferenceConstraintImpl::Triangle t;
                t.id[0] = static_cast<int>(dre->mesh->geometry->get_index(id));
                t.id[1] = static_cast<int>(dre->mesh->geometry->get_index(id + 1));
                t.id[2] = static_cast<int>(dre->mesh->geometry->get_index(id + 2));
                t.v[0] = dre->mesh->geometry->get_position(t.id[0]);
                t.v[1] = dre->mesh->geometry->get_position(t.id[1]);
                t.v[2] = dre->mesh->geometry->get_position(t.id[2]);
                auto sort = [&](int i, int j) {
                    if (t.id[i] > t.id[j]) {
                        std::swap(t.v[i], t.v[j]);
                        std::swap(t.id[i], t.id[j]);
                    }
                };
                sort(0, 1);
                sort(0, 2);
                sort(1, 2);
                triangles.push_back(t);
            };
            if (!m_faces.empty()) {
                for (int i: m_faces) {
                    add_triangle(i * 3);
                }
            } else {
                for (int i = 0; i < static_cast<int>(dre->mesh->num_indices / 3); ++i) {
                    add_triangle(i * 3);
                }
            }

            TrianglesToIndices(triangles);
        }

        if (!m_alphas.empty()) {
            auto compute_point = [&indices = m_indices, &alphas = m_alphas](size_t i, const cppgl::Drawelement &m) {
                int pa = indices.at(i * 2 + 0);
                int pb = indices.at(i * 2 + 1);
                float alpha = alphas.at(i);
                cppgl::vec3 point = m->mesh->geometry->get_position(pa) * (1 - alpha) + (alpha) * m->mesh->geometry->get_position(pb);
                return point;
            };
            if (!m_consRepr) {
                if (!cppgl::Material::valid("line")) {
                    cppgl::Material lineMat("line");
                    lineMat->vec4_map["color"] = cppgl::vec4(0, 0, 1, 1);
                }
                if (!cppgl::Shader::valid("pointCloudUni")) {
                    cppgl::Shader s3("pointCloudUni", "shader/pointCloudUni.vert", "shader/pointCloud.frag");
                }
                std::vector<uint> ind;
                std::vector<cppgl::vec3> vert;
                ind.reserve(m_alphas.size() * 2);
                vert.reserve(m_alphas.size());
                for (size_t i = 0; i < m_alphas.size(); i++) {
                    vert.push_back(compute_point(i, dre));
                    ind.push_back(i);
                    ind.push_back((i + 1) % m_alphas.size());
                }
                cppgl::Geometry geo(name + "_geo", vert, ind);
                cppgl::Mesh m(name + "_mesh", geo, cppgl::Material::find("line"), GL_DYNAMIC_DRAW);
                geo->register_mesh(m);
                m->primitive_type = GL_LINES;
                m_consRepr = cppgl::Drawelement(name + "_dre", cppgl::Shader::find("pointCloudUni"), m);
                m_consRepr->add_pre_draw_func("linewidth", []() {
                    glDisable(GL_DEPTH_TEST);
                    glLineWidth(2.f);
                });
                m_consRepr->add_post_draw_func("linewidth", []() {
                    glEnable(GL_DEPTH_TEST);
                });
            } else {
                std::vector<uint> ind;
                std::vector<float> vert;
                ind.reserve(m_alphas.size() * 2);
                vert.reserve(m_alphas.size() * 3);
                for (size_t i = 0; i < m_alphas.size(); i++) {

                    cppgl::vec3 tmp = compute_point(i, dre);
                    vert.push_back(tmp[0]);
                    vert.push_back(tmp[1]);
                    vert.push_back(tmp[2]);
                    ind.push_back(i);
                    ind.push_back((i + 1) % m_alphas.size());
                }
                m_consRepr->mesh->geometry->set(vert, ind);
                m_consRepr->mesh->geometry->update_meshes();
            }
        }
        m_recompute_rendering = false;
    }

    void CircumferenceConstraintImpl::Serialize(nlohmann::json &json) const {
        nlohmann::json &json2 = json[name];
        json2["type"] = "Circumference";
        json2["target"] = m_target;
        json2["dot"] = m_dot;
        json2["faces"] = m_faces;
        json2["weight"] = m_weight;
        json2["normal"] = std::vector<float>(m_normal.begin(), m_normal.end());
    }

    void CircumferenceConstraintImpl::Deserialize(const nlohmann::json &json, std::string cname) {
        name = cname;
        const nlohmann::json &con = json[name];
        m_weight = con["weight"];
        m_target = con["target"];
        m_dot = con["dot"];
        m_faces = con["faces"].get<std::vector<int>>();
        int i = 0;
        for (auto iter = con["normal"].begin(); iter != con["normal"].end(); ++iter, ++i) {
            m_normal[i] = iter.value().get<float>();
        }
        if (!m_torch_constraint) {
            m_torch_constraint = TorchCircumferenceConstraint(name + "_circumference_" + std::to_string(m_id) + "_torch");
        }
        ExchangeData(m_torch_constraint);
    }

    bool CircumferenceConstraintImpl::UsesTarget() {
        if (ImGui::SliderFloat("", &m_target, 0.0, 2.0))
            SetChange(ConstraintImpl::Change::ParametersReconfigure);
        return true;
    }

    std::vector<CircumferenceConstraintImpl::IntersectionPoint> CircumferenceConstraintImpl::GiftWrapping(std::vector<CircumferenceConstraintImpl::IntersectionPoint> &points, const cppgl::vec2 &center) {
        if (points.empty()) {
            return std::vector<CircumferenceConstraintImpl::IntersectionPoint>();
        }
        // Move the leftmost point to the beginning of our vector.
        // It will be the first point in our convext hull.
        CircumferenceConstraintImpl::IntersectionPoint &p2 = *std::min_element(points.begin(), points.end(), [](const CircumferenceConstraintImpl::IntersectionPoint &a, const CircumferenceConstraintImpl::IntersectionPoint &b) {
            return (a.rotated[0] < b.rotated[0] || (a.rotated[0] == b.rotated[0] && a.rotated[1] < b.rotated[1]));
        });
        std::swap(points[0], p2);

        std::vector<CircumferenceConstraintImpl::IntersectionPoint> hull;
        // Repeatedly find the first ccw point from our last hull point
        // and put it at the front of our array.
        // Stop when we see our first point again.
        //
        struct ccwSorter {
            const CircumferenceConstraintImpl::IntersectionPoint &pivot;

            ccwSorter(const CircumferenceConstraintImpl::IntersectionPoint &inpivot) : pivot(inpivot) {}

            bool operator()(const CircumferenceConstraintImpl::IntersectionPoint &a, const CircumferenceConstraintImpl::IntersectionPoint &b) {
                return (a.rotated[0] - pivot.rotated[0]) * (b.rotated[1] - pivot.rotated[1]) - (a.rotated[1] - pivot.rotated[1]) * (b.rotated[0] - pivot.rotated[0]) < 0;
            }
        };
        do {
            hull.push_back(points[0]);
            std::swap(points[0], *std::min_element(points.begin() + 1, points.end(), ccwSorter(points[0])));
        } while (points[0].rotated[0] != hull[0].rotated[0] || points[0].rotated[1] != hull[0].rotated[1]);

        return hull;
    }

    TorchConstraint CircumferenceConstraintImpl::PrepareOptimImpl(ConstraintArgument &ca) {
        using namespace torch::indexing;
        torch::Tensor t_index = torch::from_blob(m_faces.data(), {static_cast<long>(m_faces.size())}, torch::kInt32).to(torch::kInt64).to(torch::kCPU);
        torch::Tensor selected_faces = m_faces.empty() ? ca.faces.to(torch::kCPU) : ca.faces.index({t_index}).to(torch::kCPU);
        torch::Tensor selected_m_facesvertices = ca.v_shaped.index({Slice(), selected_faces}).to(torch::kCPU);
        std::vector<int64_t> cpu_faces(selected_faces.data_ptr<int64_t>(), selected_faces.data_ptr<int64_t>() + selected_faces.numel());
        std::vector<float> cpu_face_verts(selected_m_facesvertices.data_ptr<float>(), selected_m_facesvertices.data_ptr<float>() + selected_m_facesvertices.numel());
        assert(cpu_face_verts.size() % 9 == 0);
        std::vector<Triangle> triangles;
        for (int i = 0; i != static_cast<int>(cpu_faces.size() / 3); ++i) {
            Triangle t;
            t.id[0] = static_cast<int>(cpu_faces[3 * i]);
            t.id[1] = static_cast<int>(cpu_faces[3 * i + 1]);
            t.id[2] = static_cast<int>(cpu_faces[3 * i + 2]);
            t.v[0] = (m_model * cppgl::make_vec4(cppgl::vec3(cpu_face_verts.data() + 9 * i + 0), 1)).head<3>();
            t.v[1] = (m_model * cppgl::make_vec4(cppgl::vec3(cpu_face_verts.data() + 9 * i + 3), 1)).head<3>();
            t.v[2] = (m_model * cppgl::make_vec4(cppgl::vec3(cpu_face_verts.data() + 9 * i + 6), 1)).head<3>();
            auto sort = [&](int i, int j) {
                if (t.id[i] > t.id[j]) {
                    std::swap(t.v[i], t.v[j]);
                    std::swap(t.id[i], t.id[j]);
                }
            };
            sort(0, 1);
            sort(0, 2);
            sort(1, 2);
            triangles.push_back(t);
        }

        TrianglesToIndices(triangles);

        std::lock_guard guard(m_torch_constraint->m_lock);
        m_torch_constraint->m_t_index = t_index;
        m_torch_constraint->m_segment_index = torch::from_blob(m_indices.data(), {static_cast<long>(m_indices.size() / 2), 2}, torch::kInt32).to(torch::kInt64).to(torch::kCUDA);
        m_torch_constraint->m_segment_alpha = torch::from_blob(m_alphas.data(), {static_cast<long>(m_alphas.size())}).to(torch::kFloat32).to(torch::kCUDA);
        return m_torch_constraint;
    }

    TorchConstraint CircumferenceConstraintImpl::GetConstraint() {
        return m_torch_constraint;
    }

    std::pair<std::vector<CircumferenceConstraintImpl::IntersectionPoint>, cppgl::vec2> CircumferenceConstraintImpl::TrianglePlaneIntersection(const std::vector<CircumferenceConstraintImpl::Triangle> &triangles, const Plane &plane) const {
        static constexpr int edge_order[3][2] = {{0, 1},
                                                 {0, 2},
                                                 {1, 2}};

        std::set<std::pair<int, int>> used;
        std::vector<IntersectionPoint> points;
        cppgl::vec2 center(0, 0);
        if (triangles.empty()) {
            return std::make_pair<>(points, center);
        }

        cppgl::mat3 rotation = Eigen::Quaternionf::FromTwoVectors(m_normal, cppgl::vec3(0, 1, 0)).toRotationMatrix();

        int left_most_point_id = -1;
        float left_most_point = std::numeric_limits<float>::max();
        // triangle plane intersection
        for (long unsigned int i = 0; i < triangles.size(); ++i) {
            CircumferenceConstraintImpl::Triangle tri = triangles[i];
            for (int edge = 0; edge != 3; ++edge) {
                std::pair<int, int> index = std::make_pair(tri.id[edge_order[edge][0]], tri.id[edge_order[edge][1]]);
                if (used.count(index)) {
                    continue;
                } else {
                    used.insert(index);
                    cppgl::vec3 v0 = tri.v[edge_order[edge][0]];
                    cppgl::vec3 v1 = tri.v[edge_order[edge][1]];
                    float len = (v1 - v0).norm();
                    cppgl::vec3 dir = (v1 - v0) / len;
                    Ray ray = Ray(dir, v0);
                    float t = 0;
                    bool hit = Intersection::RayPlane(ray, plane, t);
                    if (hit) {
                        t /= len;
                        if (t > 0 && t <= 1) {
                            IntersectionPoint s = {.v0 = tri.id[edge_order[edge][0]], .v1 = tri.id[edge_order[edge][1]], .alpha = t, .point = v0 + t * (v1 - v0)};
                            cppgl::vec3 rotated = rotation * (s.point - m_dot * m_normal);
                            s.rotated = cppgl::make_vec2(rotated[0], rotated[2]);
                            s.tri = static_cast<int>(i);
                            points.push_back(s);
                            center += s.rotated;
                            if (s.rotated[0] < left_most_point) {
                                left_most_point = s.rotated[0];
                                left_most_point_id = static_cast<int>(points.size()) - 1;
                            }
                        }
                    }
                }
            }
        }
        if (points.empty()) {
            return std::make_pair(points, center);
        }
        std::swap(points[0], points[left_most_point_id]);
        center /= static_cast<float>(points.size());
        return std::make_pair(points, center);
    }

    void CircumferenceConstraintImpl::TrianglesToIndices(
            const std::vector<CircumferenceConstraintImpl::Triangle> &triangles) {
        Plane plane;
        plane.m_normal = m_normal;
        plane.m_d = m_dot;

        auto [points, center] = TrianglePlaneIntersection(triangles, plane);

        std::vector<CircumferenceConstraintImpl::IntersectionPoint> points_in_hull = GiftWrapping(points, center);

        m_indices.clear();
        m_alphas.clear();
        for (const CircumferenceConstraintImpl::IntersectionPoint &p: points_in_hull) {
            m_indices.push_back(p.v0);
            m_indices.push_back(p.v1);
            m_alphas.push_back(p.alpha);
        }
    }

    TorchCircumferenceConstraintImpl::TorchCircumferenceConstraintImpl(std::string name) : TorchConstraintImpl(std::move(name)) {}

    torch::Tensor TorchCircumferenceConstraintImpl::compute_impl(ConstraintArgument &ca) {
        using namespace torch::indexing;
        torch::Tensor verts = ca.v_shaped.index({Slice(), m_segment_index});
        torch::Tensor diff = (verts.index({Slice(), Slice(), 1}) - verts.index({Slice(), Slice(), 0}));

        torch::Tensor point = verts.index({Slice(), Slice(), 0}) + m_segment_alpha.index({None, Slice(), None}) * diff;

        torch::Tensor roll = torch::roll(point, 1, {1});

        torch::Tensor dpoint = point - roll;
        torch::Tensor dist = torch::linalg_norm(dpoint, c10::nullopt, torch::IntArrayRef(2));
        torch::Tensor res = torch::sum(dist);

        torch::Tensor loss = m_weight.load(std::memory_order_relaxed) * torch::abs(res - m_target.load(std::memory_order_relaxed));

        m_loss.store(loss.item().toFloat(), std::memory_order_relaxed);
        m_value.store(res.item().toFloat(), std::memory_order_relaxed);
        return loss;
    }

TWIN_NAMESPACE_END

#endif