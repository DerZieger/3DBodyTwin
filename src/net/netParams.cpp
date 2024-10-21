#ifdef HAS_DL_MODULE

#include "net/netParams.h"
#include "gui.h"


void twin::NetworkParametersImpl::CopyCommon(NetworkParameters &net) {
    std::lock_guard<std::mutex> netlock(net->m_lock);
    std::lock_guard<std::mutex> lock(m_lock);
    m_beta = net->m_beta;
    m_id = net->m_id;
    if (m_pose_loss > 10) {
        m_pose_body = net->m_pose_body;
        m_pose_hand = net->m_pose_hand;
        m_pose_foot = net->m_pose_foot;
        m_pose_jaw = net->m_pose_jaw;
        m_pose_eye = net->m_pose_eye;
        m_vposer_repr = net->m_vposer_repr;
        m_root_orient = net->m_root_orient;
        m_translation = net->m_translation;
    }
}

twin::NetworkParametersImpl::NetworkParametersImpl(std::string cname, const ModelType &t, bool constrained) : name(std::move(cname)), m_root_orient(cppgl::vec3::Zero()), m_translation(cppgl::vec3::Zero()), m_use_vposer(false), m_recompute(false), m_lock_threshold(0.01f), m_pose_loss(1e10f), m_loss(1e10f), m_id(id_counter++), m_type(t), m_constrained(constrained) {}

twin::NetworkParametersImpl::NetworkParametersImpl() : NetworkParametersImpl(std::to_string(id_counter)) {}

void twin::NetworkParametersImpl::init() {
    m_pose_body.resize(BodyEntries(), cppgl::vec3::Zero());
    m_pose_hand.resize(HandEntries(), cppgl::vec3::Zero());
    m_pose_foot.resize(FootEntries(), cppgl::vec3::Zero());
    m_pose_jaw.resize(JawEntries(), cppgl::vec3::Zero());
    m_pose_eye.resize(EyeEntries(), cppgl::vec3::Zero());
    m_beta.resize(BetaEntries(), 0);
    m_beta_lock.resize(BetaEntries(), PermanentUnlocked);
    m_vposer_repr.resize(32, 0);
    m_root_orient = cppgl::vec3::Zero();
    m_translation = cppgl::vec3::Zero();
}

bool twin::NetworkParametersImpl::HasFoot() const {
    return false;
}

std::vector<std::string> twin::NetworkParametersImpl::FootJointNames() const {
    return {};
}

void twin::NetworkParametersImpl::UpdateLock() {
    if (m_beta_lock.empty()) {
        m_beta_lock = std::vector<Lock>(m_beta.size(), m_recompute ? Lock::RecomputeUnlocked : Lock::PermanentUnlocked);
    }
    if (m_beta_grad.empty()) {
        m_beta_grad = std::vector<float>(m_beta.size(), 0);
    }

    for (size_t i = 0; i != m_beta_lock.size(); ++i) {
        Lock &l = m_beta_lock.at(i);
        float g = m_beta_grad.at(i);
        switch (l) {
            case Lock::RecomputeUnlocked:
            case Lock::RecomputeLocked:
                if (abs(g) < m_lock_threshold) {
                    l = Lock::RecomputeUnlocked;
                } else {
                    l = Lock::RecomputeLocked;
                }
                break;
            case Lock::PermanentLocked:
            case Lock::PermanentUnlocked:
                break;
        }
    }
}

void twin::NetworkParametersImpl::PutZin(const torch::Tensor &zin) {
    std::lock_guard<std::mutex> lock(m_lock);
    torch::Tensor hzin = zin.to(torch::kCPU);
    std::vector<float> data(hzin.data_ptr<float>(), hzin.data_ptr<float>() + hzin.numel());
    std::copy(data.begin(), data.end(), m_vposer_repr.begin());
}

