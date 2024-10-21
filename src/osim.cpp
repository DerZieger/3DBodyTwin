/*
 This file is a modified version of https://github.com/mad-lab-fau/BASH-Model/blob/main/src/osim.cpp
 We use the base structure of the file to load all necessary part of the opensim model


 License:
                                  Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.

      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      "Licensor" shall mean the copyright owner or entity authorized by
      the copyright owner that is granting the License.

      "Legal Entity" shall mean the union of the acting entity and all
      other entities that control, are controlled by, or are under common
      control with that entity. For the purposes of this definition,
      "control" means (i) the power, direct or indirect, to cause the
      direction or management of such entity, whether by contract or
      otherwise, or (ii) ownership of fifty percent (50%) or more of the
      outstanding shares, or (iii) beneficial ownership of such entity.

      "You" (or "Your") shall mean an individual or Legal Entity
      exercising permissions granted by this License.

      "Source" form shall mean the preferred form for making modifications,
      including but not limited to software source code, documentation
      source, and configuration files.

      "Object" form shall mean any form resulting from mechanical
      transformation or translation of a Source form, including but
      not limited to compiled object code, generated documentation,
      and conversions to other media types.

      "Work" shall mean the work of authorship, whether in Source or
      Object form, made available under the License, as indicated by a
      copyright notice that is included in or attached to the work
      (an example is provided in the Appendix below).

      "Derivative Works" shall mean any work, whether in Source or Object
      form, that is based on (or derived from) the Work and for which the
      editorial revisions, annotations, elaborations, or other modifications
      represent, as a whole, an original work of authorship. For the purposes
      of this License, Derivative Works shall not include works that remain
      separable from, or merely link (or bind by name) to the interfaces of,
      the Work and Derivative Works thereof.

      "Contribution" shall mean any work of authorship, including
      the original version of the Work and any modifications or additions
      to that Work or Derivative Works thereof, that is intentionally
      submitted to Licensor for inclusion in the Work by the copyright owner
      or by an individual or Legal Entity authorized to submit on behalf of
      the copyright owner. For the purposes of this definition, "submitted"
      means any form of electronic, verbal, or written communication sent
      to the Licensor or its representatives, including but not limited to
      communication on electronic mailing lists, source code control systems,
      and issue tracking systems that are managed by, or on behalf of, the
      Licensor for the purpose of discussing and improving the Work, but
      excluding communication that is conspicuously marked or otherwise
      designated in writing by the copyright owner as "Not a Contribution."

      "Contributor" shall mean Licensor and any individual or Legal Entity
      on behalf of whom a Contribution has been received by Licensor and
      subsequently incorporated within the Work.

   2. Grant of Copyright License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      copyright license to reproduce, prepare Derivative Works of,
      publicly display, publicly perform, sublicense, and distribute the
      Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      (except as stated in this section) patent license to make, have made,
      use, offer to sell, sell, import, and otherwise transfer the Work,
      where such license applies only to those patent claims licensable
      by such Contributor that are necessarily infringed by their
      Contribution(s) alone or by combination of their Contribution(s)
      with the Work to which such Contribution(s) was submitted. If You
      institute patent litigation against any entity (including a
      cross-claim or counterclaim in a lawsuit) alleging that the Work
      or a Contribution incorporated within the Work constitutes direct
      or contributory patent infringement, then any patent licenses
      granted to You under this License for that Work shall terminate
      as of the date such litigation is filed.

   4. Redistribution. You may reproduce and distribute copies of the
      Work or Derivative Works thereof in any medium, with or without
      modifications, and in Source or Object form, provided that You
      meet the following conditions:

      (a) You must give any other recipients of the Work or
          Derivative Works a copy of this License; and

      (b) You must cause any modified files to carry prominent notices
          stating that You changed the files; and

      (c) You must retain, in the Source form of any Derivative Works
          that You distribute, all copyright, patent, trademark, and
          attribution notices from the Source form of the Work,
          excluding those notices that do not pertain to any part of
          the Derivative Works; and

      (d) If the Work includes a "NOTICE" text file as part of its
          distribution, then any Derivative Works that You distribute must
          include a readable copy of the attribution notices contained
          within such NOTICE file, excluding those notices that do not
          pertain to any part of the Derivative Works, in at least one
          of the following places: within a NOTICE text file distributed
          as part of the Derivative Works; within the Source form or
          documentation, if provided along with the Derivative Works; or,
          within a display generated by the Derivative Works, if and
          wherever such third-party notices normally appear. The contents
          of the NOTICE file are for informational purposes only and
          do not modify the License. You may add Your own attribution
          notices within Derivative Works that You distribute, alongside
          or as an addendum to the NOTICE text from the Work, provided
          that such additional attribution notices cannot be construed
          as modifying the License.

      You may add Your own copyright statement to Your modifications and
      may provide additional or different license terms and conditions
      for use, reproduction, or distribution of Your modifications, or
      for any such Derivative Works as a whole, provided Your use,
      reproduction, and distribution of the Work otherwise complies with
      the conditions stated in this License.

   5. Submission of Contributions. Unless You explicitly state otherwise,
      any Contribution intentionally submitted for inclusion in the Work
      by You to the Licensor shall be under the terms and conditions of
      this License, without any additional terms or conditions.
      Notwithstanding the above, nothing herein shall supersede or modify
      the terms of any separate license agreement you may have executed
      with Licensor regarding such Contributions.

   6. Trademarks. This License does not grant permission to use the trade
      names, trademarks, service marks, or product names of the Licensor,
      except as required for reasonable and customary use in describing the
      origin of the Work and reproducing the content of the NOTICE file.

   7. Disclaimer of Warranty. Unless required by applicable law or
      agreed to in writing, Licensor provides the Work (and each
      Contributor provides its Contributions) on an "AS IS" BASIS,
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
      implied, including, without limitation, any warranties or conditions
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
      PARTICULAR PURPOSE. You are solely responsible for determining the
      appropriateness of using or redistributing the Work and assume any
      risks associated with Your exercise of permissions under this License.

   8. Limitation of Liability. In no event and under no legal theory,
      whether in tort (including negligence), contract, or otherwise,
      unless required by applicable law (such as deliberate and grossly
      negligent acts) or agreed to in writing, shall any Contributor be
      liable to You for damages, including any direct, indirect, special,
      incidental, or consequential damages of any character arising as a
      result of this License or out of the use or inability to use the
      Work (including but not limited to damages for loss of goodwill,
      work stoppage, computer failure or malfunction, or any and all
      other commercial damages or losses), even if such Contributor
      has been advised of the possibility of such damages.

   9. Accepting Warranty or Additional Liability. While redistributing
      the Work or Derivative Works thereof, You may choose to offer,
      and charge a fee for, acceptance of support, warranty, indemnity,
      or other liability obligations and/or rights consistent with this
      License. However, in accepting such obligations, You may act only
      on Your own behalf and on Your sole responsibility, not on behalf
      of any other Contributor, and only if You agree to indemnify,
      defend, and hold each Contributor harmless for any liability
      incurred by, or claims asserted against, such Contributor by reason
      of your accepting any such warranty or additional liability.

   END OF TERMS AND CONDITIONS

   APPENDIX: How to apply the Apache License to your work.

      To apply the Apache License to your work, attach the following
      boilerplate notice, with the fields enclosed by brackets "[]"
      replaced with your own identifying information. (Don't include
      the brackets!)  The text should be enclosed in the appropriate
      comment syntax for the file format. We also recommend that a
      file or class name and description of purpose be included on the
      same "printed page" as the copyright notice for easier
      identification within third-party archives.

   Copyright [yyyy] [name of copyright owner]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
 */



