#include <set>
#include <glog/logging.h>
#include "gui.h"
#include "jsonMarkers.h"
#include <fstream>
#include <nlohmann/json.hpp>

TWIN_NAMESPACE_BEGIN

    int JMarker::c_count = 0;
    bool JMarker::c_gui = false;


    JMarker::JMarker(const std::string &path) : m_path(path), m_numFrames(-1), m_numPoints(-1), m_framerate(-1),                                                m_curFrame(0),                                                m_err(false), m_draw(true), m_id(c_count++) {
        cppgl::gui_add_callback("jmarkergui" + std::to_string(m_id), twin::GuiCallbackWrapper<twin::JMarker>, this);
        cppgl::gui_add_callback("jmarker", drawClassGUI);
        if (!path.empty()) {
            if (!readFile(path)) {
                m_err = true;
                LOG(ERROR) << "JMarker file at " << path << " doesn't exist" << std::endl;
            }
        } else {
            m_err = true;
        }
    }

    JMarker::~JMarker() {
        cppgl::gui_remove_callback("jmarkergui" + std::to_string(m_id));
    }

    std::vector<std::vector<cppgl::vec3>> JMarker::getPointData() const {
        return m_pointData;
    }

    std::vector<cppgl::vec3> JMarker::getFramePoints(int frame) const {
        if (frame == -1) {
            return m_pointData.at(m_curFrame);
        } else if (frame < -1 || frame >= m_numFrames) {
            LOG(FATAL) << "Access of index " << frame << " in getFramePoints is out of bounds" << std::endl;
            return {};
        }
        return m_pointData.at(frame);
    }

    std::map<std::string, cppgl::vec3> JMarker::getFrameMarkers(int frame) const {
        if (frame == -1) {
            return m_markerData.at(m_curFrame);
        } else if (frame < 0 || frame >= m_numFrames) {
            LOG(FATAL) << "Access of index " << frame << " in getFrameMarkers is out of bounds" << std::endl;
            return {};
        }
        return m_markerData.at(frame);
    }

    std::vector<cppgl::vec3> JMarker::getPointFrames(const std::string &name) const {
        if (m_nameIdMap.count(name) < 1) {
            LOG(ERROR) << "Access of point with name " << name << " in getPointFrames does not exist" << std::endl;
            return {};
        }
        return getPointFrames(m_nameIdMap.at(name));
    }

    std::vector<cppgl::vec3> JMarker::getPointFrames(int id) const {
        if (id < 0 || id >= m_numPoints) {
            LOG(FATAL) << "Access of index " << id << " in getPointFrames is out of bounds" << std::endl;
            return {};
        }
        std::vector<cppgl::vec3> ret(m_numFrames);

        for (int i = 0; i < m_numFrames; ++i) {
            ret.at(i) = m_pointData.at(i).at(id);
        }

        return ret;
    }

    std::vector<Event> JMarker::getEvents() const {
        return m_events;
    }

    int JMarker::getNumFrames() const {
        return m_numFrames;
    }

    int JMarker::getNumPoints() const {
        return m_numPoints;
    }

    int JMarker::getFramerate() const {
        return m_framerate;
    }

    int JMarker::getSyncFrame() const {
        return m_zeroframe;
    }

    void JMarker::setFrame(int frame) {
        if (frame < 0 || frame >= m_numFrames) {
            LOG(FATAL) << "Access of index " << frame << " in getFrameMarkers is out of bounds" << std::endl;
            return;
        }
        m_curFrame = frame;
        if (m_repr) {
            m_repr->mesh->update_vertex_buffer(0, m_pointData.at(frame).data());
        }
    }

    void JMarker::drawGUI() {


        if (c_gui) {
            if (ImGui::Begin(std::string("JMarker##").c_str(), &c_gui)) {
                ImGui::PushID("jmarker");
                if (ImGui::CollapsingHeader(std::to_string(m_id).c_str())) {
                    ImGui::PushID(m_id);
                    ImGui::InputText(_labelPrefix("Filepath:").c_str(), &m_path);
                    if (m_err) {
                        if (ImGui::Button("Load file##")) {
                            m_err = !readFile(m_path);
                        }
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                        ImGui::Text("Path doesn't lead to valid file destination!");
                        ImGui::PopStyleColor();
                    } else {
                        if (ImGui::SliderInt(_labelPrefix("Motion frame").c_str(), &m_curFrame, 0, m_numFrames - 1)) {
                            setFrame(m_curFrame);
                        }
                        ImGui::Checkbox(_labelPrefix("Draw points:").c_str(), &m_draw);
                        ImGui::Text("Frames: %d", m_numFrames);
                        ImGui::Text("Framerate: %d", m_framerate);
                        ImGui::Text("Number of Points: %d", m_numPoints);
                        if(ImGui::Combo(_labelPrefix("Sync Event").c_str(),&m_framesel,m_eventnames)){
                            m_zeroframe=m_events.at(m_framesel).frame;
                        }
                        ImGui::InputInt(_labelPrefix("Synchronization Frame:").c_str(), &m_zeroframe);
                    }

                    ImGui::PopID();
                }
                ImGui::PopID();
            }
            ImGui::End();
        }
    }

    void JMarker::Render() const {
        if (m_repr && m_draw) {
            m_repr->bind();
            m_repr->draw();
            m_repr->unbind();
        }
    }

    void JMarker::drawClassGUI() {
        if (cppgl::Context::show_gui) {
            if (ImGui::BeginMainMenuBar()) {

                ImGui::Separator();
                ImGui::Checkbox("JMarker##", &c_gui);
                ImGui::EndMainMenuBar();
            }
        }
    }

    std::ostream &operator<<(std::ostream &os, const JMarker &jmarker) {
        os << "[JMarker] number of frames: " << jmarker.getNumFrames() << " number of points: "           << jmarker.getNumPoints()           << " capture framerate: " << jmarker.getFramerate() << "\n";
        os << "\nMarkernames:\n";
        for (const std::string &n: jmarker.m_markerNames) {
            os << n << "\n";
        }

        os << "\nJointnames:\n";
        for (const std::string &j: jmarker.m_gaitBonesNames) {
            os << j << "\n";
        }
        os << "\n";
        std::vector<std::vector<cppgl::vec3>> points = jmarker.getPointData();
        for (unsigned long int f = 0; f < points.size(); ++f) {
            os << "Frame: " << f << "\n";
            for (unsigned long int p = 0; p < points.at(f).size(); ++p) {
                os << "[ " << points.at(f).at(p).x() << ",\t" << points.at(f).at(p).y() << ",\t"                   << points.at(f).at(p).z() << "]\n";
            }
            os << "\n";
        }
        os << std::endl;
        return os;
    }

    bool JMarker::readFile(const std::string &path) {
        try {
            m_nameIdMap.clear();
            m_markerNames.clear();
            m_gaitBonesNames.clear();
            m_ununsedNames.clear();
            m_pointData.clear();
            m_markerData.clear();
            m_gaitBoneData.clear();
            m_events.clear();
            std::ifstream f(path);
            nlohmann::json data = nlohmann::json::parse(f);
            m_numPoints = data["nPoints"];
            m_numFrames = data["nFrames"];
            m_framerate = data["framerate"];
            std::vector<std::string> allnames;
            if (m_numFrames > 0) {
                nlohmann::json &zframe = data["Frames"]["0"];
                for (auto &kv: zframe.items()) {
                    allnames.emplace_back(kv.key());
                }
            }


            m_events.resize(data["nEvents"]);
            for(uint32_t i=0;i<m_events.size();++i){
                for(auto & [k,v]:data["Events"][std::to_string(i)].items()){
                    m_events.at(i).event=k;
                    m_events.at(i).frame=v;
                }
            }


            std::sort(m_events.begin(), m_events.end());
            m_zeroframe = m_events.empty() ? -1 : m_events.front().frame;
            for(auto& ev:m_events){
                m_eventnames.push_back(ev.event);
            }
            // https://docs.vicon.com/display/Nexus215/Full+body+modeling+with+Plug-in+Gait
            // https://docs.vicon.com/download/attachments/133828966/Plug-in%20Gait%20Reference%20Guide.pdf?version=2&modificationDate=1637681079000&api=v2
            // Here add markernames
            const std::set<std::string> possibleMarkernames = {"RFHD", "LFHD", "LBHD", "RBHD", "RSHO", "LSHO",
                                                               "CLAV", "STRN",
                                                               "LUPA", "RUPA", "LELB", "RELB", "LFRM", "RFRM",
                                                               "LASI", "RASI", "RWRA", "RWRB", "LWRA", "LWRB",
                                                               "RFIN", "LFIN", "RTHAP", "LTHAP", "RTHI", "LTHI",
                                                               "RTAD", "LTAD", "RKNE", "RKNM", "LKNE", "LKNM",
                                                               "RTIAP", "LTIAP", "RTIB", "LTIB", "LTIAD",
                                                               "RTIAD", "RANK", "LANK", "RMED", "LMED", "RTOE",
                                                               "LTOE", "C7", "RBAK", "T10", "RPSI", "LPSI",
                                                               "RHEE", "LHEE", "LKAX", "RKAX", "RKD1", "RKD2",
                                                               "LKD1", "LKD2", "SCAR", "SACR", "SHO", "ULN", "RAD",
                                                               "W1", "W2", "W3", "W4", "L1", "L2", "L3", "L4", "L5",
                                                               "R1", "R2", "R3", "R4", "R5", "M1", "M2", "M3", "M4",
                                                               "M5", "I1", "I2", "I3", "I4", "I5", "T1", "T2", "T3",
                                                               "T4", "T5", "T8"};

            const std::set<std::string> possibleGaitBonesnames = {"PELO", "PELP", "PELA", "PELL", "RFEO", "RFEP",
                                                                  "RFEA",
                                                                  "RFEL", "LFEO", "LFEP", "LFEA", "LFEL", "RTIO",
                                                                  "RTIP",
                                                                  "RTIA", "RTIL", "LTIO", "LTIP", "LTIA", "LTIL",
                                                                  "HEDO",
                                                                  "HEDP", "HEDA", "HEDL", "TRXO", "TRXP", "TRXA",
                                                                  "TRXL",
                                                                  "CSPO", "CSPP", "CSPA", "CSPL", "SACO", "SACP",
                                                                  "SACA",
                                                                  "SACL", "RCLO", "RCLP", "RCLA", "RCLL", "RFOO",
                                                                  "RFOP",
                                                                  "RFOA", "RFOL", "LFOO", "LFOP", "LFOA", "LFOL",
                                                                  "RTOO",
                                                                  "RTOP", "RTOA", "RTOL", "LTOO", "LTOP", "LTOA",
                                                                  "LTOL",
                                                                  "LCLO", "LCLP", "LCLA", "LCLL", "RHUO", "RHUP",
                                                                  "RHUA",
                                                                  "RHUL", "LHUO", "LHUP", "LHUA", "LHUL", "RRAO",
                                                                  "RRAP",
                                                                  "RRAA", "RRAL", "LRAO", "LRAP", "LRAA", "LRAL",
                                                                  "RHNO",
                                                                  "RHNP", "RHNA", "RHNL", "LHNO", "LHNP", "LHNA",
                                                                  "LHNL",
                                                                  "RFIO", "RFIP", "RFIA", "RFIL", "LFIO", "LFIP",
                                                                  "LFIA",
                                                                  "LFIL", "RTBO", "RTBP", "RTBA", "RTBL", "LTBO",
                                                                  "LTBP",
                                                                  "LTBA", "LTBL"};

            for (unsigned long int i = 0; i < allnames.size(); ++i) {
                m_nameIdMap[allnames.at(i)] = static_cast<int>(i);

                if (possibleMarkernames.count(allnames.at(i)) > 0) {
                    m_markerNames.push_back(allnames.at(i));
                } else if (possibleGaitBonesnames.count(allnames.at(i)) > 0) {
                    m_gaitBonesNames.push_back(allnames.at(i));
                } else {
                    m_ununsedNames.push_back(allnames.at(i));
                }
            }

            m_pointData.resize(m_numFrames);
            for (int f = 0; f < m_numFrames; ++f) {
                m_pointData.at(f).resize(m_numPoints);
                int p = 0;
                for (auto &[k, v]: data["Frames"][std::to_string(f)].items()) {
                    std::vector<float> pos = v.get<std::vector<float>>();
                    m_pointData.at(f).at(p++) = cppgl::vec3(pos.at(0), pos.at(1), pos.at(2));
                }
            }

            //Hopefully remove all nans
            for (int cf = 0; cf < m_numFrames; ++cf) {
                std::vector<cppgl::vec3> &ps = m_pointData.at(cf);
                for (int cp = 0; cp < m_numPoints; ++cp) {
                    if (std::isnan(ps.at(cp).x())) {
                        if (cf == 0) {
                            ps.at(cp) = m_pointData.at(cf + 1).at(cp);//Case on the border just take the next frame
                        } else if (cf == m_numFrames - 1) {
                            ps.at(cp) = m_pointData.at(cf - 1).at(cp);//Case on the border just take the previous frame
                        } else if (!std::isnan(m_pointData.at(cf - 1).at(cp).x()) &&                                   !std::isnan(m_pointData.at(cf + 1).at(cp).x())) {
                            ps.at(cp) = 0.5 * (m_pointData.at(cf - 1).at(cp) + m_pointData.at(cf + 1).at(                                    cp));//Just linearly interpolate if neighbouring frame have data
                        }
                    }
                }
            }

            m_markerData.resize(m_numFrames);
            m_gaitBoneData.resize(m_numFrames);
            for (int f = 0; f < m_numFrames; ++f) {
                m_markerData.at(f).clear();
                for (const std::string &name: m_markerNames) {
                    m_markerData.at(f)[name] = m_pointData.at(f).at(m_nameIdMap.at(name));
                }
                m_gaitBoneData.at(f).clear();
                for (const std::string &name: m_gaitBonesNames) {
                    m_gaitBoneData.at(f)[name] = m_pointData.at(f).at(m_nameIdMap.at(name));
                }
            }

            if (!m_repr) {
                if (!cppgl::Shader::valid("pointCloud")) {
                    cppgl::Shader s("pointCloud", "shader/pointCloud.vert", "shader/pointCloud.frag");
                }
                if (!cppgl::Material::valid("c3dMat")) {
                    cppgl::Material markerMat("c3dMat");
                }
                std::vector<cppgl::vec4> cols(m_numPoints, cppgl::vec4(1, 1, 0, 1));
                for (const std::string &n: possibleMarkernames) {
                    if (m_nameIdMap.count(n) != 0) {
                        cols.at(m_nameIdMap.at(n)) = cppgl::vec4(0, 0, 1, 1);
                    }
                }
                for (const std::string &n: possibleGaitBonesnames) {
                    if (m_nameIdMap.count(n) != 0) {
                        cols.at(m_nameIdMap.at(n)) = cppgl::vec4(0, 1, 0, 1);
                    }
                }
                cppgl::Geometry geo("c3d_geo_" + path, m_pointData.at(0));
                geo->add_attribute_vec4("color", cols);
                cppgl::Mesh me("c3d_mesh_" + path, geo, cppgl::Material::find("c3dMat"), GL_DYNAMIC_DRAW);
                geo->register_mesh(me);
                me->primitive_type = GL_POINTS;
                m_repr = cppgl::Drawelement("c3d_" + path, cppgl::Shader::find("pointCloud"), me);
                m_repr->add_pre_draw_func("pointSize", []() {
                    glPointSize(5);
                });
            } else {
                std::vector<cppgl::vec4> cols(m_numPoints, cppgl::vec4(1, 1, 0, 1));
                for (const std::string &n: possibleMarkernames) {
                    if (m_nameIdMap.count(n) != 0) {
                        cols.at(m_nameIdMap.at(n)) = cppgl::vec4(0, 0, 1, 1);
                    }
                }
                for (const std::string &n: possibleGaitBonesnames) {
                    if (m_nameIdMap.count(n) != 0) {
                        cols.at(m_nameIdMap.at(n)) = cppgl::vec4(0, 1, 0, 1);
                    }
                }
                std::vector<uint32_t> ind(m_numPoints);
                std::iota(ind.begin(), ind.end(), 0);
                std::vector<float> pos(3 * m_numPoints);
#pragma omp parallel for default(none) shared(pos, m_pointData)
                for (int p = 0; p < m_numPoints; ++p) {
                    pos.at(p * 3) = m_pointData.at(m_curFrame).at(p)[0];
                    pos.at(p * 3 + 1) = m_pointData.at(m_curFrame).at(p)[1];
                    pos.at(p * 3 + 2) = m_pointData.at(m_curFrame).at(p)[2];
                }
                m_repr->mesh->geometry->set(pos, ind);
                m_repr->mesh->geometry->add_attribute_vec4("color", cols, false, true);
                m_repr->mesh->geometry->update_meshes();
            }

            return true;
        } catch (const std::exception &ex) {
            LOG(ERROR) << ex.what();
            return false;
        }
    }

TWIN_NAMESPACE_END