torch::Tensor twin::NetworkParametersImpl::GetZin() {
    std::lock_guard<std::mutex> lock(m_lock);
    torch::Tensor ret = torch::from_blob(const_cast<float *>(m_vposer_repr.data()), {1, long(m_vposer_repr.size())},
                                         torch::kFloat32);
    return ret;
}

void twin::NetworkParametersImpl::PutBeta(const torch::Tensor &beta) {
    std::lock_guard<std::mutex> lock(m_lock);
    torch::Tensor hbeta = beta.to(torch::kCPU);
    std::vector<float> data(hbeta.data_ptr<float>(), hbeta.data_ptr<float>() + hbeta.numel());
    std::copy(data.begin(), data.end(), m_beta.begin());
}

torch::Tensor twin::NetworkParametersImpl::GetBeta() {
    std::lock_guard lock(m_lock);
    auto ret = torch::from_blob(const_cast<float *>(m_beta.data()), {1, long(m_beta.size())}, torch::kFloat32);
    return ret;
}

void twin::NetworkParametersImpl::PutTrans(const torch::Tensor &trans) {
    std::lock_guard<std::mutex> lock(m_lock);
    auto htrans = trans.to(torch::kCPU);
    std::vector<float> data(htrans.data_ptr<float>(), htrans.data_ptr<float>() + htrans.numel());
    std::copy(data.begin(), data.end(), m_translation.data());
}

torch::Tensor twin::NetworkParametersImpl::GetTrans() {
    std::lock_guard<std::mutex> lock(m_lock);
    auto ret = torch::from_blob(const_cast<float *>(m_translation.data()), {1, long(3)}, torch::kFloat32);
    return ret;
}

void twin::NetworkParametersImpl::PutPose(const torch::Tensor &dpose) {
    std::lock_guard<std::mutex> lock(m_lock);
    torch::Tensor pose = dpose.to(torch::kCPU);
    std::vector<float> full_pose(pose.data_ptr<float>(), pose.data_ptr<float>() + pose.numel());
    std::vector<float> pb, ph, pe;
    int offset = 0;
    m_root_orient[0] = full_pose.at(0);
    m_root_orient[1] = full_pose.at(1);
    m_root_orient[2] = full_pose.at(2);
    offset += 3;
    for (int i = 0; i < BodyEntries(); ++i) {
        m_pose_body.at(i)[0] = full_pose.at(offset + i * 3 + 0);
        m_pose_body.at(i)[1] = full_pose.at(offset + i * 3 + 1);
        m_pose_body.at(i)[2] = full_pose.at(offset + i * 3 + 2);
    }
    offset += BodyEntries() * 3;
    for (int i = 0; i < JawEntries(); ++i) {
        m_pose_jaw.at(i)[0] = full_pose.at(offset + i * 3 + 0);
        m_pose_jaw.at(i)[1] = full_pose.at(offset + i * 3 + 1);
        m_pose_jaw.at(i)[2] = full_pose.at(offset + i * 3 + 2);
    }
    offset += 3 * JawEntries();
    for (int i = 0; i < EyeEntries(); ++i) {
        m_pose_eye.at(i)[0] = full_pose.at(offset + i * 3 + 0);
        m_pose_eye.at(i)[1] = full_pose.at(offset + i * 3 + 1);
        m_pose_eye.at(i)[2] = full_pose.at(offset + i * 3 + 2);
    }
    offset += 3 * EyeEntries();

    for (int i = 0; i < HandEntries(); ++i) {
        m_pose_hand.at(i)[0] = full_pose.at(offset + i * 3 + 0);
        m_pose_hand.at(i)[1] = full_pose.at(offset + i * 3 + 1);
        m_pose_hand.at(i)[2] = full_pose.at(offset + i * 3 + 2);
    }
    offset += HandEntries() * 3;
    for (int i = 0; i < FootEntries(); ++i) {
        m_pose_foot.at(i)[0] = full_pose.at(offset + i * 3 + 0);
        m_pose_foot.at(i)[1] = full_pose.at(offset + i * 3 + 1);
        m_pose_foot.at(i)[2] = full_pose.at(offset + i * 3 + 2);
    }
}