#include "osim.h"
#include "glog//logging.h"
#include "gui.h"
#include <string>
#include "osimGeometryWrapper.h"
#include "Iir.h"

int twin::Osim::c_count = 0;
bool twin::Osim::c_gui = false;

twin::Osim::Osim(const std::string &filepath_osim, const std::string &filepath_scale, const std::string &filepath_mot, const std::string &filepath_sto) : m_path(filepath_osim), current_frame(-1), drawBones(true), drawMuscles(false), drawMarkers(false), drawJoints(false), drawLines(false), motion_frame(-1), muscleActCol(false), m_id(c_count++), m_framerate(-1), m_zeroframe(-1) {
    cppgl::gui_add_callback("osim", drawClassGUI);
    try {
        // load the Osim file
        osimModel = OpenSim::Model(filepath_osim);
        LOG(INFO) << filepath_osim << " " << filepath_mot << std::endl;

        // Initialize the system and get the state representing the state system
        state = std::ref(osimModel.initSystem());

        // load the scale factors from the .xml configuration file
        if (!filepath_scale.empty()) {
            loadScaleFactors(filepath_scale);
        }

        // load the motion file (.mot)
        if (!filepath_mot.empty()) {
            motStorage = loadStorageFile(filepath_mot);
            LOG(INFO) << motStorage.getSize() << std::endl;

            // load the muscle activation file (.sto)
            if (!filepath_sto.empty()) {
                loadMuscleActivationFile(filepath_sto);
            }
        }
        createBody();
        // add Osim gui in cppgl gui callback
        cppgl::gui_add_callback("osimgui" + std::to_string(m_id), twin::GuiCallbackWrapper<twin::Osim>, this);
    }
    catch (const std::exception &ex) {
        LOG(FATAL) << ex.what();
        throw;
    }
}

twin::Osim::~Osim() {
    cppgl::gui_remove_callback("osimgui" + std::to_string(m_id));
}

// Draw all the data that is read from the Osim model
void twin::Osim::draw() const {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);                      // for transparency
    for (const BodyRendering::BodyPartData &bpd: body_rendering.part_data) // iterate over all bodyparts and render them
    {
        if (drawLines && bpd.line_bones) {
            bpd.line_bones->bind_draw_unbind();
        }
        if (drawBones && bpd.bones) {
            bpd.bones->bind_draw_unbind();
        }
        if (drawJoints && bpd.joints) {
            bpd.joints->bind();
            bpd.joints->draw();
            bpd.joints->unbind();
        }
        if (drawMarkers && bpd.markers) {
            bpd.markers->bind();
            bpd.markers->draw();
            bpd.markers->unbind();
        }
    }
    for (const cppgl::GroupedDrawelements &mu: body_rendering.muscles) // render the muscles
    {
        glLineWidth(5);
        if (drawMuscles) {
            mu->bind_draw_unbind();
        }
        glLineWidth(1);
    }

    glDisable(GL_BLEND);
}

// Draw the gui associated with Osim
void twin::Osim::drawGUI() {
    if (c_gui) {
        if (ImGui::Begin(std::string("OpenSim##").c_str(), &c_gui)) {
            ImGui::PushID("Osim");
            if (ImGui::CollapsingHeader(std::to_string(m_id).c_str())) {
                ImGui::PushID(m_id);
                if (ImGui::SliderInt(_labelPrefix("Motion frame").c_str(), &motion_frame, -1, getNumFrames() - 1))
                    SetStateToFrame(motion_frame);
                if (ImGui::Combo(_labelPrefix("Sync Event").c_str(), &m_framesel, m_eventnames)) {
                    m_zeroframe = m_events.at(m_framesel).frame;
                }
                ImGui::InputInt(_labelPrefix("Synchronization Frame:").c_str(), &m_zeroframe);
                if (ImGui::Checkbox(_labelPrefix("Draw bones").c_str(), &drawBones))
                    SetStateToFrame(motion_frame);
                if (ImGui::Checkbox(_labelPrefix("Draw muscles").c_str(), &drawMuscles))
                    SetStateToFrame(motion_frame);
                if (drawMuscles)
                    ImGui::Checkbox(_labelPrefix("Muscle activation color").c_str(), &muscleActCol); // choose if the muscle are just arbitraryly colored oder according to activation intensity
                if (ImGui::Checkbox(_labelPrefix("Draw markers").c_str(), &drawMarkers))
                    SetStateToFrame(motion_frame);
                if (ImGui::Checkbox(_labelPrefix("Draw joints").c_str(), &drawJoints))
                    SetStateToFrame(motion_frame);
                if (ImGui::Checkbox(_labelPrefix("Draw line bones").c_str(), &drawLines))
                    SetStateToFrame(motion_frame);
                if (ImGui::CollapsingHeader("Color picker##")) {
                    if (drawBones)
                        ImGui::ColorEdit4("Bone color##", cppgl::Material::find("bones")->vec4_map["color"].data());
                    if (drawMarkers)
                        ImGui::ColorEdit4("Marker color##", cppgl::Material::find("marker")->vec4_map["color"].data());
                    if (drawJoints)
                        ImGui::ColorEdit4("Joint color##", cppgl::Material::find("joint")->vec4_map["color"].data());
                    if (drawLines)
                        ImGui::ColorEdit4("Line color##", cppgl::Material::find("line")->vec4_map["color"].data());
                }
                ImGui::PopID();
            }
            ImGui::PopID();
        }
        ImGui::End();
    }
}

void twin::Osim::drawClassGUI() {
    if (cppgl::Context::show_gui) {
        if (ImGui::BeginMainMenuBar()) {
            ImGui::Separator();
            ImGui::Checkbox("Osim##", &c_gui);
            ImGui::EndMainMenuBar();
        }
    }
}

int twin::Osim::getFramerate() const {
    return m_framerate;
}

int twin::Osim::getSyncFrame() const {
    return m_zeroframe;
}

std::vector<twin::Event> twin::Osim::getEvents() const {
    return m_events;
}

