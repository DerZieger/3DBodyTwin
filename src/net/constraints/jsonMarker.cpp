#ifdef HAS_DL_MODULE

#include "net/constraints/c3dMarker.h"
#include "gui.h"
#include "net/constraints/jsonMarker.h"

#include <utility>


TWIN_NAMESPACE_BEGIN

    JsonMarkerConstraintImpl::JsonMarkerConstraintImpl() : MarkerConstraintBaseImpl() {}

    JsonMarkerConstraintImpl::JsonMarkerConstraintImpl(std::shared_ptr<JMarker> mjs) : MarkerConstraintBaseImpl(), m_json(std::move(mjs)) {
        UpdateMarkerData();
    }

    JsonMarkerConstraintImpl::JsonMarkerConstraintImpl(const std::string &cname) : MarkerConstraintBaseImpl(cname) {}

    JsonMarkerConstraintImpl::JsonMarkerConstraintImpl(const std::string &cname, std::shared_ptr<JMarker> mjs) : MarkerConstraintBaseImpl(cname), m_json(std::move(mjs)) {
        UpdateMarkerData();
    }

    void JsonMarkerConstraintImpl::Serialize(nlohmann::json &json) const {
        nlohmann::json &json2 = json[name];
        json2["type"] = "JSONMarker";
        json2["Markers"] = m_markers;
    }

    void JsonMarkerConstraintImpl::Deserialize(const nlohmann::json &json, std::string cname) {
        name = cname;
        m_markers = json[cname]["Markers"];
        if (m_json) {
            UpdateMarkerData();
        }
    }

    bool JsonMarkerConstraintImpl::isReferenceSet() const {
        return m_json ? true : false;
    }

    void JsonMarkerConstraintImpl::referenceGUI(std::vector<std::shared_ptr<JMarker>> &vmjs) {
        if (m_json) {
            return;
        }
        std::vector<std::string> c3dnames(vmjs.size());
#pragma omp parallel for default(none) shared(c3dnames)
        for (unsigned long int i = 0; i < c3dnames.size(); ++i) {
            c3dnames.at(i) = ("ID " + std::to_string(i));
        }
        static int c3dselect = 0;
        ImGui::Combo(_labelPrefix("Select the C3D file:").c_str(), &c3dselect, c3dnames);
        if (ImGui::Button("Select") && !vmjs.empty()) {
            m_json = vmjs.at(c3dselect);
            UpdateMarkerData();
        }
    }

    void JsonMarkerConstraintImpl::drawGUIEdit(BVH &bvh, const cppgl::Drawelement &mesh, const cppgl::Drawelement &mesh_shaped, const Ray &ray, cppgl::vec2 relative_pixel, bool pressed) {
        if (m_json) {
            MarkerConstraintBaseImpl::drawGUIEdit(bvh, mesh, mesh_shaped, ray, relative_pixel, pressed);
        }
    }

    void JsonMarkerConstraintImpl::Finalize() {
        if (!m_torch_constraint) {
            m_torch_constraint = TorchMarkerConstraint(name + "_Marker_" + std::to_string(m_id) + "_torch");
        }
        if (m_json) {
            UpdateMarkerData();
        } else {
            LOG(ERROR) << "C3D reference isn't set before finalizing" << std::endl;
        }
    }

    int JsonMarkerConstraintImpl::getZeroFrame() const {
        if (m_json) {
            return m_json->getSyncFrame();
        }
        return -1;
    }

    int JsonMarkerConstraintImpl::getFramerate() const {
        if (m_json) {
            return m_json->getFramerate();
        }
        return -1;
    }

    TorchConstraint JsonMarkerConstraintImpl::PrepareOptimImpl(ConstraintArgument &ca) {
        std::vector<float> positions;
        std::vector<long> facesm;
        std::vector<float> barycenter;
        std::vector<float> normal;
        const std::map<std::string, cppgl::vec3> &marker_map = m_json->getFrameMarkers(m_frameid);
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

    void JsonMarkerConstraintImpl::UpdateMarkerData() {
        const std::map<std::string, cppgl::vec3> &marker_map = m_json->getFrameMarkers(m_frameid);
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


TWIN_NAMESPACE_END

#endif