torch::Tensor twin::NetworkParametersImpl::GetPose() {
    std::vector<float> pb, ph, pf, pe, pj;

    std::lock_guard<std::mutex> lock(m_lock);
    for (cppgl::vec3 &a: m_pose_body) {
        pb.push_back(a[0]);
        pb.push_back(a[1]);
        pb.push_back(a[2]);
    }
    for (cppgl::vec3 &a: m_pose_hand) {
        ph.push_back(a[0]);
        ph.push_back(a[1]);
        ph.push_back(a[2]);
    }
    for (cppgl::vec3 &a: m_pose_foot) {
        pf.push_back(a[0]);
        pf.push_back(a[1]);
        pf.push_back(a[2]);
    }
    for (cppgl::vec3 &a: m_pose_jaw) {
        pj.push_back(a[0]);
        pj.push_back(a[1]);
        pj.push_back(a[2]);
    }
    for (cppgl::vec3 &a: m_pose_eye) {
        pe.push_back(a[0]);
        pe.push_back(a[1]);
        pe.push_back(a[2]);
    }
    torch::Tensor root_orie = torch::from_blob((float *) m_root_orient.data(), {1, 3});
    torch::Tensor pose_body = torch::from_blob(pb.data(), {1, (int) pb.size()});
    torch::Tensor pose_hand = torch::from_blob(ph.data(), {1, (int) ph.size()});
    torch::Tensor pose_foot = torch::from_blob(pf.data(), {1, (int) pf.size()});
    torch::Tensor pose_jaws = torch::from_blob(pj.data(), {1, (int) pj.size()});
    torch::Tensor pose_eyes = torch::from_blob(pe.data(), {1, (int) pe.size()});
    torch::Tensor full_pose = torch::cat(
            {
                    root_orie,
                    pose_body,
                    pose_jaws,
                    pose_eyes,
                    pose_hand,
                    pose_foot,
            },
            -1);
    return full_pose;
}

std::ostream &operator<<(std::ostream &os, const twin::NetworkParametersImpl &params) {
    auto print = [&os](const cppgl::vec3 &v) { os << v[0] << " " << v[1] << " " << v[2] << " "; };
    print(params.m_root_orient);
    print(params.m_translation);
    for (const cppgl::vec3 &v: params.m_pose_body) print(v);
    for (const cppgl::vec3 &v: params.m_pose_hand) print(v);
    for (const cppgl::vec3 &v: params.m_pose_foot) print(v);
    for (const cppgl::vec3 &v: params.m_pose_eye) print(v);
    for (const cppgl::vec3 &v: params.m_pose_jaw) print(v);
    for (const float &v: params.m_beta) os << v << " ";
    for (const float &v: params.m_expression) os << v << " ";
    for (const float &v: params.m_dmpls) os << v << " ";
    return os;
}

std::istream &operator>>(std::istream &is, twin::NetworkParametersImpl &params) {
    auto read = [&is](cppgl::vec3 &v) { is >> v[0] >> v[1] >> v[2]; };
    read(params.m_root_orient);
    read(params.m_translation);
    for (cppgl::vec3 &v: params.m_pose_body) read(v);
    for (cppgl::vec3 &v: params.m_pose_hand) read(v);
    for (cppgl::vec3 &v: params.m_pose_foot) read(v);
    for (cppgl::vec3 &v: params.m_pose_eye) read(v);
    for (cppgl::vec3 &v: params.m_pose_jaw) read(v);

    float val;

    params.m_beta.resize(0);
    params.m_expression.resize(0);
    params.m_dmpls.resize(0);

    std::vector<int> test;
    while (is >> val) {
        if (params.m_beta.size() < static_cast<unsigned long int>(params.BetaEntries()))
            params.m_beta.push_back(val);
        else if (params.m_expression.size() < static_cast<unsigned long int>(params.ExpressionEntries()))
            params.m_expression.push_back(val);
        else if (params.m_dmpls.size() < static_cast<unsigned long int>(params.DMPLEntries()))
            params.m_dmpls.push_back(val);
    }
    if (params.m_beta.size() != static_cast<unsigned long int>(params.BetaEntries()))
        params.m_beta.resize(params.BetaEntries());
    if (params.m_expression.size() != static_cast<unsigned long int>(params.ExpressionEntries()))
        params.m_expression.resize(params.ExpressionEntries());
    if (params.m_dmpls.size() != static_cast<unsigned long int>(params.DMPLEntries()))
        params.m_dmpls.resize(params.DMPLEntries());
    return is;
}