// Creates the drawelements of the body except the muscles
void twin::Osim::createDrawelements() {
    // create materials for  different body parts
    cppgl::Material boneMat("bones");
    boneMat->vec4_map["lightDir"] = cppgl::vec4(-1, -1, -1, 0).normalized();
    boneMat->vec4_map["color"] = cppgl::vec4(1, 1, 1, 1);

    if (!cppgl::Material::valid("marker")) {
        cppgl::Material markerMat("marker");
        markerMat->vec4_map["color"] = cppgl::vec4(1, 0, 0, 1);
    }
    if (!cppgl::Material::valid("joint")) {
        cppgl::Material jointMat("joint");
        jointMat->vec4_map["color"] = cppgl::vec4(0, 1, 0, 1);
    }
    if (!cppgl::Material::valid("line")) {
        cppgl::Material lineMat("line");
        lineMat->vec4_map["color"] = cppgl::vec4(0, 0, 1, 1);
    }
    if (!cppgl::Shader::valid("pointCloudUni")) {
        cppgl::Shader s3("pointCloudUni", "shader/pointCloudUni.vert", "shader/pointCloud.frag");
    }
    SimTK::State &cstate = state.value().get();
    SetStateToFrame(-1);
    osimModel.realizePosition(cstate);

    OpenSim::Array<std::string> bodyNames;
    osimModel.getBodySet().getNames(bodyNames);
    // load geometry for each body part
    for (int bodyID = 0; bodyID < bodyNames.size(); bodyID++) {
        std::string bodyName = bodyNames[bodyID];
        const OpenSim::Body &b = osimModel.getBodySet().get(bodyName);
        SimTK::Array_<SimTK::DecorativeGeometry> geometries;
        OpenSim::ModelDisplayHints hints;

        // extract the geometry from body and current state
        int numAttachedGeometry = b.getProperty_attached_geometry().size();
        if (numAttachedGeometry > 0) {
            for (int geometryID = 0; geometryID < numAttachedGeometry; geometryID++) {
                const OpenSim::Geometry &attachedGeometry = b.get_attached_geometry(geometryID);

                const OpenSim::AbstractProperty &a = attachedGeometry.getPropertyByName("mesh_file");
                std::string mesh_file = a.getValue<std::string>();
                attachedGeometry.generateDecorations(true, hints, cstate, geometries);
            }
        }
        // create drawelement from the extracted geometries
        std::vector<cppgl::Drawelement> drE;
        for (unsigned int i = 0; i != geometries.size(); ++i) {
            drE.push_back(getDrawelementfromMesh(geometries[i], cstate, bodyName + std::to_string(m_id), static_cast<int>(i)));
        }
        body_rendering.part_map[bodyName] = static_cast<int>(body_rendering.part_data.size());
        cppgl::GroupedDrawelements bone(bodyName, drE);
        BodyRendering::BodyPartData pdata;
        pdata.name = bodyName;
        pdata.bones = bone;
        pdata.model = std::move(bone);
        body_rendering.part_data.push_back(std::move(pdata)); // save the bodypart in a configuration struct to be able to access it later
    }

    const OpenSim::MarkerSet &markerSet = osimModel.getMarkerSet();
    OpenSim::Array<std::string> markerNames;
    markerSet.getNames(markerNames);

    std::map<std::string, std::vector<cppgl::vec3>> markermap;
    std::map<std::string, std::vector<cppgl::vec3>> jointmap;
    //  find bodypart for each attached marker
    for (int markerID = 0; markerID < markerNames.size(); markerID++) {
        std::string markerName = markerNames[markerID];
        if (markerName.rfind("CP_", 0) == 0) { // drop markers named "CP_*"
            if (markerName.rfind("CPM") == std::string::npos)
                continue;
        }
        const OpenSim::Marker &m = markerSet.get(markerName);
        const OpenSim::Frame &f = m.getParentFrame();

        std::string attachedBody = f.getName();
        cppgl::vec3 local = twin::toEigen(m.findLocationInFrame(cstate, f));
        VLOG(3) << markerName << " at " << attachedBody << "\n";
        VLOG(3) << (f.getTransformInGround(cstate) * m.findLocationInFrame(cstate, f)) << "\n";
        VLOG(3) << (m.getLocationInGround(cstate)) << "\n";
        markermap[attachedBody].push_back(local);
    }

    // create rigged markers
    for (const std::pair<const std::string, std::vector<cppgl::vec3>> &a: markermap) {
        if (!cppgl::GroupedDrawelements::valid(a.first)) {
            LOG(ERROR) << a.first;
            continue;
        }

        cppgl::Geometry geo(a.first + "_marker_geo_" + std::to_string(m_id), a.second);
        cppgl::Mesh meshs(a.first + "_marker_mesh_" + std::to_string(m_id), geo, cppgl::Material::find("marker"));
        meshs->primitive_type = GL_POINTS;

        BodyRendering::BodyPartData &data = body_rendering.part_data[body_rendering.part_map[a.first]];
        data.markers = cppgl::Drawelement(a.first + "_marker_" + std::to_string(m_id), cppgl::Shader::find("pointCloudUni"), meshs);
        data.markers->add_pre_draw_func("pointsize", []() {
            glPointSize(5);
        });
    }

    const OpenSim::JointSet &jointSet = osimModel.getJointSet();
    OpenSim::Array<std::string> jointNames;
    jointSet.getNames(jointNames);
    // create rigged line asset
    for (int jointID = 0; jointID < jointNames.size(); jointID++) {
        std::string jointName = jointNames[jointID];
        const OpenSim::Joint &j = jointSet.get(jointName);

        const OpenSim::Frame &childFrame = j.getChildFrame();
        const OpenSim::Frame &parentFrame = j.getParentFrame().findBaseFrame();

        std::string cname = childFrame.getName();
        std::string name = cname.substr(0, cname.size() - std::string("_offset").size());

        BodyRendering::BodyPartData &pdata = body_rendering.part_data[body_rendering.part_map[parentFrame.getName()]];
        const SimTK::Transform &globalTransformChild = childFrame.getTransformInGround(cstate);
        const SimTK::Transform &globalTransformParent = parentFrame.getTransformInGround(cstate);

        cppgl::vec3 pos = twin::toEigen(globalTransformChild.invert() * globalTransformChild.T());

        if (jointName != "ground_pelvis") {
            cppgl::vec3 v1 = twin::toEigen(globalTransformParent.invert() * globalTransformChild.T());
            cppgl::vec3 v2 = twin::toEigen(globalTransformParent.invert() * globalTransformParent.T());
            std::vector<cppgl::vec3> vline = {v1, v2};
            if (!pdata.line_bones) {
                std::vector<cppgl::Drawelement> linetmp;

                cppgl::Geometry geo(jointName + "_line_geo_" + std::to_string(m_id), vline);
                cppgl::Mesh meshs(jointName + "_line_mesh_" + std::to_string(m_id), geo, cppgl::Material::find("line"));
                meshs->primitive_type = GL_LINES;

                linetmp.emplace_back(jointName + "_line_" + std::to_string(m_id), cppgl::Shader::find("pointCloudUni"), meshs);
                linetmp.at(0)->add_pre_draw_func("linewidth", []() {
                    glLineWidth(2.f);
                });
                pdata.line_bones = cppgl::GroupedDrawelements(jointName + "_line_" + std::to_string(m_id), linetmp);
            } else {
                cppgl::Geometry geo(jointName + "_line_geo_" + std::to_string(m_id), vline);
                cppgl::Mesh meshs(jointName + "_line_mesh_" + std::to_string(m_id), geo, cppgl::Material::find("line"));
                meshs->primitive_type = GL_LINES;
                cppgl::Drawelement linetmp(jointName + "_line_" + std::to_string(m_id), cppgl::Shader::find("pointCloudUni"), meshs);
                linetmp->add_pre_draw_func("linewidth", []() {
                    glLineWidth(2.f);
                });
                pdata.line_bones->add_drawelement(linetmp);
            }
        }
        jointmap[name].push_back(pos);
    }

    // create rigged points for each joint
    for (const std::pair<const std::string, std::vector<cppgl::vec3>> &a: jointmap) {
        if (!body_rendering.part_map.count(a.first)) {
            VLOG(3) << a.first << '\n';
            continue;
        }

        cppgl::Geometry geo(a.first + "_joint_geo_" + std::to_string(m_id), a.second);
        cppgl::Mesh meshs(a.first + "_joint_mesh_" + std::to_string(m_id), geo, cppgl::Material::find("joint"));
        meshs->primitive_type = GL_POINTS;

        BodyRendering::BodyPartData &data = body_rendering.part_data[body_rendering.part_map[a.first]];
        data.joints = cppgl::Drawelement(a.first + "_joint_" + std::to_string(m_id), cppgl::Shader::find("pointCloudUni"), meshs);
        data.joints->add_pre_draw_func("pointsize", []() {
            glPointSize(5);
        });
    }
}

