#include "twinviewer.h"
#include "extensions/camera.h"
#include <argparse/argparse.hpp>
#include "gui.h"
#include <sstream>
#include <iomanip>
#include <tinyfiledialogs.h>

TWIN_NAMESPACE_BEGIN

    bool TwinViewer::c_gui = false;
    bool TwinViewer::c_guiconf = false;

    TwinViewer::TwinViewer(int argc, char *argv[]) : m_frame(0), m_prevFrame(0), m_record(false), m_recordframe(0), m_autoplay(false), m_syncplay(false), m_virtTime(0.0), m_maxFramerate(-1), m_frameadv(60), m_minFrame(std::numeric_limits<int>::max()), m_maxFrame(-1), m_dataset(DatasetType::NONE), m_osimstr(""), m_motstr(""), m_stostr(""), m_scastr(""), m_mot(false), m_sca(false), m_sto(false), m_zeroed(false), m_swap(true) {
        cppgl::Context::show_gui=true;
        //m_paramsarams init
        parseCmd(argc, argv);
        // interaction init
        m_interact = twin::Interaction();

        setupWindow();
        if (!m_params.osimMod.empty()) {
            m_osim.push_back(std::make_shared<Osim>(m_params.osimMod, m_params.osimSca, m_params.osimMot, m_params.osimSto));
        }
        if (!m_params.c3dP.empty()) {
            m_c3d.push_back(std::make_shared<C3D>(m_params.c3dP));
        }
        for (std::shared_ptr<Osim> &osim: m_osim) {
            m_minFrame = std::min(m_minFrame, osim->getNumFrames());
            m_maxFrame = std::max(m_maxFrame, osim->getNumFrames());
        }
        for (std::shared_ptr<C3D> &c3d: m_c3d) {
            m_minFrame = std::min(c3d->getNumFrames(), m_minFrame);
            m_maxFrame = std::max(c3d->getNumFrames(), m_maxFrame);
        }
        for (std::shared_ptr<JMarker> &mjs: m_mjs) {
            m_minFrame = std::min(mjs->getNumFrames(), m_minFrame);
            m_maxFrame = std::max(mjs->getNumFrames(), m_maxFrame);
        }

#ifdef HAS_DL_MODULE
        addOptimizerset();
#endif


        cppgl::gui_add_callback(m_params.title, GuiCallbackWrapper<TwinViewer>, this);
        cppgl::gui_add_callback("twinviewer", drawClassGUI);
        cppgl::gui_add_callback("config", drawConfigGUI);
    }

    TwinViewer::~TwinViewer() {
        cppgl::gui_remove_callback(m_params.title);
    }

    void TwinViewer::setupWindow() {

        // init GL
        cppgl::ContextParameters params;
        params.width = m_params.width;
        params.height = m_params.height;
        params.title = m_params.title;
        // params.floating = GLFW_TRUE;
        // params.resizable = GLFW_FALSE;
        params.swap_interval = m_params.interval;
        cppgl::Context::init(params);

        // setup fbo
        const cppgl::ivec2 res = cppgl::Context::resolution();
        if (!cppgl::Framebuffer::valid("std_fbo")) {
            fbo = cppgl::Framebuffer("std_fbo", res.x(), res.y());
        }
        fbo->attach_depthbuffer(cppgl::Texture2D("std_fbo/depth", res.x(), res.y(), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT));
        fbo->attach_colorbuffer(cppgl::Texture2D("std_fbo/col", res.x(), res.y(), GL_RGBA32F, GL_RGBA, GL_FLOAT));
        fbo->check();
        // setup draw shader
        cppgl::Shader s1("draw", "shader/draw.vert", "shader/draw.frag");
        cppgl::Shader s2("blinnPhong", "shader/blinnPhong.vert", "shader/blinnPhong.frag");
        cppgl::Shader s3("pointCloud", "shader/pointCloud.vert", "shader/pointCloud.frag");
        cppgl::Shader s4("fallback", "shader/quad.vert", "shader/fallback.frag");
        if (!cppgl::Shader::valid("blit"))
            blit = cppgl::Shader("blit", "shader/quad.vert", "shader/blit.frag");

        //own Camera
        Camera twincam("standard");
        cppgl::make_camera_current(twincam);

        // install callbacks
        cppgl::Context::set_resize_callback(ResizeCallbackWrapper<TwinViewer>, this);
        cppgl::Context::set_keyboard_callback(KeyboardCallbackWrapper<TwinViewer>, this);
        cppgl::Context::set_mouse_callback(MousePosCallbackWrapper<TwinViewer>, this);
        cppgl::Context::set_mouse_button_callback(MouseButtonCallbackWrapper<TwinViewer>, this);
        cppgl::Context::set_mouse_scroll_callback(MouseScrollCallbackWrapper<TwinViewer>, this);
        cppgl::current_camera()->dir = m_params.camDir;
        cppgl::current_camera()->pos = m_params.camPos;
    }

    twinParams TwinViewer::parseCmd(int argc, char **argv) {
        argparse::ArgumentParser program("twinVis");
        program.add_argument("--configPath", "-cfgP")
                .help("Path to a configFile")
                .default_value(std::string("."));
        program.add_argument("--outputDir", "-out")
                .help("Output directory")
                .default_value(std::string("./out"));
        program.add_argument("--inputDir", "-in")
                .help("Input directory")
                .default_value(std::string("./data"));
        program.add_argument("--commandLineOnly", "-cmdO")
                .help("Programm doesn't visualize and just saves output sequence")
                .default_value(false)
                .implicit_value(true);
        program.add_argument("--noConfig", "-noC")
                .help("Use if the config file shouldn't be used")
                .default_value(false)
                .implicit_value(true);
        program.add_argument("--width", "-wi")
                .help("Window width")
                .default_value(1920)
                .scan<'i', int>();
        program.add_argument("--height", "-he")
                .help("Window height")
                .default_value(1080)
                .scan<'i', int>();
        program.add_argument("--interval", "-inter")
                .help("Frameinterval value for cppgl, 0 = no vsync, 1 = 60fps, 2 = 30fps, etc")
                .default_value(1)
                .scan<'i', int>();
        program.add_argument("--title", "-tit")
                .help("Window title")
                .default_value(std::string("TwinVis"));
        program.add_argument("--osimModel", "-omod")
                .help("Path to .osim file containing the osimModel")
                .default_value(std::string(""));
        program.add_argument("--osimMotion", "-omot")
                .help("Path to .mot file containing the osimMotion")
                .default_value(std::string(""));
        program.add_argument("--osimScale", "-osca")
                .help("Path to .xml file containing the osimScaleFactors")
                .default_value(std::string(""));
        program.add_argument("--osimSto", "-osto")
                .help("Path to .sto file containing the osimMuscleActivation")
                .default_value(std::string(""));
        program.add_argument("--cameraPosition", "-camPos")
                .help("Starting position of the camera (input looks like {1.0, 0.0, 2.0})")
                .nargs(3)
                .default_value(std::vector<double>{0.0, 0.0, 0.0})
                .scan<'g', double>();
        program.add_argument("--cameraDirection", "-camDir")
                .help("Starting direction of the camera (input looks like {1.0, 0.0, 0.0})")
                .nargs(3)
                .default_value(std::vector<double>{1.0, 0.0, 0.0})
                .scan<'g', double>();
        program.add_argument("--defaultConstraints", "-defCons")
                .help("Path to default constraints")
                .default_value(std::string("./data/default_constraints.json"));
        program.add_argument("--c3dPath", "-c3dP")
                .help("Path to c3d file")
                .default_value(std::string(""));
        program.add_argument("--suprPath", "-suprP")
                .help("Path to supr files")
                .default_value(std::string(""));
        program.add_argument("--sceneEval", "-sE")
                .help("Path to eval files")
                .default_value(std::string(""));

        try {
            program.parse_args(argc, argv);
        }
        catch (const std::runtime_error &err) {
            LOG(FATAL) << err.what() << std::endl;
            LOG(FATAL) << program;
            std::exit(1);
        }
        std::vector<double> cD = program.get<std::vector<double>>("-camDir");
        std::vector<double> cP = program.get<std::vector<double>>("-camPos");
        twinParams twin = twinParams();
        twin.cmdOnly = program.get<bool>("-cmdO");
        twin.inDir = program.get<std::string>("-in");
        twin.outDir = program.get<std::string>("-out");
        twin.width = program.get<int>("-wi");
        twin.height = program.get<int>("-he");
        twin.cfgP = program.get<std::string>("-cfgP");
        twin.title = program.get<std::string>("-tit");
        twin.interval = program.get<int>("-inter");
        twin.osimMod = program.get<std::string>("-omod");
        twin.osimSca = program.get<std::string>("-osca");
        twin.osimMot = program.get<std::string>("-omot");
        twin.osimSto = program.get<std::string>("-osto");
        twin.camPos = cppgl::vec3(static_cast<float>(cP.at(0)), static_cast<float>(cP.at(1)), static_cast<float>(cP.at(2)));
        twin.camDir = cppgl::vec3(static_cast<float>(cD.at(0)), static_cast<float>( cD.at(1)), static_cast<float>(cD.at(2)));
        twin.defCon = program.get<std::string>("-defCons");
        twin.c3dP = program.get<std::string>("-c3dP");
        twin.suprP = program.get<std::string>("-suprP");
        if (std::filesystem::exists(program.get<std::string>("-cfgP") + "/twinConfig.json") && !program.get<bool>("-noC")) {
            std::ifstream f(program.get<std::string>("-cfgP") + "/twinConfig.json");
            nlohmann::json data = nlohmann::json::parse(f);

            twin = twinParams();

            twin.cmdOnly = data["cmd"];
            twin.inDir = data["in"];
            twin.outDir = data["out"];
            twin.width = data["width"];
            twin.height = data["height"];
            twin.cfgP = data["configPath"];
            twin.title = data["windowTitle"];
            twin.interval = data["frameInterval"];
            twin.osimMod = data["osim"]["Model"];
            twin.osimSca = data["osim"]["Scalefactor"];
            twin.osimMot = data["osim"]["Motion"];
            twin.osimSto = data["osim"]["Sto"];
            twin.camPos = cppgl::vec3(data["cameraPosition"]["x"], data["cameraPosition"]["y"], data["cameraPosition"]["z"]);
            twin.camDir = cppgl::vec3(data["cameraDirection"]["x"], data["cameraDirection"]["y"], data["cameraDirection"]["z"]);
            twin.defCon = data["defaultConstrPath"];
            twin.c3dP = data["c3dPath"];
            twin.suprP = data["suprPath"];
        }

        twin.evalS = program.get<std::string>("-sE");
        m_params = twin;
        return twin;
    }

    void TwinViewer::writeConfig() {
        nlohmann::json conf;
        conf["cmd"] = m_params.cmdOnly;
        conf["in"] = m_params.inDir;
        conf["out"] = m_params.outDir;
        conf["width"] = m_params.width;
        conf["height"] = m_params.height;
        conf["configPath"] = m_params.cfgP;
        conf["windowTitle"] = m_params.title;
        conf["frameInterval"] = m_params.interval;
        conf["osim"]["Model"] = m_params.osimMod;
        conf["osim"]["Scalefactor"] = m_params.osimSca;
        conf["osim"]["Motion"] = m_params.osimMot;
        conf["osim"]["Sto"] = m_params.osimSto;
        cppgl::vec3 tmpdir = cppgl::current_camera()->dir;
        cppgl::vec3 tmppos = cppgl::current_camera()->pos;
        conf["cameraPosition"]["x"] = tmppos.x();
        conf["cameraPosition"]["y"] = tmppos.y();
        conf["cameraPosition"]["z"] = tmppos.z();
        conf["cameraDirection"]["x"] = tmpdir.x();
        conf["cameraDirection"]["y"] = tmpdir.y();
        conf["cameraDirection"]["z"] = tmpdir.z();
        conf["defaultConstrPath"] = m_params.defCon;
        conf["c3dPath"] = m_params.c3dP;
        conf["suprPath"] = m_params.suprP;
        std::ofstream o(m_params.cfgP + "/twinConfig.json");
        o << std::setw(4) << conf << std::endl;
    }


    static bool wfMode = true;

    void togWF() {
        for (auto &[key, drawelement]: cppgl::Drawelement::map) {
            drawelement->set_wireframe_mode(wfMode);
        }
        wfMode = !wfMode;
    }

    void TwinViewer::drawGUI() {
        if (c_guiconf) {
            ImGui::PushID("Config");
            if (ImGui::Begin(std::string("TwinVis Config##").c_str(), &c_guiconf)) {
                ImGui::InputText(_labelPrefix("Window title: ").c_str(), &m_params.title);
                ImGui::InputInt(_labelPrefix("Window Width: ").c_str(), &m_params.width);
                ImGui::InputInt(_labelPrefix("Window Height: ").c_str(), &m_params.height);
                ImGui::Combo(_labelPrefix("FPS mode: ").c_str(), &m_params.interval, "Unlimited\00060fps\00030fps\000");
                ImGui::InputText(_labelPrefix("Config file path: ").c_str(), &m_params.cfgP);
                ImGui::InputText(_labelPrefix("Input directory: ").c_str(), &m_params.inDir);
                ImGui::InputText(_labelPrefix("Output directory: ").c_str(), &m_params.outDir);
                ImGui::InputFloat3(_labelPrefix("CamPos:").c_str(), m_params.camPos.data());
                if (ImGui::InputFloat3(_labelPrefix("CamDir:").c_str(), m_params.camDir.data())) {
                    m_params.camDir.normalize();
                }
                if (ImGui::InputFloat3(_labelPrefix("CamUp:").c_str(), m_params.camUp.data())) {
                    m_params.camUp.normalize();
                }
                if (cppgl::Material::valid("network_mat")) {
                    if (ImGui::InputFloat4(_labelPrefix("LightDir:").c_str(), cppgl::Material::find("network_mat")->vec4_map["lightDir"].data())) {
                        cppgl::Material::find("network_mat")->vec4_map["lightDir"].normalize();
                    }
                }
                if (ImGui::Button("Save configfile##")) {
                    writeConfig();
                }
                if (ImGui::CollapsingHeader("Keybindings")) {
                    ImGui::Text("Reload Shaders: Shift + R");
                    ImGui::Text("Screenshot: Enter");
                    ImGui::Text("Toggle Wireframe: M");
                    ImGui::Text("Start/Stop Recording: Ctrl + Alt + V");
                    ImGui::Text("Start Optimizer: Ctrl + O");
                    ImGui::Text("Stop Optimizer: Shift + O");
                    ImGui::Text("Next Frame: N");
                    ImGui::Text("Previous Frame: B");
                    ImGui::Text("Toggle Autoplay: P");
                    ImGui::Text("Toggle synchronized Autoplay: Shift + P");
                    ImGui::Text("Reset Camera pos and dir: C");
                }
            }
            ImGui::End();
            ImGui::PopID();
        }
        if (c_gui) {
            ImGui::PushID("TwinViewer");
            if (ImGui::Begin(std::string("TwinViewer##").c_str(), &c_gui)) {
                if (ImGui::SliderInt(_labelPrefix("Motion frame").c_str(), &m_frame, -1, m_maxFrame - 1)) {
                    refreshFrame();
                }
                ImGui::Checkbox(_labelPrefix("Autoplay:").c_str(), &m_autoplay);
                if (ImGui::Checkbox(_labelPrefix("Syncplay:").c_str(), &m_syncplay)) {
                    m_virtTime = 0.0;
                }
                ImGui::SliderInt(_labelPrefix("Advance every x frame:").c_str(), &m_frameadv, 1, 250);
                if (ImGui::Button("ToggleWireframe##")) {
                    togWF();
                }
                ImGui::Checkbox(_labelPrefix("Model in Origin:").c_str(), &m_zeroed);
                datasetGUI();
                bool dummy;
                if (!m_optis.empty() && ImGui::CollapsingHeader("Models")) {
                    ImGui::PushID("Models");
                    ImGui::Indent(3);
                    for (int i = 0; i < static_cast<int>(m_optis.size()); ++i) {
                        ImGui::PushID(i);
                        if (ImGui::CollapsingHeader(std::to_string(i).c_str())) {
                            if (ImGui::InputFloat3(_labelPrefix("Translation").c_str(), m_optis.at(i).trans.data())) {
                                torch::Tensor t = m_optis.at(i).netzP->GetTrans().to(torch::kCPU);
                                float t0 = t[0][0].item<float>();
                                float t1 = t[0][1].item<float>();
                                float t2 = t[0][2].item<float>();
                                m_optis.at(i).netz->SetTranslation(-cppgl::vec3(t0, t1, t2) + m_optis.at(i).trans);
                            }
                            ImGui::InputFloat3(_labelPrefix("Rot Pivot").c_str(), m_optis.at(i).pivot.data());
                            if (SliderReset("Rotation", m_optis.at(i).rot.data(), -M_PI / 2, M_PI, dummy)) {
                                m_optis.at(i).netz->SetRotation(m_optis.at(i).rot);
                            }
                        }
                        ImGui::PopID();
                    }
                    ImGui::Unindent(3);
                    ImGui::PopID();
                }
                if (ImGui::CollapsingHeader("Record")) {
                    if (m_record) {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
                        ImGui::Text("Recording in progress");
                        ImGui::PopStyleColor();
                        if (ImGui::Button("Stop recording##")) {
                            m_record = false;
                        }
                    } else {
                        if (ImGui::Button("Start recording##")) {
                            m_record = true;
                            m_recordframe = 0;
                        }
                    }
                    if (ImGui::Button("Screenshot")) {
                        cppgl::Context::screenshot(m_params.outDir + "/screenshot.png");
                    }
                }
            }
            ImGui::End();
            ImGui::PopID();
        }
        if (cppgl::Context::show_gui) {
            if (ImGui::BeginMainMenuBar()) {
                ImGui::Separator();
                if (ImGui::Button("Save current model##")) {
#ifdef HAS_DL_MODULE
                    int i = 0;
                    for (auto &[opti, consHand, optP, netz, netzP, bvh, trans, pivot, rot, zeroFrame]: m_optis) {
                        save_mesh_cpu(netz->m_mesh->mesh, m_params.outDir + "/" + std::to_string(i++) + ".obj", "",
                                      false);
                    }
#endif
                }
                ImGui::EndMainMenuBar();
            }
        }
    }

    void TwinViewer::drawClassGUI() {
        if (cppgl::Context::show_gui) {
            if (ImGui::BeginMainMenuBar()) {
                ImGui::Separator();
                ImGui::Checkbox("TwinViewer##", &c_gui);
                ImGui::EndMainMenuBar();
            }
        }
    }

    void TwinViewer::drawConfigGUI() {
        if (cppgl::Context::show_gui) {
            if (ImGui::BeginMainMenuBar()) {

                ImGui::Separator();
                ImGui::Checkbox("Config##", &c_guiconf);
                ImGui::EndMainMenuBar();
            }
        }
    }

    void TwinViewer::run() {
        cppgl::Timer framet;
        while (cppgl::Context::running()) {
            // handle input
            glfwPollEvents();
            CameraImpl::default_input_handler(cppgl::Context::frame_time());
            // update and reload shaders
            cppgl::current_camera()->update();
            static uint32_t frame_counter = 0;
            if (frame_counter++ % 100 == 0)
                cppgl::reload_modified_shaders();

            // render all drawelements into fbo
            fbo->bind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            for (std::shared_ptr<Osim> &osim: m_osim) {
                osim->draw();
            }
            for (std::shared_ptr<C3D> &c3d: m_c3d) {
                c3d->Render();
            }
            for (std::shared_ptr<JMarker> &mjs: m_mjs) {
                mjs->Render();
            }

#ifdef HAS_DL_MODULE
            for (auto &[opti, consHand, optP, netz, netzP, bvh, trans, pivot, rot, zeroFrame]: m_optis) {
                opti->Update(netzP);
                netz->Inference(netzP);
                cppgl::Material::find("network_mat")->vec3_map["pivot"] = pivot;
                netz->Render();
                bvh.setNextTris(netz->getTriangleSoup());
                consHand.RecomputeRendering(netz->m_obj, true);
                consHand.RenderImGui(netz, m_osim, m_c3d, m_mjs, netz->m_constraints, cppgl::mat4::Identity(), bvh, m_interact.ray, m_interact.relative_pixel, m_interact.pressed, false, zeroFrame, m_maxFramerate);
                if (m_syncplay && consHand.getFramerate() != -1) {
                    int curframe = static_cast<int>(m_virtTime * consHand.getFramerate() + 1e-6) + zeroFrame;
                    consHand.SetFrame(curframe % m_minFrame);
                    if (curframe == zeroFrame) {
                        torch::Tensor t = netzP->GetTrans().to(torch::kCPU);
                        float t0 = t[0][0].item<float>();
                        float t1 = t[0][1].item<float>();
                        float t2 = t[0][2].item<float>();
                        pivot = -cppgl::vec3(t0, t1, t2);
                        netz->SetTranslation(-cppgl::vec3(t0, t1, t2) + trans);
                    }
                } else if (m_zeroed) {
                    torch::Tensor t = netzP->GetTrans().to(torch::kCPU);
                    float t0 = t[0][0].item<float>();
                    float t1 = t[0][1].item<float>();
                    float t2 = t[0][2].item<float>();
                    pivot = -cppgl::vec3(t0, t1, t2);
                    netz->SetTranslation(-cppgl::vec3(t0, t1, t2) + trans);
                } else {
                    consHand.Render();
                }
            }
#endif
            fbo->unbind();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            blit->bind();
            blit->uniform("tex", fbo->color_textures.at(0), 0);
            cppgl::Quad::draw();
            blit->unbind();
            // finish frame
            if (m_swap) {
                cppgl::Context::swap_buffers();
            }
            if (m_record) {
                std::stringstream ss;
                ss << std::setw(6) << std::setfill('0') << m_recordframe;
                std::string s = ss.str();
                fbo->color_textures.at(0)->save_ldr(m_params.outDir + "/screenshot_" + ss.str() + ".png", true, true);
            }
            if (m_autoplay && (frame_counter % m_frameadv == 0 || (framet.look() / 1000.) > (static_cast<double>(m_frameadv) / 60.))) {
                m_frame = (m_frame + 1) % (m_minFrame - 1);
                refreshFrame();
                if (m_frame < m_recordframe) {
                    m_record = false;
                }
                m_recordframe = m_frame;
                framet.begin();
            } else if (m_maxFramerate != -1 && m_syncplay && frame_counter % m_frameadv == 0) {
                m_virtTime += 1.0 / m_maxFramerate;
                ++m_recordframe;
            }
        }
    }

    void TwinViewer::mouse_button_callback(int button, int action, int mods) {
        if (ImGui::GetIO().WantCaptureMouse)
            return;
        if (button == GLFW_MOUSE_BUTTON_LEFT && mods & GLFW_MOD_SHIFT && action == GLFW_PRESS) {
            cppgl::current_camera()->can_move = false;
            m_interact.disable_camera_interpolation = true;

            cppgl::vec2 rel = cppgl::Context::mouse_pos();
            cppgl::ivec2 size = cppgl::Context::resolution();
            cppgl::vec2 size2(size.x(), size.y());
            m_interact.relative_pixel = rel.array() / size2.array();
            m_interact.pressed = true;
            if (rel.x() >= 0 && rel.x() < static_cast<float>(size.x()) && rel.y() >= 0 && rel.y() < static_cast<float>(size.y()) && cppgl::Camera::valid("standard")) {
                std::shared_ptr<CameraImpl> cam = std::dynamic_pointer_cast<CameraImpl>(cppgl::Camera::find("standard").ptr);
                m_interact.ray = cam->PixelRay(rel, size.x(), size.y(), true);
            }
        } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            cppgl::current_camera()->can_move = true;
            m_interact.pressed = false;
            m_interact.shift = false;
            m_interact.disable_camera_interpolation = false;
        }
    }

    void TwinViewer::keyboard_callback(int key, int scancode, int action, int mods) {
        if (ImGui::GetIO().WantCaptureKeyboard)
            return;
        if (mods == GLFW_MOD_SHIFT && key == GLFW_KEY_R && action == GLFW_PRESS) {
            cppgl::reload_modified_shaders();
        } else if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
            cppgl::Context::screenshot(m_params.outDir + "/screenshot.png");
        } else if (key == GLFW_KEY_M && action == GLFW_PRESS) {
            togWF();
        } else if (key == GLFW_KEY_V && ((mods & GLFW_MOD_CONTROL) && (mods & GLFW_MOD_ALT)) && action == GLFW_PRESS) {
            m_record = !m_record;
            m_recordframe = 0;
        } else if (key == GLFW_KEY_O && (mods & GLFW_MOD_CONTROL) && action == GLFW_PRESS) {
#ifdef HAS_DL_MODULE
            for (auto &[opti, consHand, optP, netz, netzP, bvh, trans, pivot, rot, zeroFrame]: m_optis) {
                opti->StartConcurrentOptimizer();
            }
#endif
        } else if (key == GLFW_KEY_O && (mods & GLFW_MOD_SHIFT) && action == GLFW_PRESS) {
#ifdef HAS_DL_MODULE
            for (auto &[opti, consHand, optP, netz, netzP, bvh, trans, pivot, rot, zeroFrame]: m_optis) {
                opti->StopConcurrentOptimizer();
            }
#endif
        } else if (key == GLFW_KEY_N && action == GLFW_PRESS) {
            ++m_frame;
            refreshFrame();
        } else if (key == GLFW_KEY_B && action == GLFW_PRESS) {
            --m_frame;
            refreshFrame();
        } else if (key == GLFW_KEY_P && (mods & GLFW_MOD_SHIFT) && action == GLFW_PRESS) {
            m_syncplay = !m_syncplay;
        } else if (key == GLFW_KEY_P && action == GLFW_PRESS) {
            m_autoplay = !m_autoplay;
        } else if (key == GLFW_KEY_C && (mods & GLFW_MOD_SHIFT) && action == GLFW_PRESS) {
            std::cout << "Campos: " << cppgl::current_camera()->pos.transpose() << "\n";
            std::cout << "Camdir: " << cppgl::current_camera()->dir.transpose() << "\n";
            std::cout << "Camup: " << cppgl::current_camera()->up.transpose() << std::endl;
        } else if (key == GLFW_KEY_C && action == GLFW_PRESS) {
            cppgl::current_camera()->dir = m_params.camDir;
            cppgl::current_camera()->pos = m_params.camPos;
            cppgl::current_camera()->up = m_params.camUp;
        } else if (key == GLFW_KEY_K && (mods & GLFW_MOD_SHIFT) && action == GLFW_PRESS) {
            m_swap = !m_swap;
        }
    }

    void TwinViewer::resize_callback(int w, int h) {
        fbo->resize(w, h);
        m_params.height = h;
        m_params.width = w;
    }

    void TwinViewer::mouse_scroll_callback(double xoffset, double yoffset) {}

    void TwinViewer::mouse_pos_callback(double xpo, double ypos) {
        if (ImGui::GetIO().WantCaptureMouse)
            return;

        if (m_interact.pressed && cppgl::Camera::valid("standard")) {
            cppgl::vec2 rel = cppgl::Context::mouse_pos();
            cppgl::ivec2 size = cppgl::Context::resolution();
            cppgl::vec2 size2(size.x(), size.y());
            m_interact.relative_pixel = rel.array() / size2.array();
            std::shared_ptr<twin::CameraImpl> cam = std::dynamic_pointer_cast<twin::CameraImpl>(cppgl::Camera::find("standard").ptr);
            m_interact.ray = cam->PixelRay(rel, size.x(), size.y(), true);
        }
    }

    void TwinViewer::refreshFrame() {
        if (m_frame == m_prevFrame) {
            return;
        }
        m_prevFrame = m_frame;
        for (std::shared_ptr<Osim> &osim: m_osim) {
            osim->SetStateToFrame(std::min(m_frame, osim->getNumFrames() - 1));
        }
        for (std::shared_ptr<C3D> &c3d: m_c3d) {
            c3d->setFrame(std::max(std::min(m_frame, c3d->getNumFrames() - 1), 0));
        }
        for (std::shared_ptr<JMarker> &mjs: m_mjs) {
            mjs->setFrame(std::max(std::min(m_frame, mjs->getNumFrames() - 1), 0));
        }

#ifdef HAS_DL_MODULE
        for (auto &[opti, consHand, optP, netz, netzP, bvh, trans, pivot, rot, zeroFrame]: m_optis) {
            consHand.SetFrame(m_frame);
        }
#endif
    }

    void TwinViewer::addOptimizerset() {
#ifdef HAS_DL_MODULE
        Contraints cons("Constraints");
        SUPR supr("Supr", m_params.suprP, twin::Gender::Neutral);
        supr->SetConstraints(cons);
        supr->Inference();
        ConstraintHandler consHand(m_params.defCon);
        std::shared_ptr<OptimizationParams> optP = std::make_shared<OptimizationParams>();
        NetworkParameters netzP = supr->params;
        Network netz = supr;
        std::shared_ptr<Optimizer> opti = std::make_shared<Optimizer>("Optimizer", optP, netz, true);
        opti->UpdateInputs(netzP, optP, true);
        BVH bvh;

        std::vector<twin::Triangle> tris = supr->getTriangleSoup();
        bvh.setNextTris(tris);

        m_optis.push_back({opti, consHand, optP, netz, netzP, bvh, cppgl::vec3(1.F * static_cast<float>(m_optis.size()), 0, 0), cppgl::vec3::Zero(), cppgl::vec3::Zero(), -1});

#endif
    }

    void TwinViewer::addC3D(const std::string &path) {
        std::shared_ptr<C3D> tmpptr = std::make_shared<C3D>(path);
        m_minFrame = std::min(tmpptr->getNumFrames(), m_minFrame);
        m_maxFrame = std::max(tmpptr->getNumFrames(), m_maxFrame);
        m_c3d.push_back(tmpptr);
    }

    void TwinViewer::addJMarker(const std::string &path) {
        std::shared_ptr<JMarker> tmpptr = std::make_shared<JMarker>(path);
        m_minFrame = std::min(tmpptr->getNumFrames(), m_minFrame);
        m_maxFrame = std::max(tmpptr->getNumFrames(), m_maxFrame);
        m_mjs.push_back(tmpptr);
    }

    void TwinViewer::addOsim(const std::string &filepath_osim, const std::string &filepath_mot, const std::string &filepath_sto, const std::string &filepath_scale) {
        std::shared_ptr<Osim> tmpptr = std::make_shared<Osim>(filepath_osim, filepath_scale, filepath_mot, filepath_sto);
        m_minFrame = std::min(tmpptr->getNumFrames(), m_minFrame);
        m_maxFrame = std::max(tmpptr->getNumFrames(), m_maxFrame);
        m_osim.push_back(tmpptr);
    }

    void TwinViewer::datasetGUI() {
        ImGui::PushID("Datasets");

        if (m_dataset == DatasetType::NONE) {
            if (ImGui::Button("Add C3D")) {
                m_dataset = DatasetType::DT_C3D;
            }
            ImGui::SameLine();
            if (ImGui::Button("Add OSIM")) {
                m_dataset = DatasetType::DT_OSIM;
            }
            ImGui::SameLine();
            if (ImGui::Button("Add JSONMarker")) {
                m_dataset = DatasetType::DT_JSON;
            }
#ifdef HAS_DL_MODULE
            ImGui::SameLine();
            if (ImGui::Button("Add Model")) {
                addOptimizerset();
            }
#endif
        } else if (m_dataset == DatasetType::DT_JSON) {
            const char *filters[] = {"*.json", "*.JSON", "*.Json"};  // File filters

            char const *filename = tinyfd_openFileDialog(
                    "Open a file",             // Dialog title
                    "",                        // Default path
                    2,                         // Number of filters
                    filters,                   // Filter patterns
                    "JSON File",        // Filter description (can be NULL)
                    0                          // Allow multiple file selection: 0 for single file, 1 for multiple
            );

            if (filename) {
                std::string file = filename;
                addJMarker(file);
            }
            m_dataset = DatasetType::NONE;

        } else if (m_dataset == DatasetType::DT_C3D) {
            const char *filters[] = {"*.c3d", "*.C3D"};  // File filters

            char const *filename = tinyfd_openFileDialog(
                    "Open a file",             // Dialog title
                    "",                        // Default path
                    2,                         // Number of filters
                    filters,                   // Filter patterns
                    "C3D File",        // Filter description (can be NULL)
                    0                          // Allow multiple file selection: 0 for single file, 1 for multiple
            );

            if (filename) {
                std::string file = filename;
                addC3D(file);
            }
            m_dataset = DatasetType::NONE;

        } else if (m_dataset == DatasetType::DT_OSIM) {
            if (!m_osimstr.empty()) {
                ImGui::Text("Osim file path: %s", m_osimstr.c_str());
                ImGui::SameLine();
            }
            if (ImGui::Button("Select Osim file")) {
                cppgl::current_camera()->can_move = false;
                const char *filters[] = {"*.osim", "*.Osim", "*.OSIM"};  // File filters

                char const *filename = tinyfd_openFileDialog(
                        "Open a file",             // Dialog title
                        "",                        // Default path
                        3,                         // Number of filters
                        filters,                   // Filter patterns
                        "Osim File",        // Filter description (can be NULL)
                        0                          // Allow multiple file selection: 0 for single file, 1 for multiple
                );

                if (filename) {
                    m_osimstr = filename;
                }
                cppgl::current_camera()->can_move = true;
            }
            ImGui::PushID("Mot");
            ImGui::Text("Mot file");
            ImGui::SameLine();
            ImGui::PushID("Check");
            ImGui::Checkbox("", &m_mot);
            ImGui::PopID();
            if (m_mot) {
                ImGui::SameLine();
                if (ImGui::Button("Select mot file")) {
                    const char *filters[] = {"*.mot", "*.MOT"};  // File filters

                    char const *filename = tinyfd_openFileDialog(
                            "Open a file",             // Dialog title
                            "",                        // Default path
                            2,                         // Number of filters
                            filters,                   // Filter patterns
                            "Mot File",        // Filter description (can be NULL)
                            0                          // Allow multiple file selection: 0 for single file, 1 for multiple
                    );

                    if (filename) {
                        m_motstr = filename;
                    }
                }
                if (!m_motstr.empty()) {
                    ImGui::SameLine();
                    ImGui::Text("%s", m_motstr.c_str());
                }
            } else {
                m_motstr = "";
            }
            ImGui::PopID();
            ImGui::PushID("Sto");
            ImGui::Text("Sto file");
            ImGui::SameLine();
            ImGui::PushID("Check");
            ImGui::Checkbox("", &m_sto);
            ImGui::PopID();
            if (m_sto) {
                ImGui::SameLine();
                if (ImGui::Button("Select sto file")) {
                    const char *filters[] = {"*.sto", "*.STO"};  // File filters

                    char const *filename = tinyfd_openFileDialog(
                            "Open a file",             // Dialog title
                            "",                        // Default path
                            2,                         // Number of filters
                            filters,                   // Filter patterns
                            "Sto File",        // Filter description (can be NULL)
                            0                          // Allow multiple file selection: 0 for single file, 1 for multiple
                    );

                    if (filename) {
                        m_stostr = filename;
                    }
                }
                if (!m_stostr.empty()) {
                    ImGui::SameLine();
                    ImGui::Text("%s", m_stostr.c_str());
                }
            } else {
                m_stostr = "";
            }
            ImGui::PopID();
            ImGui::PushID("Sca");
            ImGui::Text("XML scale");
            ImGui::SameLine();
            ImGui::PushID("Check");
            ImGui::Checkbox("", &m_sca);
            ImGui::PopID();
            if (m_sca) {
                ImGui::SameLine();
                if (ImGui::Button("Select xml scale file")) {
                    const char *filters[] = {"*.xml", "*.XML"};  // File filters

                    char const *filename = tinyfd_openFileDialog(
                            "Open a file",             // Dialog title
                            "",                        // Default path
                            2,                         // Number of filters
                            filters,                   // Filter patterns
                            "XML File",        // Filter description (can be NULL)
                            0                          // Allow multiple file selection: 0 for single file, 1 for multiple
                    );

                    if (filename) {
                        m_scastr = filename;
                    }
                }
                if (!m_scastr.empty()) {
                    ImGui::SameLine();
                    ImGui::Text("%s", m_scastr.c_str());
                }
            } else {
                m_scastr = "";
            }
            ImGui::PopID();
            if (!m_osimstr.empty() && ImGui::Button("Add")) {
                addOsim(m_osimstr, m_motstr, m_stostr, m_scastr);
                m_osimstr = "";
                m_motstr = "";
                m_stostr = "";
                m_scastr = "";
                m_mot = false;
                m_sto = false;
                m_sca = false;
                m_dataset = DatasetType::NONE;
            }
        }

        ImGui::PopID();
    }


TWIN_NAMESPACE_END
