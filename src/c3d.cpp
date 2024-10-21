#include "c3d.h"
#include "ezc3d/ezc3d.h"
#include "ezc3d/Header.h"
#include "ezc3d/Data.h"
#include "ezc3d/Parameters.h"
#include <set>
#include <glog/logging.h>
#include "gui.h"

TWIN_NAMESPACE_BEGIN
    int C3D::c_count = 0;
    bool C3D::c_gui = false;

    C3D::C3D(const std::string &path) : m_path(path), m_framesel(0), m_numFrames(-1), m_numPoints(-1), m_framerate(-1), m_curFrame(0), m_err(false), m_draw(true), m_id(c_count++) {
        cppgl::gui_add_callback("c3dgui" + std::to_string(m_id), twin::GuiCallbackWrapper<twin::C3D>, this);
        cppgl::gui_add_callback("c3d", drawClassGUI);
        if (!path.empty()) {
            if (!readFile(path)) {
                m_err = true;
                LOG(ERROR) << "C3D file at " << path << " doesn't exist" << std::endl;
            }
        } else {
            m_err = true;
        }
    }

    C3D::~C3D() {
        cppgl::gui_remove_callback("c3dgui" + std::to_string(m_id));
    }

    std::vector<std::vector<cppgl::vec3>> C3D::getPointData() const {
        return m_pointData;
    }

    std::vector<cppgl::vec3> C3D::getFramePoints(int frame) const {
        if (frame == -1) {
            return m_pointData.at(m_curFrame);
        } else if (frame < -1 || frame >= m_numFrames) {
            LOG(FATAL) << "Access of index " << frame << " in getFramePoints is out of bounds" << std::endl;
            return {};
        }
        return m_pointData.at(frame);
    }

    std::map<std::string, cppgl::vec3> C3D::getFrameMarkers(int frame) const {
        if (frame == -1) {
            return m_markerData.at(m_curFrame);
        } else if (frame < 0 || frame >= m_numFrames) {
            LOG(FATAL) << "Access of index " << frame << " in getFrameMarkers is out of bounds" << std::endl;
            return {};
        }
        return m_markerData.at(frame);
    }

    std::vector<cppgl::vec3> C3D::getPointFrames(const std::string &name) const {
        if (m_nameIdMap.count(name) < 1) {
            LOG(ERROR) << "Access of point with name " << name << " in getPointFrames does not exist" << std::endl;
            return {};
        }
        return getPointFrames(m_nameIdMap.at(name));
    }

    std::vector<cppgl::vec3> C3D::getPointFrames(int id) const {
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

    std::vector<Event> C3D::getEvents() const {
        return m_events;
    }

    int C3D::getNumFrames() const {
        return m_numFrames;
    }

    int C3D::getNumPoints() const {
        return m_numPoints;
    }

    int C3D::getFramerate() const {
        return m_framerate;
    }

    int C3D::getSyncFrame() const {
        return m_zeroframe;
    }

    void C3D::setFrame(int frame) {
        if (frame < 0 || frame >= m_numFrames) {
            LOG(FATAL) << "Access of index " << frame << " in getFrameMarkers is out of bounds" << std::endl;
            return;
        }
        m_curFrame = frame;
        if (m_repr) {
            m_repr->mesh->update_vertex_buffer(0, m_pointData.at(frame).data());
        }
    }

    void C3D::drawGUI() {
        if (c_gui) {
            if (ImGui::Begin(std::string("C3D##").c_str(), &c_gui)) {
                ImGui::PushID("c3d");
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
                        if (ImGui::Combo(_labelPrefix("Sync Event").c_str(), &m_framesel, m_eventnames)) {
                            m_zeroframe = m_events.at(m_framesel).frame;
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

    void C3D::Render() const {
        if (m_repr && m_draw) {
            m_repr->bind();
            m_repr->draw();
            m_repr->unbind();
        }
    }

    void C3D::drawClassGUI() {
        if (cppgl::Context::show_gui) {
            if (ImGui::BeginMainMenuBar()) {

                ImGui::Separator();
                ImGui::Checkbox("C3D##", &c_gui);
                ImGui::EndMainMenuBar();
            }
        }
    }

    std::ostream &operator<<(std::ostream &os, const C3D &c3d) {
        os << "[C3D] number of frames: " << c3d.getNumFrames() << " number of points: " << c3d.getNumPoints() << " capture framerate: " << c3d.getFramerate() << "\n";
        os << "\nMarkernames:\n";
        for (const std::string &n: c3d.m_markerNames) {
            os << n << "\n";
        }

        os << "\nJointnames:\n";
        for (const std::string &j: c3d.m_gaitBonesNames) {
            os << j << "\n";
        }
        os << "\n";
        std::vector<std::vector<cppgl::vec3>> points = c3d.getPointData();
        for (unsigned long int f = 0; f < points.size(); ++f) {
            os << "Frame: " << f << "\n";
            for (unsigned long int p = 0; p < points.at(f).size(); ++p) {
                os << "[ " << points.at(f).at(p).x() << ",\t" << points.at(f).at(p).y() << ",\t" << points.at(f).at(p).z() << "]\n";
            }
            os << "\n";
        }
        os << std::endl;
        return os;
    }

    bool C3D::readFile(const std::string &path) {
        try {
            m_nameIdMap.clear();
            m_markerNames.clear();
            m_gaitBonesNames.clear();
            m_ununsedNames.clear();
            m_pointData.clear();
            m_markerData.clear();
            m_gaitBoneData.clear();
            m_events.clear();
            m_eventnames.clear();
            ezc3d::c3d myc3d(path);
            m_numPoints = static_cast<int>(myc3d.header().nb3dPoints());
            m_numFrames = static_cast<int>(myc3d.header().nbFrames());
            m_framerate = static_cast<int>(myc3d.header().frameRate());
            std::vector<std::string> allnames;
            ScreenDir xdir = ScreenDir::XP;
            ScreenDir ydir = ScreenDir::YP;
            std::string unit = "m";
            auto getDir = [&](std::string &dir) {
                if (dir == "x+" || dir == "+x" || dir == "X+" || dir == "+X") {
                    return ScreenDir::XP;
                } else if (dir == "x-" || dir == "-x" || dir == "X-" || dir == "-X") {
                    return ScreenDir::XM;
                } else if (dir == "y+" || dir == "+y" || dir == "Y+" || dir == "+Y") {
                    return ScreenDir::YP;
                } else if (dir == "y-" || dir == "-y" || dir == "Y-" || dir == "-Y") {
                    return ScreenDir::YM;
                } else if (dir == "z+" || dir == "+z" || dir == "Z+" || dir == "+Z") {
                    return ScreenDir::ZP;
                } else if (dir == "z-" || dir == "-z" || dir == "Z-" || dir == "-Z") {
                    return ScreenDir::ZM;
                } else {
                    return ScreenDir::UNKN;
                }
            };

            for (auto &g: myc3d.parameters().groups()) {
                if (g.name() == "POINT") {
                    for (auto &p: g.parameters()) {
                        if (p.name() == "LABELS") {
                            allnames = p.valuesAsString();
                        } else if (p.name() == "UNITS") {
                            unit = p.valuesAsString().at(0);
                        } else if (p.name() == "X_SCREEN") {
                            std::string dir = p.valuesAsString().at(0);
                            xdir = getDir(dir);
                        } else if (p.name() == "Y_SCREEN") {
                            std::string dir = p.valuesAsString().at(0);
                            ydir = getDir(dir);
                        }
                    }
                } else if (g.name() == "EVENT") {
                    for (auto &p: g.parameters()) {
                        if (p.name() == "USED") {
                            m_events.resize(p.valuesAsInt().at(0), Event(-1, ""));
                        } else if (p.name() == "CONTEXTS") {
                            for (unsigned long int i = 0; i < m_events.size(); ++i) {
                                m_events.at(i).event += p.valuesAsString().at(i) + " ";
                            }
                        } else if (p.name() == "LABELS") {
                            for (unsigned long int i = 0; i < m_events.size(); ++i) {
                                m_events.at(i).event += p.valuesAsString().at(i);
                            }
                        } else if (p.name() == "TIMES") {
                            for (unsigned long int i = 0; i < m_events.size(); ++i) {
                                m_events.at(i).frame = static_cast<int>(m_framerate * p.valuesAsDouble().at(i * 2 + 1) - static_cast<double>(myc3d.header().firstFrame()));//TODO: Seems pretty hacky and like it would only work for our specialized case
                            }
                        }
                    }
                }
            }
            std::sort(m_events.begin(), m_events.end());
            m_zeroframe = m_events.empty() ? -1 : m_events.front().frame;
            for (auto &ev: m_events) {
                m_eventnames.push_back(ev.event);
            }

            const std::map<std::string, float> lengthToMeter = {{"mm",   0.001f},    // millimeter
                                                                {"cm",   0.01f},     // centimeter
                                                                {"dm",   0.1f},      // decimeter
                                                                {"m",    1.0f},       // meter
                                                                {"dam",  10.0f},    // dekameter
                                                                {"hm",   100.0f},    // hectometer
                                                                {"km",   1000.0f},   // kilometer
                                                                {"inch", 0.0254f}, // inch
                                                                {"foot", 0.3048f}, // foot
                                                                {"yard", 0.9144f}, // yard
                                                                {"mile", 1609.34f} // statute mile
            };

            // https://docs.vicon.com/display/Nexus215/Full+body+modeling+with+Plug-in+Gait
            // https://docs.vicon.com/download/attachments/133828966/Plug-in%20Gait%20Reference%20Guide.pdf?version=2&modificationDate=1637681079000&api=v2
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
                for (int p = 0; p < m_numPoints; ++p) {
                    const ezc3d::DataNS::Points3dNS::Point &po = myc3d.data().frames().at(f).points().point(p);
                    float xpos = static_cast<float>(po.x());
                    float ypos = static_cast<float>(po.y());
                    float zpos = static_cast<float>(po.z());
                    //Sign of the third axis is according to right handed coordinate system
                    switch (xdir) {
                        case ScreenDir::XP:
                            switch (ydir) {
                                case ScreenDir::XP:
                                case ScreenDir::XM:
                                    LOG(FATAL) << "Impossible x/y screendir combination" << std::endl;
                                    break;
                                case ScreenDir::YP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(xpos, ypos, zpos);
                                    break;
                                case ScreenDir::YM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(xpos, -ypos, -zpos);
                                    break;
                                case ScreenDir::ZP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(xpos, zpos, -ypos);
                                    break;
                                case ScreenDir::ZM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(xpos, -zpos, ypos);
                                    break;
                                default:
                                    LOG(FATAL) << "Unknown y screendir" << std::endl;
                                    break;
                            }
                            break;
                        case ScreenDir::XM:
                            switch (ydir) {
                                case ScreenDir::XP:
                                case ScreenDir::XM:
                                    LOG(FATAL) << "Impossible x/y screendir combination" << std::endl;
                                    break;
                                case ScreenDir::YP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-xpos, ypos, -zpos);
                                    break;
                                case ScreenDir::YM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-xpos, -ypos, zpos);
                                    break;
                                case ScreenDir::ZP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-xpos, zpos, ypos);
                                    break;
                                case ScreenDir::ZM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-xpos, -zpos, -ypos);
                                    break;
                                default:
                                    LOG(FATAL) << "Unknown y screendir" << std::endl;
                                    break;
                            }
                            break;
                        case ScreenDir::YP:
                            switch (ydir) {
                                case ScreenDir::XP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(ypos, xpos, -zpos);
                                    break;
                                case ScreenDir::XM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(ypos, -xpos, zpos);
                                    break;
                                case ScreenDir::YP:
                                case ScreenDir::YM:
                                    LOG(FATAL) << "Impossible x/y screendir combination" << std::endl;
                                    break;
                                case ScreenDir::ZP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(ypos, zpos, xpos);
                                    break;
                                case ScreenDir::ZM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(ypos, -zpos, -xpos);
                                    break;
                                default:
                                    LOG(FATAL) << "Unknown y screendir" << std::endl;
                                    break;
                            }
                            break;
                        case ScreenDir::YM:
                            switch (ydir) {
                                case ScreenDir::XP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-ypos, xpos, zpos);
                                    break;
                                case ScreenDir::XM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-ypos, -xpos, -zpos);
                                    break;
                                case ScreenDir::YP:
                                case ScreenDir::YM:
                                    LOG(FATAL) << "Impossible x/y screendir combination" << std::endl;
                                    break;
                                case ScreenDir::ZP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-ypos, zpos, -xpos);
                                    break;
                                case ScreenDir::ZM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-ypos, -zpos, xpos);
                                    break;
                                default:
                                    LOG(FATAL) << "Unknown y screendir" << std::endl;
                                    break;
                            }
                            break;
                        case ScreenDir::ZP:
                            switch (ydir) {
                                case ScreenDir::XP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(zpos, xpos, ypos);
                                    break;
                                case ScreenDir::XM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(zpos, -xpos, -ypos);
                                    break;
                                case ScreenDir::YP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(zpos, ypos, -xpos);
                                    break;
                                case ScreenDir::YM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(zpos, -ypos, xpos);
                                    break;
                                case ScreenDir::ZP:
                                case ScreenDir::ZM:
                                    LOG(FATAL) << "Impossible x/y screendir combination" << std::endl;
                                    break;
                                default:
                                    LOG(FATAL) << "Unknown y screendir" << std::endl;
                                    break;
                            }
                            break;
                        case ScreenDir::ZM:
                            switch (ydir) {
                                case ScreenDir::XP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-zpos, xpos, -ypos);
                                    break;
                                case ScreenDir::XM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-zpos, -xpos, ypos);
                                    break;
                                case ScreenDir::YP:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-zpos, ypos, xpos);
                                    break;
                                case ScreenDir::YM:
                                    m_pointData.at(f).at(p) = cppgl::vec3(-zpos, -ypos, -xpos);
                                    break;
                                case ScreenDir::ZP:
                                case ScreenDir::ZM:
                                    LOG(FATAL) << "Impossible x/y screendir combination" << std::endl;
                                    break;
                                default:
                                    LOG(FATAL) << "Unknown y screendir" << std::endl;
                                    break;
                            }
                            break;
                        default:
                            LOG(FATAL) << "Unknown x screendir" << std::endl;
                            break;
                    }
                    m_pointData.at(f).at(p) *= lengthToMeter.at(unit); // unit conversion to meter
                }
            }

            //Hopefully remove all nans
            for (int cf = 0; cf < m_numFrames; ++cf) {
                std::vector<cppgl::vec3> &ps = m_pointData.at(cf);
                for (int cp = 0; cp < m_numPoints; ++cp) {
                    if (std::isnan(ps.at(cp).x())) {
                        if (cf == 0) {
                            int nnan = 1;
                            for (; nnan < m_numFrames; ++nnan) {
                                if (!std::isnan(m_pointData.at(nnan).at(cp).x())) {
                                    break;
                                }
                            }
                            for (int fix = 0; fix < nnan; ++fix) {
                                m_pointData.at(fix).at(cp) = m_pointData.at(nnan).at(cp);//Just use first not nan
                            }
                        } else if (!std::isnan(m_pointData.at(cf - 1).at(cp).x())) {
                            //find next not nan
                            int nnan = cf + 1;
                            for (; nnan < m_numFrames; ++nnan) {
                                if (!std::isnan(m_pointData.at(nnan).at(cp).x())) {
                                    break;
                                }
                            }
                            if (nnan >= m_numFrames) {
                                ps.at(cp) = m_pointData.at(cf - 1).at(cp);//copy last known valid point
                            } else {
                                int range = nnan - (cf - 1);
                                for (int fix = cf; fix < nnan; ++fix) {
                                    float fixes = fix - cf + 1;
                                    m_pointData.at(fix).at(cp) = (1 - fixes / range) * m_pointData.at(cf - 1).at(cp) + fixes / range * m_pointData.at(nnan).at(cp);//Just linearly interpolate if neighbouring frame have data
                                }
                            }
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