void twin::Osim::setStateToDefault() {
    try {
        current_frame = -1;
        SimTK::State &cstate = state.value().get();
        osimModel.setStateVariableValues(cstate, default_values);
        osimModel.assemble(cstate);
        // transform the bodypart according to the current state
        for (std::shared_ptr<twin::BodyConfiguration::BodyPart> &spBP: bconfigs.at(current_frame + 1).parts) {
            if (drawBones && body_rendering.part_data[body_rendering.part_map[spBP->name]].bones) {
                body_rendering.part_data[body_rendering.part_map[spBP->name]].bones->set_model_transform(spBP->global_transform);
            }
            if (drawLines && body_rendering.part_data[body_rendering.part_map[spBP->name]].line_bones) {
                body_rendering.part_data[body_rendering.part_map[spBP->name]].line_bones->set_model_transform(spBP->global_transform);
            }
            if (drawJoints && body_rendering.part_data[body_rendering.part_map[spBP->name]].joints) {
                body_rendering.part_data[body_rendering.part_map[spBP->name]].joints->set_model_transform(spBP->global_transform);
            }
            if (drawMarkers && body_rendering.part_data[body_rendering.part_map[spBP->name]].markers) {
                body_rendering.part_data[body_rendering.part_map[spBP->name]].markers->set_model_transform(spBP->global_transform);
            }
            if (drawMuscles) {
                for (std::pair<const std::string, twin::Osim::Muscle> &mus: muscleData.at(0)) {
                    if (cppgl::GroupedDrawelements::valid(mus.first)) {
                        float a = 0.5f * static_cast<float>(mus.second.activation);
                        if (muscleActCol) {
                            cppgl::Material::find(mus.first + "_mat")->vec4_map["color"] = cppgl::vec4(a, 0, 0, 1);
                        } else {
                            cppgl::Material::find(mus.first + "_mat")->vec4_map["color"] = mus.second.color;
                        }

                        cppgl::GroupedDrawelements gdeMuscle = cppgl::GroupedDrawelements::find(mus.first);
                        for (size_t cmuscle = 0; cmuscle < mus.second.lineSet.size() - 1; ++cmuscle) {
                            Eigen::Vector<float, 6> cvp(mus.second.lineSet.at(cmuscle).first.x(),
                                                        mus.second.lineSet.at(cmuscle).first.y(),
                                                        mus.second.lineSet.at(cmuscle).first.z(),
                                                        mus.second.lineSet.at(cmuscle + 1).first.x(),
                                                        mus.second.lineSet.at(cmuscle + 1).first.y(),
                                                        mus.second.lineSet.at(cmuscle + 1).first.z());
                            gdeMuscle->elems.at(cmuscle)->mesh->update_vertex_buffer(0,
                                                                                     cvp.data()); // change the actual vertexpositions
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception &ex) {
        LOG(FATAL) << ex.what();
        throw;
    }
}

// update the internal state of the model to a certain time frameID
void twin::Osim::SetStateToFrame(const int frameID) {
    motion_frame = frameID;
    if (frameID == -1)
        return setStateToDefault();

    try {
        SimTK::State &cstate = state.value().get();
        OpenSim::StateVector *stateVector = motStorage.getStateVector(frameID);
        OpenSim::Array<std::string> labels = motStorage.getColumnLabels();
        VLOG(3) << motStorage.getName();
        for (int i = 0; i != labels.size(); ++i) {
            VLOG(3) << "label " << labels.get(i);
        }
        cstate.setTime(stateVector->getTime());
        VLOG(2) << "SetBodies " << frameID << " " << stateVector->getTime();

        // convert string array -> vector
        OpenSim::Array<double> stateVectorData = stateVector->getData();
        SimTK::Vector stateVariableValues(stateVectorData.size());
        for (int i = 0; i < stateVectorData.size(); i++) {
            VLOG(3) << "statevar " << stateVectorData[i];
            stateVariableValues.set(i, stateVectorData[i]);
        }

        // update Osim model with state
        osimModel.setStateVariableValues(cstate, stateVariableValues);
        osimModel.assemble(cstate);

        VLOG(3) << "Bodyset";
        std::vector<std::string> inputs = osimModel.getInputNames();
        const OpenSim::BodySet &bs = osimModel.getBodySet();
        for (int i = 0; i != bs.getSize(); ++i)
            VLOG(3) << bs.get(i).getName();
        VLOG(3) << "Jointset";
        const OpenSim::JointSet &js = osimModel.getJointSet();
        for (int i = 0; i != js.getSize(); ++i)
            VLOG(3) << js.get(i).getName();
        VLOG(3) << "Coordinateset";
        const OpenSim::CoordinateSet &cs = osimModel.getCoordinateSet();
        for (int i = 0; i != cs.getSize(); ++i) {
            VLOG(3) << cs.get(i).getName();
            OpenSim::Coordinate &coordinate = cs.get(i);
            double value = coordinate.getValue(cstate);
            VLOG(3) << value;
        }

        for (std::string &s: inputs) {
            VLOG(3) << s;
        }
        osimModel.equilibrateMuscles(cstate);
        current_frame = frameID;

        // Set Transforms for the skeleton
        // transform the bodypart according to the current state
        for (std::shared_ptr<twin::BodyConfiguration::BodyPart> &spBP: bconfigs.at(current_frame + 1).parts) {
            if (drawBones && body_rendering.part_data[body_rendering.part_map[spBP->name]].bones) {
                body_rendering.part_data[body_rendering.part_map[spBP->name]].bones->set_model_transform(spBP->global_transform);
            }
            if (drawLines && body_rendering.part_data[body_rendering.part_map[spBP->name]].line_bones) {
                body_rendering.part_data[body_rendering.part_map[spBP->name]].line_bones->set_model_transform(spBP->global_transform);
            }
            if (drawJoints && body_rendering.part_data[body_rendering.part_map[spBP->name]].joints) {
                body_rendering.part_data[body_rendering.part_map[spBP->name]].joints->set_model_transform(spBP->global_transform);
            }
            if (drawMarkers && body_rendering.part_data[body_rendering.part_map[spBP->name]].markers) {
                body_rendering.part_data[body_rendering.part_map[spBP->name]].markers->set_model_transform(spBP->global_transform);
            }
            if (drawMuscles) {
                for (std::pair<const std::string, twin::Osim::Muscle> &mus: muscleData.at(current_frame + 1)) {
                    if (cppgl::GroupedDrawelements::valid(mus.first)) {

                        float a = 0.5f * static_cast<float>(mus.second.activation);
                        if (muscleActCol) {
                            cppgl::Material::find(mus.first + "_mat")->vec4_map["color"] = cppgl::vec4(a, 0, 0, 1);
                        } else {
                            cppgl::Material::find(mus.first + "_mat")->vec4_map["color"] = mus.second.color;
                        }


                        cppgl::GroupedDrawelements gdeMuscle = cppgl::GroupedDrawelements::find(mus.first);
                        for (size_t cmuscle = 0; cmuscle < mus.second.lineSet.size() - 1; ++cmuscle) {
                            Eigen::Vector<float, 6> cvp(mus.second.lineSet.at(cmuscle).first.x(),
                                                        mus.second.lineSet.at(cmuscle).first.y(),
                                                        mus.second.lineSet.at(cmuscle).first.z(),
                                                        mus.second.lineSet.at(cmuscle + 1).first.x(),
                                                        mus.second.lineSet.at(cmuscle + 1).first.y(),
                                                        mus.second.lineSet.at(cmuscle + 1).first.z());
                            gdeMuscle->elems.at(cmuscle)->mesh->update_vertex_buffer(0,
                                                                                     cvp.data()); // change the actual vertex positions
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception &ex) {
        LOG(FATAL) << ex.what();
        throw;
    }
}

// create a draw element for a given DecorativeGeometry
cppgl::Drawelement twin::getDrawelementfromMesh(const SimTK::DecorativeGeometry &g, const SimTK::State &state, const std::string &name, const int number) {
    osimGeometryWrapper osgw(state);
    g.implementGeometry(osgw);
    auto &&val = osgw.getLastMesh();
    SimTK::Transform &trans = val.trans;
    const SimTK::Vec3 &sca = val.scale;
    //const SimTK::Vec3 &col = val.color;

    std::vector<cppgl::vec3> position;
    std::vector<uint32_t> indices;

    for (int i = 0; i < val.mesh.getNumVertices(); i++) {
        SimTK::Vec3 pos = val.mesh.getVertexPosition(i);
        position.push_back(twin::toEigen(pos));
    }

    for (int i = 0; i < val.mesh.getNumFaces(); i++) {
        int numVert = val.mesh.getNumVerticesForFace(i);
        if (numVert < 3)
            continue; // Ignore it.
        if (numVert == 3) {
            indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, 0)));
            indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, 1)));
            indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, 2)));
        } else if (numVert == 4) {
            // Split it into two triangles.
            indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, 0)));
            indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, 1)));
            indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, 2)));
            indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, 2)));
            indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, 3)));
            indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, 0)));
        } else {
            // Add a vertex at the center, then split it into triangles.

            SimTK::Vec3 center(0);
            for (int j = 0; j < numVert; j++) {
                SimTK::Vec3 pos = val.mesh.getVertexPosition(val.mesh.getFaceVertex(i, j));
                center += pos;
            }
            center /= numVert;
            position.push_back(twin::toEigen(center));
            const int newIndex = static_cast<int>(position.size() - 1);
            for (int j = 0; j < numVert - 1; j++) {
                indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, j)));
                indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, j + 1)));
                indices.push_back(static_cast<uint32_t>(newIndex));
            }
            // Close the face (thanks, Alexandra Zobova).
            indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, numVert - 1)));
            indices.push_back(static_cast<uint32_t>(val.mesh.getFaceVertex(i, 0)));
            indices.push_back(static_cast<uint32_t>(newIndex));
        }
    }
    cppgl::Geometry geo(name + std::string("_") + std::to_string(number) + std::string("_geo"), position, indices);
    geo->auto_generate_normals();

    geo->scale(twin::toEigen(sca));
    geo->transform(cppgl::make_mat4(twin::toEigen(trans.R().asMat33())));
    geo->translate(twin::toEigen(trans.T()));
    geo->recompute_aabb();
    geo->update_meshes();
    cppgl::Mesh m(name + std::string("_") + std::to_string(number) + std::string("_mesh"), geo, cppgl::Material::find("bones"));
    geo->register_mesh(m);
    cppgl::Drawelement drawEle(name + std::string("_") + std::to_string(number), cppgl::Shader::find("blinnPhong"), m);

    return drawEle;
}

