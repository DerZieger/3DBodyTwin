#ifdef HAS_DL_MODULE

#include "net/constraints/osimMarker.h"

#include <utility>
#include "gui.h"

TWIN_NAMESPACE_BEGIN
    OsimMarkerConstraintImpl::OsimMarkerConstraintImpl() : MarkerConstraintBaseImpl() {}

    OsimMarkerConstraintImpl::OsimMarkerConstraintImpl(std::shared_ptr<Osim> osim) : MarkerConstraintBaseImpl(), m_osim(std::move(osim)) {
        UpdateMarkerData();//TODO:Paul Fragen
    }

    OsimMarkerConstraintImpl::OsimMarkerConstraintImpl(const std::string &cname) : MarkerConstraintBaseImpl(cname) {}

    OsimMarkerConstraintImpl::OsimMarkerConstraintImpl(const std::string &cname, std::shared_ptr<Osim> osim) : MarkerConstraintBaseImpl(cname), m_osim(std::move(osim)) {
        UpdateMarkerData();
    }

    void OsimMarkerConstraintImpl::Serialize(nlohmann::json &json) const {
        nlohmann::json &json2 = json[name];
        json2["type"] = "OsimMarker";
        json2["Markers"] = m_markers;
    }


    void OsimMarkerConstraintImpl::Deserialize(const nlohmann::json &json, std::string cname) {
        name = cname;
        m_markers = json[cname]["Markers"];
        if (m_osim) {
            UpdateMarkerData();
        }
    }

    TorchConstraint OsimMarkerConstraintImpl::PrepareOptimImpl(ConstraintArgument &ca) {
        std::vector<float> positions;
        std::vector<long> facesm;
        std::vector<float> barycenter;
        std::vector<float> normal;
        BodyConfiguration &config = m_osim->getBodyConfig(m_frameid);
        for (std::pair<const std::string, MarkerData> &m: m_markers) {
            if (m.second.m_use) {
                cppgl::vec3 &pos = config.marker_map[m.first];
                positions.push_back(pos[0]);
                positions.push_back(pos[1]);
                positions.push_back(pos[2]);
                m.second.m_target = pos;
                facesm.push_back(m.second.m_fid);
                barycenter.push_back(m.second.m_baryCoords[0]);
                barycenter.push_back(m.second.m_baryCoords[1]);
                barycenter.push_back(m.second.m_baryCoords[2]);
                normal.push_back(m.second.m_dis);

            }
        }
        std::lock_guard<std::mutex> guard(m_torch_constraint->m_lock);
        m_torch_constraint->m_positions = torch::from_blob(positions.data(), {static_cast<int>(facesm.size()), 3}, torch::kFloat32).to(ca.device);
        m_torch_constraint->m_faces = torch::from_blob(facesm.data(), {static_cast<int>(facesm.size())}, torch::kInt64).to(ca.device);
        m_torch_constraint->m_barycenter = torch::from_blob(barycenter.data(), {static_cast<int>(facesm.size()), 3}, torch::kFloat32).to(ca.device);
        m_torch_constraint->m_dis = torch::from_blob(normal.data(), {static_cast<int>(facesm.size())}, torch::kFloat32).to(ca.device);

        return m_torch_constraint;
    }

    void OsimMarkerConstraintImpl::UpdateMarkerData() {
        BodyConfiguration &config = m_osim->getBodyConfig(m_frameid);
        for (std::pair<const std::string, MarkerData> &m: m_markers) {
            if (config.marker_map.count(m.first)) {
                m.second.m_reference = true;
                m.second.m_use = true;
                m.second.m_target = config.marker_map[m.first];
            } else {
                m.second.m_reference = false;
                m.second.m_use = false;
            }
        }
        m_unused_markers.clear();
        for (const std::pair<const std::string, cppgl::vec3> &m: config.marker_map) {
            if (!m_markers.count(m.first)) {
                m_unused_markers.push_back(m.first);
            }
        }
    }

    bool OsimMarkerConstraintImpl::isReferenceSet() const {
        if (m_osim) {
            return true;
        }
        return false;
    }

    void OsimMarkerConstraintImpl::referenceGUI(std::vector<std::shared_ptr<Osim>> &vOsim) {
        if (m_osim) {
            return;
        }
        std::vector<std::string> osimnames(vOsim.size());
#pragma omp parallel for default(none) shared(osimnames)
        for (unsigned long int i = 0; i < osimnames.size(); ++i) {
            osimnames.at(i) = ("ID " + std::to_string(i));
        }
        static int osimselect = 0;
        ImGui::Combo(_labelPrefix("Select the Osim object:").c_str(), &osimselect, osimnames);
        if (ImGui::Button("Select") && !vOsim.empty()) {
            m_osim = vOsim.at(osimselect);
            UpdateMarkerData();
        }
    }

    void OsimMarkerConstraintImpl::drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const Ray &ray, cppgl::vec2 relative_pixel, bool pressed) {
        if (m_osim) {
            MarkerConstraintBaseImpl::drawGUIEdit(bvh, mesh, mesh_shaped, ray, relative_pixel, pressed);
        }
    }

    void OsimMarkerConstraintImpl::Finalize() {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchMarkerConstraint(name + "_Marker_" + std::to_string(m_id) + "_torch");
        }
        if (m_osim) {
            UpdateMarkerData();
        } else {
            LOG(ERROR) << "Osim reference isn't set before finalizing" << std::endl;
        }
    }

    int OsimMarkerConstraintImpl::getZeroFrame() const {
        return m_osim->getSyncFrame();
    }

    int OsimMarkerConstraintImpl::getFramerate() const {
        if (m_osim) {
            return m_osim->getFramerate();
        }
        return -1;
    }

TWIN_NAMESPACE_END


#endif