void twin::NetworkParametersImpl::Deserialize(std::istream &os) {
    os >> *this;
}

void twin::NetworkParametersImpl::Serialize(std::ostream &os) const {
    os << *this;
}


bool twin::NetworkParametersImpl::drawGUI() {
    bool ret = false;
    ImGui::PushID(m_id);


    auto clear = [](auto &vec, auto zero) {
        auto size = vec.size();
        vec.clear();
        vec.resize(size, zero);
    };
    ImGui::Indent(2);
    ImGui::Indent(2);
    SliderReset("Translation", m_translation.data(), -5, 5, ret);
    SliderReset("Root Rotation", m_root_orient.data(), -M_PI, M_PI, ret);
    ImGui::Unindent(2);
    if (HasBody() && ImGui::CollapsingHeader("PoseBody##")) {
        ImGui::Indent(2);
        ImGui::PushID("Body");
        std::vector<std::string> joints = JointNames();
        if (ImGui::Button("Reset##")) {
            clear(m_pose_body, cppgl::vec3::Zero());
            ret = true;
        }
        for (int i = 0; i < BodyEntries(); ++i) {
            ImGui::PushID(i);
            SliderReset(joints.at(i), m_pose_body.at(i).data(), -M_PI, M_PI, ret);
            ImGui::PopID();
        }
        ImGui::PopID();
        ImGui::Unindent(2);
    }
    if (HasHand() && ImGui::CollapsingHeader("PoseHand##")) {
        ImGui::Indent(2);
        ImGui::PushID("Hand");
        if (ImGui::Button("Reset All##")) {
            clear(m_pose_hand, cppgl::vec3::Zero());
            ret = true;
        }
        std::vector<std::string> joints_hand = HandJointNames();
        for (int i = 0; i < HandEntries(); ++i) {
            ImGui::PushID(i);
            SliderReset(joints_hand.at(i), m_pose_hand.at(i).data(), -M_PI, M_PI, ret);
            ImGui::PopID();
        }
        ImGui::PopID();
        ImGui::Unindent(2);
    }
    if (HasFoot() && ImGui::CollapsingHeader("PoseFoot##")) {
        ImGui::Indent(2);
        ImGui::PushID("Foot");
        if (ImGui::Button("Reset All##")) {
            clear(m_pose_foot, cppgl::vec3::Zero());
            ret = true;
        }
        std::vector<std::string> joints_foot = FootJointNames();
        for (int i = 0; i < FootEntries(); ++i) {
            ImGui::PushID(i);
            SliderReset(joints_foot.at(i), m_pose_foot.at(i).data(), -M_PI, M_PI, ret);
            ImGui::PopID();
        }
        ImGui::PopID();
        ImGui::Unindent(2);
    }
    if (HasEyes() && ImGui::CollapsingHeader("PoseEyes##")) {
        assert(m_pose_eye.size() >= 2);
        ImGui::Indent(2);
        SliderReset("PoseEye_l", m_pose_eye.at(0).data(), -M_PI, M_PI, ret);
        SliderReset("PoseEye_r", m_pose_eye.at(1).data(), -M_PI, M_PI, ret);
        ImGui::Unindent(2);
    }
    if (HasJaw() && ImGui::CollapsingHeader("PoseJaw##")) {
        assert(m_pose_jaw.size() >= 1);
        ImGui::Indent(2);
        SliderReset("PoseJaw", m_pose_jaw.at(0).data(), -M_PI, M_PI, ret);
        ImGui::Unindent(2);
    }

    if (ImGui::CollapsingHeader("Beta##")) {
        ImGui::Indent(2);
        ImGui::PushID("Beta");
        if (ImGui::Button("Reset All##")) {
            clear(m_beta, 0);
            ret = true;
        }
        UpdateLock();
        if (ImGui::Checkbox(_labelPrefix("Recompute").c_str(), &m_recompute)) {
            for (int i = 0; i < BetaEntries(); ++i) {
                bool recompute = m_recompute;
                bool locked = m_beta_lock[i] & (0b01);//If Locked true else false
                m_beta_lock[i] = Lock(int(!recompute) << 1 | int(locked));//Use recompute and locked to calculate the right Lock enum value
            }
        }
        ImGui::InputFloat(_labelPrefix("LockThreshold").c_str(), &m_lock_threshold);
        int gsize = static_cast<int>(m_beta_grad.size());
        for (int i = 0; i < BetaEntries(); ++i) {
            ImGui::PushID(i);
            bool has_grad = i < gsize;
            float f[2] = {m_beta.at(i), has_grad ? m_beta_grad.at(i) : 0};
            bool recompute = !(m_beta_lock.at(i) & (0b10));
            bool locked = m_beta_lock.at(i) & (0b01);
            if (m_recompute) {
                ImGui::Checkbox("##", &recompute);
                ImGui::SameLine();
            }
            ImGui::PushID(0);
            ImGui::Checkbox("##", &locked);
            m_beta_lock.at(i) = Lock(int(!recompute) << 1 | int(locked));

            ImGui::PushID(1);
            ImGui::SameLine();
            if (locked) {
                ImGui::Text("\tBeta: %f\t\tBeta': %f\t", f[0], f[1]);
            } else {
                if (ImGui::SliderFloat2("Beta, Beta'##", f, -5, 5)) ret = true;
            }
            m_beta.at(i) = f[0];
            if (has_grad) m_beta_grad.at(i) = f[1];
            ImGui::PopID();
            ImGui::PopID();
            ImGui::PopID();
        }
        ImGui::PopID();
        ImGui::Unindent(2);
    }
    if (ImGui::CollapsingHeader("Zin")) {
        ImGui::Indent(2);
        ImGui::PushID("Zin");
        ImGui::Checkbox(_labelPrefix("use for inference").c_str(), &m_use_vposer);

        if (ImGui::Button("Reset All##")) {
            std::fill_n(m_vposer_repr.begin(), m_vposer_repr.size(), 0.f);
            ret = true;
        }
        for (int i = 0; i < static_cast<int>(m_vposer_repr.size()); ++i) {
            ImGui::PushID(i);
            if (ImGui::SliderFloat("##", &m_vposer_repr.at(i), -5, 5)) ret = true;
            ImGui::PopID();
        }
        ImGui::PopID();
        ImGui::Unindent(2);
    }

    if (ImGui::CollapsingHeader("Expression##")) {
        ImGui::PushID("Expression");
        for (int i = 0; i < ExpressionEntries(); ++i) {
            ImGui::PushID(i);
            if (ImGui::SliderFloat("##", &m_expression.at(i), -5, 5)) ret = true;
            ImGui::PopID();
        }
        ImGui::PopID();
    }
    ImGui::Unindent(2);
    ImGui::PopID();
    return ret;
}

void twin::NetworkParametersImpl::setType(const twin::ModelType &t) {
    m_type = t;
}

void twin::NetworkParametersImpl::setConstrained(bool constrained) {
    m_constrained = constrained;
}

#endif