void filterBWZerolag(std::vector<cppgl::vec3> & val){
    auto split = [](const std::vector<cppgl::vec3> &vec) -> std::tuple<std::vector<double>, std::vector<double>, std::vector<double>> {
        std::vector<double> x, y, z;
        for (auto &v: vec) {
            x.push_back(v.x());
            y.push_back(v.y());
            z.push_back(v.z());
        }
        return {x, y, z};
    };
    auto combine = [](const std::vector<double> &x, const std::vector<double> &y, const std::vector<double> &z) {
        std::vector<cppgl::vec3> res;
        for (unsigned long int i = 0; i < x.size(); ++i) {
            res.emplace_back(x.at(i), y.at(i), z.at(i));
        }
        return res;
    };

    Iir::Butterworth::LowPass<4> bw;
    bw.setup(175.,7.);
    auto [a, b, c] = split(val);
    bw.reset();
    for(double & i : a){
        i=bw.filter(i);
    }
    std::reverse(a.begin(),a.end());
    bw.reset();
    for(double & i : a){
        i=bw.filter(i);
    }
    std::reverse(a.begin(),a.end());

    bw.reset();
    for(double & i : b){
        i=bw.filter(i);
    }
    std::reverse(b.begin(),b.end());
    bw.reset();
    for(double & i : b){
        i=bw.filter(i);
    }
    std::reverse(b.begin(),b.end());

    bw.reset();
    for(double & i : c){
        i=bw.filter(i);
    }
    std::reverse(c.begin(),c.end());
    bw.reset();
    for(double & i : c){
        i=bw.filter(i);
    }
    std::reverse(c.begin(),c.end());

    val=combine(a,b,c);
}

