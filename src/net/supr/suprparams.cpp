#ifdef HAS_DL_MODULE

#include "net/supr/suprparams.h"

twin::SuprParametersImpl::SuprParametersImpl(const std::string &cname, const ModelType &t, bool c) : NetworkParametersImpl(cname, t, c) {
    init();
}

bool twin::SuprParametersImpl::HasBody() const {
    if (m_type == ModelType::human) {
        return true;
    }
    return false;
}

bool twin::SuprParametersImpl::HasHand() const {
    if (m_type == ModelType::head) {
        return false;
    }
    return true;
}

bool twin::SuprParametersImpl::HasFoot() const {
    if (m_type == ModelType::human) {
        return true;
    }
    return false;
}

bool twin::SuprParametersImpl::HasEyes() const {
    if (m_type == ModelType::hand) {
        return false;
    }
    return true;
}

bool twin::SuprParametersImpl::HasJaw() const {
    if (m_type == ModelType::hand || m_constrained) {
        return false;
    }
    return true;
}

bool twin::SuprParametersImpl::HasDMPL() const {
    return false;
}

bool twin::SuprParametersImpl::HasExpression() const {
    return false;
}

int twin::SuprParametersImpl::BodyEntries() const {
    if (m_type == ModelType::human) {
        if (m_constrained)
            return 19;
        return 21;
    }
    return 0;
}

int twin::SuprParametersImpl::HandEntries() const {
    switch (m_type) {
        case ModelType::human:
            if (m_constrained)
                return 14;
            return 30;
        case ModelType::hand:
            if (m_constrained)
                return 8;
            return 16;
        case ModelType::head:
        default:
            return 0;
    }
}

int twin::SuprParametersImpl::FootEntries() const {
    if (m_type == ModelType::human) {
        return 20;
    }
    return 0;
}

int twin::SuprParametersImpl::EyeEntries() const {
    if (m_type == ModelType::hand) {
        return 0;
    }
    return 2;
}

int twin::SuprParametersImpl::JawEntries() const {
    if (m_type == ModelType::hand || m_constrained) {
        return 0;
    }
    return 1;
}

int twin::SuprParametersImpl::BetaEntries() const {
    return 50;
}

int twin::SuprParametersImpl::DMPLEntries() const {
    return 0;
}

int twin::SuprParametersImpl::ExpressionEntries() const {
    return 0;
}

std::vector<std::string> twin::SuprParametersImpl::JointNames() const {
    if (m_constrained) {
        return {"hip_l", "hip_r", "back", "knee_l/r/back2", "back2/subtalar_l", "subtalar_l/r", "subtalar_r/back3", "back3/mtp_l", "mtp_l/r", "mtp_r/neck", "neck/shoulder_l1", "shoulder_l1/r1", "shoulder_r1/neck2", "neck2/shoulder_l2", "shoulder_l2/r2", "shoulder_r2/elbow_l", "elbow_r/wrist_l", "wrist_l/r", "wrist_r/jaw"};
    }
    return {"hip_l", "hip_r", "back", "knee_l", "knee_r", "back2", "subtalar_l", "subtalar_r", "back3", "mtp_l", "mtp_r", "neck", "shoulder_l1", "shoulder_r1", "neck2", "shoulder_l2", "shoulder_r2", "elbow_l", "elbow_r", "wrist_l", "wrist_r"};
}

std::vector<std::string> twin::SuprParametersImpl::HandJointNames() const {
    if (m_type == ModelType::hand) {
        if (m_constrained) {
            return {"wrist", "index1/2", "index3/middle1", "middle2/3/pinky1", "pinky1/2/3", "ring1/2", "ring3/thumb1", "thumb1/2/3"};
        }
        return {"wrist", "index1", "index2", "index3", "middle1", "middle2", "middle3", "pinky1", "pinky2", "pinky3", "ring1", "ring2", "ring3", "thumb1", "thumb2", "thumb3"};
    }
    if (m_constrained) {
        return {"left_index1/2", "left_index3/middle1", "left_middle2/3/pinky1", "left_pinky1/2/3", "left_ring1/2", "left_ring3/thumb1", "left_thumb1/2/3", "right_index1/2", "right_index3/middle1", "right_middle2/3/pinky1", "right_pinky1/2/3", "right_ring1/2", "right_ring3/thumb1", "right_thumb1/2/3"};
    }
    return {"left_index1", "left_index2", "left_index3", "left_middle1", "left_middle2", "left_middle3", "left_pinky1", "left_pinky2", "left_pinky3", "left_ring1", "left_ring2", "left_ring3", "left_thumb1", "left_thumb2", "left_thumb3", "right_index1", "right_index2", "right_index3", "right_middle1", "right_middle2", "right_middle3", "right_pinky1", "right_pinky2", "right_pinky3", "right_ring1", "right_ring2", "right_ring3", "right_thumb1", "right_thumb2", "right_thumb3"};
}

std::vector<std::string> twin::SuprParametersImpl::FootJointNames() const {
    return {"left_foot_1", "left_foot_2", "left_foot_3", "left_foot_4", "left_foot_5", "left_foot_6", "left_foot_7", "left_foot_8", "left_foot_9", "left_foot_10", "right_foot_1", "right_foot_2", "right_foot_3", "right_foot_4", "right_foot_5", "right_foot_6", "right_foot_7", "right_foot_8", "right_foot_9", "right_foot_10",};
}

#endif