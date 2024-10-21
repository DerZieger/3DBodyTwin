#ifdef HAS_DL_MODULE

#include "net/constraints/marker.h"
#include <torch/nn/functional/normalization.h>
#include "gui.h"
#include "util/intersection.h"
#include <glog/logging.h>

#include <utility>

TWIN_NAMESPACE_BEGIN

    int MarkerConstraintBaseImpl::counter = 0;

    MarkerConstraintBaseImpl::MarkerConstraintBaseImpl() : ConstraintImpl("marker_" + std::to_string(counter++)), m_unused_markers(std::vector<std::string>()), m_current_selection(""), m_selected_unused_markers(-1), m_render_texts(false), m_optimize_marker_positions(false) {}

    MarkerConstraintBaseImpl::MarkerConstraintBaseImpl(const std::string &cname) : ConstraintImpl(cname + "_marker_" + std::to_string(counter)), m_torch_constraint(TorchMarkerConstraint(cname + "_marker_" + std::to_string(counter++) + "_torch")), m_unused_markers(std::vector<std::string>()), m_current_selection(""), m_selected_unused_markers(-1), m_render_texts(false), m_optimize_marker_positions(false) {}

    bool MarkerConstraintBaseImpl::drawGUIShort() {
        ImGui::PushID(name.c_str());
        ExchangeData(m_torch_constraint);
        ImGui::Text("Loss: %.3f", m_loss);
        ImGui::PopID();
        return false;
    }

    void MarkerConstraintBaseImpl::drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const Ray &ray, cppgl::vec2 relative_pixel, bool pressed) {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchMarkerConstraint(name + "_Marker_" + std::to_string(m_id) + "_torch");
        }
        ExchangeData(m_torch_constraint);
        ImGui::PushID(("marker_" + std::to_string(m_id)).c_str());
        ImGui::InputText(_labelPrefix("Name:").c_str(), &name);
        if (ImGui::Checkbox(_labelPrefix("OptimizeMarkerPos (deprecated)").c_str(), &m_optimize_marker_positions)) {
            m_torch_constraint->m_switch_optimize_barycenter.store(m_optimize_marker_positions);
            SetChange(Change::ParametersRestart);
        }

        if (ImGui::InputFloat(_labelPrefix("Weight:").c_str(), &m_weight)) {
            m_torch_constraint->m_weight.store(m_weight, std::memory_order_relaxed);
        }
        bool enable = ImGui::Button("EnableAll");
        static bool one_click = false;
        ImGui::SameLine();
        bool disable = ImGui::Button("DisableAll");
        ImGui::Indent(3);
        bool header = ImGui::CollapsingHeader("MarkerList");
        ImGui::Unindent(1);
        if (header && !m_unused_markers.empty()) {

            ImGui::Checkbox("OneClickSelection", &one_click);
            m_selected_unused_markers %= static_cast<int>(m_unused_markers.size());
            ImGui::Text("New Marker:");
            ImGui::SameLine();
            ImGui::Combo("##", &m_selected_unused_markers, m_unused_markers);
            ImGui::SameLine();
            if (ImGui::Button("Add")) {
                std::string name = m_unused_markers.at(m_selected_unused_markers);
                m_markers[name].m_fid = 0;
                m_markers[name].m_baryCoords = cppgl::vec3(1, 0, 0);
                m_markers[name].m_id = 0;
                m_markers[name].m_dis = 0.005f;
                m_markers[name].m_reference = true;
                m_markers[name].m_use = false;
                m_current_selection = name;
                m_unused_markers.erase(m_unused_markers.begin() + m_selected_unused_markers);
            }
        }
        if (enable || disable) {
            SetChange(Change::ParametersReconfigure);
        }
        bool changed = false;
        std::vector<std::string> remove;
        for (std::pair<const std::string, MarkerData> &m: m_markers) {
            if (!m.second.m_reference) {
                continue;
            }
            if (enable) {
                m.second.m_use = true;
            } else if (disable) {
                m.second.m_use = false;
            }
            if (!header) {
                continue;
            }
            ImGui::PushID(m.first.c_str());
            changed |= ImGui::Checkbox("", &m.second.m_use);
            ImGui::SameLine();
            bool selected = (m_current_selection == m.first);
            if (selected) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(ImColor(255, 0, 0)));
                if (ImGui::Button("DisableSelection")) {
                    m_current_selection = "";
                }
                ImGui::PopStyleColor();
            } else {
                if (ImGui::Button("Sel")) {
                    m_current_selection = m.first;
                }
            }
            ImGui::SameLine();
            ImGui::PushItemWidth(100);
            ImGui::Text("%s", m.first.c_str());
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::Text("Distance: %f", m.second.m_dist);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(90);
            ImGui::InputInt("FID:", &m.second.m_fid);
            m.second.m_id = std::min(std::max(m.second.m_fid, 0), static_cast<int>(mesh->mesh->num_indices / 3));
            ImGui::SameLine();
            if (ImGui::Button("Del")) {
                remove.push_back(m.first);
            }
            ImGui::PopID();
        }

        if (changed || !remove.empty()) {
            SetChange(Change::ParametersReconfigure);
        }

        for (std::string &s: remove) {
            m_unused_markers.push_back(s);
            m_markers.erase(s);
        }

        if (!m_current_selection.empty() && pressed) {
            bvh.Update();
            Intersection::RayTriangleIntersection intersection = bvh.getClosest(ray);

            if (intersection.valid && !intersection.backFace) {
                cppgl::vec3 pos = ray.origin + intersection.t * ray.direction;
                m_markers[m_current_selection].m_fid = intersection.triangleIndex;
                m_markers[m_current_selection].m_baryCoords = intersection.barycenter;
                m_markers[m_current_selection].m_dis = 0.005;
                m_markers[m_current_selection].m_pos = (m_model.inverse() * cppgl::make_vec4(pos, 1)).head<3>();
                m_recompute_rendering = true;
                if (one_click) {
                    m_current_selection = "";
                }
                SetChange(Change::ParametersReconfigure);
            }
        }

        ImGui::Unindent(2);
        ExchangeData(m_torch_constraint);
        ImGui::Text("Loss: %.3f", m_loss);
        ImGui::PopID();
    }

    void MarkerConstraintBaseImpl::Finalize() {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchMarkerConstraint(name + "_Marker_" + std::to_string(m_id) + "_torch");
        }
        if (isReferenceSet()) {
            UpdateMarkerData();
        }
    }

    void MarkerConstraintBaseImpl::createDrawelement(const cppgl::Drawelement &dre) {
        if (m_torch_constraint) {
            std::vector<float> distances, pos, poss;
            std::lock_guard<std::mutex> guard(m_torch_constraint->m_lock);
            if (m_torch_constraint->m_residuals.numel() > 0) {
                torch::Tensor res = m_torch_constraint->m_residuals.to(torch::kCPU);
                torch::Tensor cpos = m_torch_constraint->m_current_positions.to(torch::kCPU);
                distances = std::vector<float>(res.data_ptr<float>(), res.data_ptr<float>() + res.numel());
                pos = std::vector<float>(cpos.data_ptr<float>(), cpos.data_ptr<float>() + cpos.numel());
            }
            if (!distances.empty()) {
                int ccounter = 0;
                for (std::pair<const std::string, MarkerData> &m: m_markers) {
                    if (m.second.m_use) {
                        m.second.m_dist = distances[ccounter];
                        m.second.m_pos = cppgl::vec3(pos[3 * ccounter], pos[3 * ccounter + 1], pos[3 * ccounter + 2]);
                        ++ccounter;
                    }
                }
            }

        }
        std::vector<cppgl::vec3> poss;
        std::vector<cppgl::vec4> cols;
        std::vector<uint32_t> ind;
        for (std::pair<const std::string, MarkerData> &m: m_markers) {
            if (m.second.m_use) {
                poss.push_back(m.second.m_pos);
                poss.push_back(m.second.m_target);
                cols.emplace_back(1, 0, 0, 1);
                cols.emplace_back(0, 0, 1, 1);
            }
        }
        if (!m_current_selection.empty() && m_markers.count(m_current_selection)) {
            cppgl::vec3 pos = m_markers[m_current_selection].m_target;
            poss.push_back(pos);
            cols.emplace_back(1, 1, 1, 1);
        }
        ind.resize(poss.size());
        std::iota(ind.begin(), ind.end(), 0);
        if (!m_consRepr) {
            if (!cppgl::Material::valid("markerCon")) {
                cppgl::Material markerMat("markerCon");
            }
            if (!cppgl::Shader::valid("pointCloud")) {
                cppgl::Shader s2("pointCloud", "shader/pointCloud.vert", "shader/pointCloud.frag");
            }
            cppgl::Geometry geo("markercon_geo_" + std::to_string(m_id), poss, ind);
            geo->add_attribute_vec4("color", cols.data(), cols.size());
            cppgl::Mesh meshs("markercon_mesh_" + std::to_string(m_id), geo, cppgl::Material::find("markerCon"), GL_DYNAMIC_DRAW);
            meshs->primitive_type = GL_POINTS;

            m_consRepr = cppgl::Drawelement("markercon_de_" + std::to_string(m_id), cppgl::Shader::find("pointCloud"), meshs);
            geo->register_mesh(meshs);
            m_consRepr->add_pre_draw_func("pointSize", []() {
                glPointSize(5);
            });
        } else {
            if (m_consRepr->mesh->num_indices == ind.size()) {
                m_consRepr->mesh->update_vertex_buffer(0, poss.data());
            } else {
                std::vector<float> posss(3 * poss.size());
#pragma omp parallel for default(none) shared(poss, posss)
                for (unsigned long int i = 0; i < poss.size(); ++i) {
                    posss.at(i * 3) = poss.at(i)[0];
                    posss.at(i * 3 + 1) = poss.at(i)[1];
                    posss.at(i * 3 + 2) = poss.at(i)[2];
                }
                m_consRepr->mesh->geometry->set(posss, ind);
                m_consRepr->mesh->geometry->add_attribute_vec4("color", cols.data(), cols.size(), false, true);
                m_consRepr->mesh->geometry->update_meshes();
            }
        }

    }

    TorchConstraint MarkerConstraintBaseImpl::GetConstraint() {
        return m_torch_constraint;
    }

    ConstraintImpl::Change MarkerConstraintBaseImpl::OnFrameChange() {
        if (isReferenceSet()) {
            UpdateMarkerData();
        }
        return Change::ParametersReconfigure;
    }

    bool MarkerConstraintBaseImpl::isReferenceSet() const {
        return true;
    }


    TorchMarkerConstraintImpl::TorchMarkerConstraintImpl(std::string name) : TorchConstraintImpl(std::move(name)), m_optimize_barycenter(false), m_switch_optimize_barycenter(false) {}

    bool TorchMarkerConstraintImpl::hasOptimizableParams() const {
        return m_optimize_barycenter.load() || m_switch_optimize_barycenter.load();
    }

    std::vector<torch::Tensor> TorchMarkerConstraintImpl::OptimizableParams() {
        std::lock_guard<std::mutex> guard(m_lock);

        const bool cur = m_optimize_barycenter.load();
        const bool nex = m_switch_optimize_barycenter.load();

        if (cur == nex) {
            std::abort();
        }
        m_switch_optimize_barycenter.store(cur);

        if (nex) {
            return {m_barycenter};
        } else {
            return {};
        }
    }

    torch::Tensor TorchMarkerConstraintImpl::compute_impl(ConstraintArgument &ca) {
        using namespace torch::indexing;
        torch::Tensor vi = ca.faces.index({m_faces});
        torch::Tensor vs = ca.v_posed.index({Slice(), vi});
        torch::Tensor diff0 = (vs.index({Slice(), Slice(), 1}) - vs.index({Slice(), Slice(), 0}));
        torch::Tensor diff1 = (vs.index({Slice(), Slice(), 2}) - vs.index({Slice(), Slice(), 0}));
        torch::Tensor normal = torch::linalg_cross(diff0, diff1);
        torch::Tensor normaln = torch::nn::functional::normalize(normal);//Normalized normal
        torch::Tensor point = torch::einsum("bckd,ck->bcd", {vs, m_barycenter});
        torch::Tensor point2 = point + normaln * m_dis.index({None, Slice(), None});

        torch::Tensor dist = point2 - m_positions;
        torch::Tensor res = torch::linalg_norm(dist, torch::nullopt, torch::IntArrayRef(2));

        torch::Tensor sum = m_weight.load(std::memory_order_relaxed) * torch::sum(res, 1);

        if (m_optimize_barycenter.load(std::memory_order_relaxed)) {
            torch::Tensor abary = torch::abs(m_barycenter);
            torch::Tensor sum1 = torch::sum(abary, 1);
            torch::Tensor abary1 = sum1 - 1.0;
            torch::Tensor relu = torch::nn::functional::relu(abary1, 0);
            sum += torch::sum(relu);
        }

        m_loss.store(sum.item().toFloat(), std::memory_order_relaxed);

        m_residuals = res * 100;
        m_current_positions = point2;

        return sum;
    }


    void to_json(nlohmann::json &j, const MarkerData &d) {
        j["id"] = d.m_id;
        j["faceId"] = d.m_fid;
        j["distanceToSkin"] = d.m_dis;
        j["baryCoords"] = d.m_baryCoords;
    }

    void from_json(const nlohmann::json &j, MarkerData &d) {
        d.m_id = j["id"];
        d.m_fid = j["faceId"];
        d.m_dis = j["distanceToSkin"];
        const nlohmann::json &b = *j.find("baryCoords");
        int i = 0;
        for (auto iter = b.begin(); iter != b.end(); ++iter, ++i) {
            d.m_baryCoords[i % 3] = iter.value().get<float>();
        }
    }

TWIN_NAMESPACE_END
#endif