// create bodyconfigs for all timestamps
void twin::Osim::createBody() {
    default_values = osimModel.getStateVariableValues(state.value().get());
    bconfigs.clear();
    body_rendering = BodyRendering();
    const int N = getNumFrames();

    bconfigs.resize(N + 1);
    createDrawelements();
    for (int i = -1; i < N; ++i) {
        getMuscles(i);
        SetStateToFrame(i);
        createConfigForCurrentState();
    }


    std::vector<cppgl::vec3> prh, prt, plh, plt;//position left/right heel/toe
    for (int i = 0; i < N; ++i) {
        std::vector<std::shared_ptr<twin::BodyConfiguration::BodyPart>> &spBP1 = bconfigs.at(i + 1).parts;
        for (unsigned long int it = 0; it < spBP1.size(); ++it) {
            cppgl::vec3 vel = spBP1.at(it)->global_transform.block<3, 1>(0, 3);
            if (spBP1.at(it)->name == "calcn_r") {
                prh.push_back(vel);
            } else if (spBP1.at(it)->name == "calcn_l") {
                plh.push_back(vel);
            } else if (spBP1.at(it)->name == "toes_r") {
                prt.push_back(vel);
            } else if (spBP1.at(it)->name == "toes_l") {
                plt.push_back(vel);
            }
        }
    }
    filterBWZerolag(prh);
    filterBWZerolag(prt);
    filterBWZerolag(plh);
    filterBWZerolag(plt);

    std::vector<float> vrh, vrt, vlh, vlt;//velocity left/right heel/toe
    //Find events
    for (unsigned long int i = 0; i < prh.size() - 1; ++i) {
        cppgl::vec3 pos1 = prh.at(i);
        cppgl::vec3 pos2 = prh.at(i + 1);
        float vel = (pos1 - pos2).norm() * static_cast<float>(m_framerate);
        vrh.push_back(vel);
    }
    for (unsigned long int i = 0; i < prt.size() - 1; ++i) {
        cppgl::vec3 pos1 = prt.at(i);
        cppgl::vec3 pos2 = prt.at(i + 1);
        float vel = (pos1 - pos2).norm() * static_cast<float>(m_framerate);
        vrt.push_back(vel);
    }
    for (unsigned long int i = 0; i < plh.size() - 1; ++i) {
        cppgl::vec3 pos1 = plh.at(i);
        cppgl::vec3 pos2 = plh.at(i + 1);
        float vel = (pos1 - pos2).norm() * static_cast<float>(m_framerate);
        vlh.push_back(vel);
    }
    for (unsigned long int i = 0; i < plt.size() - 1; ++i) {
        cppgl::vec3 pos1 = plt.at(i);
        cppgl::vec3 pos2 = plt.at(i + 1);
        float vel = (pos1 - pos2).norm() * static_cast<float>(m_framerate);
        vlt.push_back(vel);
    }

    std::vector<std::vector<EventType>> posEve(N - 1);//Possible event
    bool lp = false;//Peak for left heel previous
    bool rp = false;//Peak for right heel previous
    //Auto event detection detects all events, but doesn't remove all invalid events sadly
    for (int i = 0; i < N - 1; ++i) {
        std::vector<EventType> &curv = posEve.at(i);
        if (vrh.at(i) < 0.3) {
            curv.push_back(EventType::RHS);
        }
        if (vlh.at(i) < 0.3) {
            curv.push_back(EventType::LHS);
        }
        if (vrt.at(i) < 0.3) {
            curv.push_back(EventType::RTS);
        }
        if (vlt.at(i) < 0.3) {
            curv.push_back(EventType::LTS);
        }
        if (i != 0 && i != N - 2 && vlh.at(i) > vlh.at(i - 1) && vlh.at(i) > vlh.at(i + 1)) {
            lp = true;
        }
        if (i != 0 && vlt.at(i - 1) < 1 && vlt.at(i) > 1 && lp) {
            curv.push_back(EventType::LTO);
            lp = false;
        }
        if (i != 0 && i != N - 2 && vrh.at(i) > vrh.at(i - 1) && vrh.at(i) > vrh.at(i + 1)) {
            rp = true;
        }
        if (i != 0 && vrt.at(i - 1) < 1 && vrt.at(i) > 1 && rp) {
            curv.push_back(EventType::RTO);
            rp = false;
        }
    }

    auto contains = [](std::vector<EventType> vec, EventType ele) -> bool {
        return std::find(vec.begin(), vec.end(), ele) != vec.end();
    };
    const std::map<EventType, std::string> names = {{EventType::ERROR, "Error"},
                                                    {EventType::LTO,   "left toe lift off"},
                                                    {EventType::RTO,   "right toe lift off"},
                                                    {EventType::RHS,   "right heel strike"},
                                                    {EventType::LHS,   "left heel strike"},
                                                    {EventType::LTS,   "left toe strike"},
                                                    {EventType::RTS,   "right toe strike"}};
    for (int i = 0; i < static_cast<int>(posEve.size()); ++i) {
        if (i == 0 && contains(posEve.at(i), EventType::RHS)) {
            m_events.emplace_back(i, names.at(EventType::RHS));
        } else if (contains(posEve.at(i), EventType::RHS) && !contains(posEve.at(i - 1), EventType::RHS)) {
            m_events.emplace_back(i, names.at(EventType::RHS));
        }
        if (i == 0 && contains(posEve.at(i), EventType::LHS)) {
            m_events.emplace_back(i, names.at(EventType::LHS));
        } else if (contains(posEve.at(i), EventType::LHS) && !contains(posEve.at(i - 1), EventType::LHS)) {
            m_events.emplace_back(i, names.at(EventType::LHS));
        }
        if (i == 0 && contains(posEve.at(i), EventType::RTS)) {
            m_events.emplace_back(i, names.at(EventType::RTS));
        } else if (contains(posEve.at(i), EventType::RTS) && !contains(posEve.at(i - 1), EventType::RTS)) {
            m_events.emplace_back(i, names.at(EventType::RTS));
        }
        if (i == 0 && contains(posEve.at(i), EventType::LTS)) {
            m_events.emplace_back(i, names.at(EventType::LTS));
        } else if (contains(posEve.at(i), EventType::LTS) && !contains(posEve.at(i - 1), EventType::LTS)) {
            m_events.emplace_back(i, names.at(EventType::LTS));
        }
        if (i == 0 && contains(posEve.at(i), EventType::LTO)) {
            m_events.emplace_back(i, names.at(EventType::LTO));
        } else if (contains(posEve.at(i), EventType::LTO) && !contains(posEve.at(i - 1), EventType::LTO)) {
            m_events.emplace_back(i, names.at(EventType::LTO));
        }
        if (i == 0 && contains(posEve.at(i), EventType::RTO)) {
            m_events.emplace_back(i, names.at(EventType::RTO));
        } else if (contains(posEve.at(i), EventType::RTO) && !contains(posEve.at(i - 1), EventType::RTO)) {
            m_events.emplace_back(i, names.at(EventType::RTO));
        }
    }


    std::sort(m_events.begin(), m_events.end());
    m_zeroframe = m_events.empty() ? -1 : m_events.front().frame;
    for (auto &ev: m_events) {
        m_eventnames.push_back(ev.event);
    }
    SetStateToFrame(-1);
}

