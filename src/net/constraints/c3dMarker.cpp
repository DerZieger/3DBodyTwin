#ifdef HAS_DL_MODULE

#include "net/constraints/c3dMarker.h"

#include <utility>
#include "gui.h"

TWIN_NAMESPACE_BEGIN

    C3DMarkerConstraintImpl::C3DMarkerConstraintImpl() : MarkerConstraintBaseImpl() {}

    C3DMarkerConstraintImpl::C3DMarkerConstraintImpl(std::shared_ptr<C3D> c3d) : MarkerConstraintBaseImpl(), m_c3d(std::move(c3d)) {
        UpdateMarkerData();
    }

    C3DMarkerConstraintImpl::C3DMarkerConstraintImpl(const std::string &cname) : MarkerConstraintBaseImpl(cname) {}

    C3DMarkerConstraintImpl::C3DMarkerConstraintImpl(const std::string &cname, std::shared_ptr<C3D> c3d) : MarkerConstraintBaseImpl(cname), m_c3d(std::move(c3d)) {
        UpdateMarkerData();
    }

    void C3DMarkerConstraintImpl::Serialize(nlohmann::json &json) const {
        nlohmann::json &json2 = json[name];
        json2["type"] = "C3DMarker";
        json2["Markers"] = m_markers;
    }


    void C3DMarkerConstraintImpl::Deserialize(const nlohmann::json &json, std::string cname) {
        name = cname;
        m_markers = json[cname]["Markers"];
        if (m_c3d) {
            UpdateMarkerData();
        }
    }

    TorchConstraint C3DMarkerConstraintImpl::PrepareOptimImpl(ConstraintArgument &ca) {
        std::vector<float> positions;
        std::vector<long> facesm;
        std::vector<float> barycenter;
        std::vector<float> normal;
        const std::map<std::string, cppgl::vec3> &marker_map = m_c3d->getFrameMarkers(m_frameid);
        for (std::pair<const std::string, MarkerData> &m: m_markers) {
            if (m.second.m_use) {
                const cppgl::vec3 &pos = marker_map.at(m.first);
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

    void C3DMarkerConstraintImpl::UpdateMarkerData() {
        const std::map<std::string, cppgl::vec3> &marker_map = m_c3d->getFrameMarkers(m_frameid);
        for (std::pair<const std::string, MarkerData> &m: m_markers) {
            if (marker_map.count(m.first)) {
                m.second.m_reference = true;
                m.second.m_use = true;
                m.second.m_target = marker_map.at(m.first);
            } else {
                m.second.m_reference = false;
                m.second.m_use = false;
            }
        }
        m_unused_markers.clear();
        for (const std::pair<const std::string, cppgl::vec3> &m: marker_map) {
            if (!m_markers.count(m.first)) {
                m_unused_markers.push_back(m.first);
            }
        }
    }

    bool C3DMarkerConstraintImpl::isReferenceSet() const {
        if (m_c3d) {
            return true;
        }
        return false;
    }

    void C3DMarkerConstraintImpl::referenceGUI(std::vector<std::shared_ptr<C3D>> &vC3D) {
        if (m_c3d) {
            return;
        }
        std::vector<std::string> c3dnames(vC3D.size());
#pragma omp parallel for default(none) shared(c3dnames)
        for (unsigned long int i = 0; i < c3dnames.size(); ++i) {
            c3dnames.at(i) = ("ID " + std::to_string(i));
        }
        static int c3dselect = 0;
        ImGui::Combo(_labelPrefix("Select the C3D file:").c_str(), &c3dselect, c3dnames);
        if (ImGui::Button("Select") && !vC3D.empty()) {
            m_c3d = vC3D.at(c3dselect);
            UpdateMarkerData();
        }
    }

    void C3DMarkerConstraintImpl::drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const Ray &ray, cppgl::vec2 relative_pixel, bool pressed) {
        if (m_c3d) {
            MarkerConstraintBaseImpl::drawGUIEdit(bvh, mesh, mesh_shaped, ray, relative_pixel, pressed);
        }
    }

    void C3DMarkerConstraintImpl::Finalize() {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchMarkerConstraint(name + "_Marker_" + std::to_string(m_id) + "_torch");
        }
        if (m_c3d) {
            UpdateMarkerData();
        } else {
            LOG(ERROR) << "C3D reference isn't set before finalizing" << std::endl;
        }
    }

    int C3DMarkerConstraintImpl::getZeroFrame() const {
        return m_c3d->getSyncFrame();
    }

    int C3DMarkerConstraintImpl::getFramerate() const {
        if (m_c3d) {
            return m_c3d->getFramerate();
        }
        return -1;
    }
TWIN_NAMESPACE_END


#endif