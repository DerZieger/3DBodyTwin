#ifdef HAS_DL_MODULE

#include "net/constraints/constraintHandler.h"
#include <filesystem>
#include "net/constraints/betalock.h"
#include "net/constraints/distance.h"
#include "net/constraints/volume.h"
#include "net/constraints/convex_circumference.h"
#include "net/constraints/osimMarker.h"
#include "net/constraints/c3dMarker.h"
#include "net/constraints/jointangle.h"
#include "net/constraints/jsonMarker.h"
#include "gui.h"

TWIN_NAMESPACE_BEGIN

    int ConstraintHandler::c_count = 0;
    bool ConstraintHandler::c_gui = false;

    ConstraintHandler::ConstraintHandler(std::string config) : m_path(std::move(config)), m_configuring(-1), m_resetCol(false), m_frame(0), m_id(c_count++), m_selection(-1) {
        Load();
        cppgl::gui_add_callback("ConstraintHandler", drawClassGUI);
    }

    void ConstraintHandler::Clear() {
        m_con_map.clear();
        m_id_map.clear();
        m_name_map.clear();
    }

    void ConstraintHandler::Refresh(std::vector<std::shared_ptr<Osim>> &osim, std::vector<std::shared_ptr<C3D>> &c3d, std::vector<std::shared_ptr<JMarker>> &mjs, const cppgl::mat4 &model, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped) {
        if (!m_json.empty()) {
            for (auto iter = m_json.begin(); iter != m_json.end(); ++iter) {
                const std::string &name = iter.key();
                if (!m_id_map.count(name)) {
                    auto contype = iter->find("type")->begin().value().get<std::string>();
                    Constraint constraint;
                    if (contype == "Distance") {
                        constraint = DistanceConstraint(name);
                    } else if (contype == "Volume") {
                        constraint = VolumeConstraint(name);
                    } else if (contype == "Circumference") {
                        constraint = CircumferenceConstraint(name);
                    } else if (contype == "Beta") {
                        constraint = BetaLockConstraint(name);
                    } else if (contype == "OsimMarker") {
                        constraint = OsimMarkerConstraint(name);
                    } else if (contype == "C3DMarker") {
                        constraint = C3DMarkerConstraint(name);
                    } else if (contype == "Angle") {
                        constraint = AngleConstraint(name);
                    } else if (contype == "JSONMarker") {
                        constraint = JsonMarkerConstraint(name);
                    } else {
                        throw new std::invalid_argument("Wrong Constraint Type");
                    }

                    constraint->Deserialize(m_json, name);
                    constraint->UpdateReferencePose(m_frame);
                    if (!(constraint.isType<OsimMarkerConstraintImpl>() || constraint.isType<C3DMarkerConstraintImpl>() || constraint.isType<JsonMarkerConstraintImpl>())) {
                        constraint->Finalize();
                    }
                    constraint->SetModel(model);
                    m_id_map[name] = constraint->m_id;
                    m_con_map[constraint->m_id] = {false, true, constraint};
                    m_name_map[constraint->m_id] = name;
                }
            }
        }
        RecomputeRendering(mesh, true);
    }

    void
    ConstraintHandler::RenderImGui(Network &net, std::vector<std::shared_ptr<Osim>> &osim, std::vector<std::shared_ptr<C3D>> &c3d, std::vector<std::shared_ptr<JMarker>> &mjs, Constraints &constraints, cppgl::mat4 model, BVH &bvh, const Ray &ray, const cppgl::vec2 &local_pixel, bool pressed, bool has_changed, int &zeroFrame, int &maxFramerate) {

        if (c_gui) {
            if (ImGui::Begin(std::string("Constraints##").c_str(), &c_gui)) {
                ImGui::PushID("ConstraintHandler");
                if (ImGui::CollapsingHeader(std::to_string(m_id).c_str())) {
                    ImGui::PushID(m_id);
                    if (ImGui::Button("Reload Defaults")) {
                        Refresh(osim, c3d, mjs, model, net->m_obj, net->m_mesh_shaped);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Clear and reload")) {
                        constraints->Clear();
                        Clear();
                        Refresh(osim, c3d, mjs, model, net->m_obj, net->m_mesh_shaped);
                    }

                    auto add_constraint = [&](Constraint &constraint) {
                        constraint->Finalize();
                        constraint->SetModel(model);
                        constraints->Add(constraint);
                        m_con_map[constraint->m_id] = {true, true, constraint};
                    };

                    if (!m_new_constraint) {
                        const char *constraint_names[] = {"Distance", "Volume", "Circumference", "BetaLock", "OsimMarker", "JointConstraint", "C3DMarker", "Angle", "JsonMarker"};
                        ImGui::Combo("New Constraint", &m_selection, constraint_names, sizeof(constraint_names) / sizeof(constraint_names[0]));
                        ImGui::SameLine();
                        if (ImGui::Button("Add")) {
                            if (m_selection == 0) {
                                m_new_constraint = std::make_shared<DistanceConstraintImpl>();
                            } else if (m_selection == 1) {
                                m_new_constraint = std::make_shared<VolumeConstraintImpl>();
                            } else if (m_selection == 2) {
                                m_new_constraint = std::make_shared<CircumferenceConstraintImpl>();
                            } else if (m_selection == 3) {
                                m_new_constraint = std::make_shared<BetaLockConstraintImpl>();
                            } else if (m_selection == 4) {
                                m_new_constraint = std::make_shared<OsimMarkerConstraintImpl>();
                            } else if (m_selection == 6) {
                                m_new_constraint = std::make_shared<C3DMarkerConstraintImpl>();
                            } else if (m_selection == 7) {
                                m_new_constraint = std::make_shared<AngleConstraintImpl>();
                            } else if (m_selection == 8) {
                                m_new_constraint = std::make_shared<JsonMarkerConstraintImpl>();
                            }
                        }
                    } else {
                        std::shared_ptr<OsimMarkerConstraintImpl> optr = std::dynamic_pointer_cast<OsimMarkerConstraintImpl>(m_new_constraint);
                        if (optr != nullptr && !(optr->isReferenceSet())) {
                            optr->referenceGUI(osim);
                        }
                        std::shared_ptr<C3DMarkerConstraintImpl> cptr = std::dynamic_pointer_cast<C3DMarkerConstraintImpl>(m_new_constraint);
                        if (cptr != nullptr && !(cptr->isReferenceSet())) {
                            cptr->referenceGUI(c3d);
                        }
                        std::shared_ptr<JsonMarkerConstraintImpl> jptr = std::dynamic_pointer_cast<JsonMarkerConstraintImpl>(m_new_constraint);
                        if (jptr != nullptr && !(jptr->isReferenceSet())) {
                            jptr->referenceGUI(mjs);
                        }
                        m_new_constraint->drawGUIEdit(bvh, net->m_obj, net->m_mesh_shaped, ray, local_pixel, pressed);
                        if (auto con = std::dynamic_pointer_cast<CircumferenceConstraintImpl>(m_new_constraint)) {
                            net->SetColor(con->m_faces, cppgl::vec4(1, 0, 0, 1));
                        }
                        if (ImGui::Button("Finish")) {
                            Constraint tmpc(m_new_constraint);
                            tmpc->UpdateReferencePose(m_frame);
                            add_constraint(tmpc);
                            m_new_constraint.reset();
                            m_resetCol = true;
                        }
                    }
                    if (m_new_constraint) m_configuring = m_new_constraint->m_id;


                    std::vector<int> remove;
                    for (std::pair<const int, std::tuple<bool, bool, Constraint >> &rep: m_con_map) {
                        auto &[active, render, constr] = rep.second;
                        std::string name = constr->name;
                        ImGui::PushID(rep.first);
                        if (ImGui::Checkbox("##", &active)) {
                            if (active && constr->isReferenceSet()) {
                                constraints->Add(constr);
                                zeroFrame = constr->getZeroFrame();
                            } else {
                                constraints->Remove(name);
                                zeroFrame = -1;
                                maxFramerate = -1;
                            }
                        }
                        if (active && constr->isReferenceSet()) {
                            maxFramerate = std::max(maxFramerate, constr->getFramerate());
                        }
                        ImGui::SameLine();
                        ImGui::PushID(1);
                        ImGui::Checkbox("##", &render);
                        ImGui::SameLine();
                        ImGui::PushItemWidth(150);
                        constr->UsesTarget();
                        ImGui::PopItemWidth();
                        ImGui::SameLine();
                        ImGui::SetNextItemOpen(m_configuring == rep.first);
                        if (ImGui::CollapsingHeader(name.c_str())) {
                            m_configuring = rep.first;
                            ImGui::BeginGroup();
                            ImGui::Indent(40);
                            //  this may change the name!
                            if (constr->isReferenceSet()) {
                                constr->drawGUIEdit(bvh, net->m_obj, net->m_mesh_shaped, ray, local_pixel, pressed);
                            } else {
                                if (constr.isType<OsimMarkerConstraintImpl>()) {
                                    std::dynamic_pointer_cast<OsimMarkerConstraintImpl>(constr.ptr)->referenceGUI(osim);
                                } else if (constr.isType<C3DMarkerConstraintImpl>()) {
                                    std::dynamic_pointer_cast<C3DMarkerConstraintImpl>(constr.ptr)->referenceGUI(c3d);
                                } else if (constr.isType<JsonMarkerConstraintImpl>()) {
                                    std::dynamic_pointer_cast<JsonMarkerConstraintImpl>(constr.ptr)->referenceGUI(mjs);
                                }
                            }
                            if (constr->name != name) {
                                m_id_map.erase(name);
                                m_id_map[constr->name] = rep.first;
                            }
                            if (ImGui::Button("Save Changes as default")) {
                                AddNewDefaultConstraint(constr);
                            }
                            ImGui::SameLine();
                            if (ImGui::Button("Remove from default")) {
                                RemoveDefaultConstraint(name);
                            }
                            if (ImGui::Button("Remove")) {
                                remove.push_back(constr->m_id);
                            }
                            ImGui::Unindent(40);
                            ImGui::EndGroup();
                        } else {
                            if (m_configuring == rep.first) {
                                m_configuring = -1;
                            }
                            if (active) {
                                constr->drawGUIShort();
                            }
                        }
                        ImGui::PopID();
                        ImGui::PopID();
                    }

                    if (m_con_map.count(m_configuring)) {
                        if (auto geo = std::dynamic_pointer_cast<CircumferenceConstraintImpl>(std::get<2>(m_con_map[m_configuring]).ptr)) {
                            net->SetColor(geo->m_faces, cppgl::vec4(1, 0, 0, 1));
                            m_resetCol = true;
                        }
                    } else if (m_resetCol) {
                        net->SetColor({}, cppgl::vec4(0, 0, 0, 0));
                        m_resetCol = false;
                    }


                    for (int id: remove) {
                        auto &[a, r, c] = m_con_map[id];
                        constraints->Remove(c->name);
                        m_id_map.erase(c->name);
                        m_con_map.erase(id);
                    }

                    ImGui::PopID();
                }
                ImGui::PopID();
            }
            ImGui::End();
        }
    }

    void ConstraintHandler::AddNewDefaultConstraint(Constraint con) {
        if (m_id_map.count(con->name)) {
            int id = m_id_map[con->name];
            if (m_name_map[id] != con->name)
                RemoveDefaultConstraint(m_name_map[id]);
        }
        con->Serialize(m_json);
        Save();
    }

    void ConstraintHandler::RemoveDefaultConstraint(std::string con) {
        m_json.erase(con);
        Save();
    }

    void ConstraintHandler::Render(cppgl::mat4 model) {
        for (std::pair<const int, std::tuple<bool, bool, Constraint >> &rep: m_con_map) {
            auto &[active, render, con] = rep.second;
            if (active && render && con->m_consRepr) {
                if (model != cppgl::mat4::Identity()) {
                    cppgl::mat4 tmpmat = con->m_consRepr->model;
                    con->m_consRepr->set_model_transform(model * con->m_consRepr->model);
                    con->render();
                    con->m_consRepr->set_model_transform(tmpmat);
                } else {
                    con->render();
                }
            }
        }
    }

    void ConstraintHandler::RecomputeRendering(const cppgl::Drawelement &mesh,
                                               bool mesh_changed) {
        for (std::pair<const int, std::tuple<bool, bool, Constraint >> &rep: m_con_map) {
            auto &[active, render, con] = rep.second;
            if (active && render && (mesh_changed || con->NeedRecomputeRenderObjects())) {
                con->createDrawelement(mesh);
            }
        }
    }

    void ConstraintHandler::Load() {
        if (!std::filesystem::exists(m_path))return;

        std::ifstream in(m_path);
        in >> m_json;
    }

    void ConstraintHandler::Save() {
        std::ofstream out(m_path);
        out << m_json.dump(4);
    }

    void ConstraintHandler::SetFrame(int frame) {
        m_frame = frame;
        for (std::pair<const int, std::tuple<bool, bool, Constraint >> &rep: m_con_map) {
            auto &[active, render, con] = rep.second;
            if (active) {
                con->UpdateReferencePose(frame);
            }
        }
    }

    void ConstraintHandler::drawClassGUI() {
        if (cppgl::Context::show_gui) {
            if (ImGui::BeginMainMenuBar()) {
                ImGui::Separator();
                ImGui::Checkbox("Constraints", &c_gui);
                ImGui::EndMainMenuBar();
            }
        }
    }

    int ConstraintHandler::getFramerate() const {
        int fr = -1;
        for (const std::pair<const int, std::tuple<bool, bool, Constraint >> &rep: m_con_map) {
            const auto &[active, render, constr] = rep.second;
            if (active) {
                fr = std::max(fr, constr->getFramerate());
            }
        }
        return fr;
    }
TWIN_NAMESPACE_END

#endif