// How many frames are in the motionfile
int twin::Osim::getNumFrames() {
    return motStorage.getSize();
}

// create a bodyconfig for current state
void twin::Osim::createConfigForCurrentState() {
    twin::BodyConfiguration bconfig;
    auto &cstate = state.value().get();
    osimModel.realizePosition(cstate);
    VLOG(4) << "CREATING NEW CONFIG FOR FRAME " << current_frame << std::endl;

    OpenSim::Array<std::string> bodyNames;
    osimModel.getBodySet().getNames(bodyNames);

    const OpenSim::JointSet &jointSet = osimModel.getJointSet();
    OpenSim::Array<std::string> jointNames;
    jointSet.getNames(jointNames);

    const OpenSim::MarkerSet &markerSet = osimModel.getMarkerSet();
    OpenSim::Array<std::string> markerNames;
    markerSet.getNames(markerNames);

    // create transformation for each part
    VLOG(4) << "CREATECONFIG\n";
    for (int bodyID = 0; bodyID < bodyNames.size(); bodyID++) {
        std::string bodyName = bodyNames[bodyID];
        const OpenSim::Body &b = osimModel.getBodySet().get(bodyName);
        const SimTK::Transform &globalTransform = b.getTransformInGround(cstate);
        VLOG(4) << bodyName << std::endl;
        VLOG(4) << twin::toEigen(globalTransform) << std::endl;

        std::shared_ptr<BodyConfiguration::BodyPart> bpart = std::make_shared<BodyConfiguration::BodyPart>();
        bpart->name = bodyName;
        bpart->global_transform = twin::toEigen(globalTransform);
        bconfig.parts.push_back(bpart);
        bconfig.part_map[bodyName] = bpart;
    }

    // for each joint, find parent and child in body parts plus transformation between
    for (int jointID = 0; jointID < jointNames.size(); jointID++) {
        std::string jointName = jointNames[jointID];
        const OpenSim::Joint &j = jointSet.get(jointName);

        const OpenSim::Frame &childFrame = j.getChildFrame();
        const OpenSim::Frame &parentFrame = j.getParentFrame().findBaseFrame();

        BodyConfiguration::Joint joint;
        joint.name = jointName;
        joint.parent = bconfig.part_map[parentFrame.getName()];
        auto cname = childFrame.getName();
        auto name = cname.substr(0, cname.size() - std::string("_offset").size());
        joint.child = bconfig.part_map[name];

        const SimTK::Transform &localTransform = childFrame.findTransformBetween(cstate, parentFrame);
        if (auto ptr = joint.child.lock()) {
            ptr->local_transform = twin::toEigen(localTransform.toMat44());
            ptr->parent = joint;
        }
    }
    // for each marker, find location
    for (int markerID = 0; markerID < markerNames.size(); markerID++) {
        std::string markerName = markerNames[markerID];
        if (markerName.rfind("CP_", 0) == 0) { // drop markers named "CP_*"
            if (markerName.rfind("CPM") == std::string::npos)
                continue;
        }
        const OpenSim::Marker &m = markerSet.get(markerName);

        cppgl::vec3 global = twin::toEigen(m.getLocationInGround(cstate));
        bconfig.marker_map[markerName] = global;
    }

    bconfigs.at(current_frame + 1) = std::move(bconfig);
}

void twin::Osim::loadScaleFactors(const std::string &filepath) {
    try {
        OpenSim::ScaleSet scaleSet;
        SimTK::State &currentState = osimModel.initSystem();
        osimModel.getMultibodySystem().realize(currentState, SimTK::Stage::Position);

        // load scaleSet from file
        scaleSet = OpenSim::ScaleSet(filepath);

        // fill scaleFactors vector
        for (int i = 0; i < scaleSet.getSize(); i++) {
            scaleFactors[scaleSet[i].getSegmentName()] = scaleSet[i].getScaleFactors();
        }

        // apply scaling to input model?
        // if (false) {
        osimModel.scale(currentState, scaleSet, false);
        LOG(INFO) << "Scaled the input model on given scale factors." << std::endl;
        //}
    }
    catch (const std::exception &ex) {
        LOG(FATAL) << ex.what();
        throw;
    }
}

// load the muscle activation data from a .sto file
void twin::Osim::loadMuscleActivationFile(const std::string &filepath) {
    try {
        // load .sto file
        OpenSim::TimeSeriesTable stoTable(filepath);

        // number of time states has to match the loaded .mot file
        if (stoTable.getNumRows() != static_cast<size_t>(motStorage.getSize())) {
            LOG(FATAL) << "Number of time states has to match in file: " << filepath;
            throw;
        }
        OpenSim::StateVector *stateVector1 = motStorage.getStateVector(0);
        OpenSim::StateVector *stateVector2 = motStorage.getStateVector(static_cast<int>(stoTable.getNumRows()) - 1);
        m_framerate = static_cast<int>(std::round((static_cast<int>(stoTable.getNumRows()) - 1) / (stateVector2->getTime() - stateVector1->getTime())) + 1e-6);//Calculate Framerate for synchronization between datasets

        muscleActivations.resize(stoTable.getNumRows());
        for (size_t frameID = 0; frameID < stoTable.getNumRows(); frameID++) {
            double time = stoTable.getIndependentColumn().at(frameID);

            // match time
            if (time != motStorage.getStateVector(static_cast<int>(frameID))->getTime()) {
                LOG(FATAL) << "Time state values do not match in file: " << filepath;
                throw;
            }

            // iterate through muscles
            OpenSim::Array<std::string> muscleNames;
            osimModel.getMuscles().getNames(muscleNames);

            for (int muscleID = 0; muscleID < muscleNames.size(); muscleID++) {
                std::string muscleName = muscleNames[muscleID];

                const std::string &muscleName_sto = stoTable.getColumnLabel(muscleID);
                if (muscleName != muscleName_sto) {
                    LOG(FATAL) << "Muscle name '" << muscleName << "' does not match '" << muscleName_sto << "' in " << filepath;
                    throw;
                }

                double muscleActivation = stoTable.getRowAtIndex(frameID)[muscleID];

                muscleActivations.at(frameID)[muscleName] = muscleActivation;
            }
        }
    }
    catch (const std::exception &ex) {
        LOG(FATAL) << ex.what();
        throw;
    }
}

// load OSIM storage format
OpenSim::Storage twin::Osim::loadStorageFile(const std::string &filepath) {
    OpenSim::Storage storage;

    // load the storage file
    OpenSim::Storage storageFromFile = OpenSim::Storage(filepath);

    // if data is in degrees -> convert to radians
    if (storageFromFile.isInDegrees()) {
        osimModel.getSimbodyEngine().convertDegreesToRadians(storageFromFile);
        VLOG(4) << "Converted motion data from degrees to radians." << std::endl;
    }

    // link motion data to the model and receive a new reorderd storage that matches the model internal states
    osimModel.formStateStorage(storageFromFile, storage, false);

    // per time states
    VLOG(4) << "Number of time states: " << storage.getSize() << std::endl;

    return storage;
}

// get muscles
std::map<std::string, twin::Osim::Muscle> &twin::Osim::loadMuscleData(int input_frameID) {
    // sample colors
    const cppgl::vec3 colors[33] = {{201, 255, 234},
                                    {237, 67,  0},
                                    {1,   64,  182},
                                    {218, 232, 0},
                                    {222, 115, 255},
                                    {0,   171, 10},
                                    {142, 0,   122},
                                    {205, 255, 100},
                                    {0,   46,  129},
                                    {215, 194, 0},
                                    {145, 136, 255},
                                    {118, 136, 0},
                                    {204, 0,   118},
                                    {169, 255, 165},
                                    {214, 0,   56},
                                    {143, 255, 244},
                                    {255, 73,  101},
                                    {1,   118, 96},
                                    {255, 142, 59},
                                    {9,   0,   40},
                                    {255, 181, 66},
                                    {0,   90,  121},
                                    {127, 0,   12},
                                    {246, 222, 255},
                                    {6,   0,   1},
                                    {255, 141, 221},
                                    {84,  32,  0},
                                    {255, 156, 205},
                                    {0,   79,  40},
                                    {255, 149, 162},
                                    {44,  62,  0},
                                    {255, 207, 152},
                                    {25,  30,  20}};
    int frameID = input_frameID + 1;
    for (int i = static_cast<int>(muscleData.size()); i <= frameID; ++i) {
        SimTK::State &cstate = state.value().get();
        std::map<std::string, twin::Osim::Muscle> cmuscleData;
        SetStateToFrame(i - 1);

        try {
            OpenSim::Array<std::string> muscleNames;
            osimModel.getMuscles().getNames(muscleNames);
            // iterate through all muscles
            for (int muscleID = 0; muscleID < muscleNames.size(); muscleID++) {
                std::string muscleName = muscleNames[muscleID];
                const OpenSim::Muscle &m = osimModel.getMuscles().get(muscleName);

                twin::Osim::Muscle muscle;
                muscle.name = muscleName;
                muscle.activation = 0.0;
                muscle.maxForce = m.getMaxIsometricForce();
                muscle.color = cppgl::make_vec4(colors[muscleID % 33] / 255, 1);
                // muscle line segments
                for (int musclepath = 0; musclepath < m.getGeometryPath().getPathPointSet().getSize(); musclepath++) {
                    const SimTK::Vec3 &p = m.getGeometryPath().getPathPointSet().get(musclepath).getLocationInGround(cstate);
                    std::string parentName = m.getGeometryPath().getPathPointSet().get(musclepath).getParentFrame().getName();
                    muscle.lineSet.emplace_back(twin::toEigen(p), parentName);
                }
                cmuscleData[muscleName] = muscle;

                if (!cppgl::GroupedDrawelements::valid(muscleName)) // create drawelement for muscles
                {
                    cppgl::Material tmpmat(muscleName + "_mat");
                    tmpmat->vec4_map["color"] = muscle.color;
                    std::vector<cppgl::Drawelement> tmpvec;
                    for (int ls = 0; ls < static_cast<int>(muscle.lineSet.size()) - 1; ++ls) {
                        std::vector<cppgl::vec3> vline = {muscle.lineSet.at(ls).first, muscle.lineSet.at(ls + 1).first};
                        cppgl::Geometry geo(muscleName + "_line_" + std::to_string(ls) + "_geo_" + std::to_string(m_id), vline);
                        cppgl::Mesh meshs(muscleName + "_line_" + std::to_string(ls) + "_mesh_" + std::to_string(m_id), geo, tmpmat, GL_DYNAMIC_DRAW);
                        meshs->primitive_type = GL_LINES;

                        cppgl::Line me(muscleName + "_line_" + std::to_string(ls) + "_" + std::to_string(m_id), muscle.lineSet.at(ls).first, muscle.lineSet.at(ls + 1).first, tmpmat, GL_DYNAMIC_DRAW);
                        cppgl::Drawelement tmpDE(muscleName + "_de_" + std::to_string(ls) + "_" + std::to_string(m_id), cppgl::Shader::find("pointCloudUni"), meshs);
                        tmpvec.push_back(tmpDE);
                    }
                    cppgl::GroupedDrawelements gde(muscleName, tmpvec);
                    body_rendering.muscles.push_back(gde);
                }
            }
            muscleData.push_back(std::move(cmuscleData));
        }
        catch (const std::exception &ex) {
            LOG(FATAL) << ex.what();
            throw;
        }
    }
    SetStateToFrame(current_frame);
    return muscleData.at(frameID);
}

// get muscles - for a certain frame
std::map<std::string, twin::Osim::Muscle> twin::Osim::getMuscles(int frameID) {
    // store the manually loaded muscle activations
    std::map<std::string, Muscle> &cmuscleData = loadMuscleData(frameID);
    for (std::pair<const std::string, twin::Osim::Muscle> &m: cmuscleData) {
        Muscle &muscle = m.second;

        if (frameID == -1) {
            muscle.activation = 0;
        } else {
            muscle.activation = muscleActivations.at(frameID).at(muscle.name);//update the activation for muscle acoording to the activation file
        }
    }
    return cmuscleData;
}

int twin::Osim::getCurrentFrame() const {
    return this->current_frame;
}

int twin::Osim::getMotionFrame() const {
    return this->motion_frame;
}

twin::BodyConfiguration &twin::Osim::getBodyConfig(const int frame) {
    return bconfigs.at(frame + 1);
}
