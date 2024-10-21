#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_NETPARAMS_H
#define TWIN_NETPARAMS_H

#include "twinDefines.h"
#include <cppgl.h>
#include <fstream>
#include <torch/types.h>

TWIN_NAMESPACE_BEGIN
    enum ModelType : int {
        human,
        hand,
        head
    };

    class NetworkParametersImpl {
    public:
        std::string name;

        NetworkParametersImpl();

        explicit NetworkParametersImpl(std::string cname, const ModelType &t = ModelType::human, bool constrained = false);

        void init();

        virtual ~NetworkParametersImpl() = default;

        bool drawGUI();

        virtual void Serialize(std::ostream &os) const;

        virtual void Deserialize(std::istream &os);

        virtual void CopyCommon(cppgl::NamedHandle<NetworkParametersImpl> &params);

        //Return the content of m_pose_body, m_pose_hand, m_pose_foot, m_pose_eye, m_pose_jaw, m_rotation as torch tensor resembling the full pose
        virtual torch::Tensor GetPose();

        //Copies the pose parameters from wherever to cpu into m_pose_body, m_pose_hand, m_pose_foot, m_pose_eye, m_pose_jaw, m_rotation
        virtual void PutPose(const torch::Tensor &dpose);

        //Return the content of m_translation as torch tensor
        virtual torch::Tensor GetTrans();

        //Copies the translation from wherever to cpu into m_translation
        virtual void PutTrans(const torch::Tensor &trans);

        //Return the content of m_beta as torch tensor
        virtual torch::Tensor GetBeta();

        //Copies the beta parameter from wherever to cpu into m_beta
        virtual void PutBeta(const torch::Tensor &beta);

        //Return the content of m_vposer_repr as torch tensor
        torch::Tensor GetZin();

        //Copies the vposer parameter from wherever to cpu into m_vposer_repr
        void PutZin(const torch::Tensor &zin);


        cppgl::vec3 m_root_orient;
        cppgl::vec3 m_translation;
        std::vector<cppgl::vec3> m_pose_body, m_pose_hand, m_pose_foot, m_pose_eye, m_pose_jaw;

        std::vector<float> m_vposer_repr;
        bool m_use_vposer = false;

        std::vector<float> m_beta;
        std::vector<float> m_beta_grad;

        enum Lock {
            RecomputeUnlocked = 0, RecomputeLocked = 1, PermanentUnlocked = 2, PermanentLocked = 3
        };
        bool m_recompute;
        std::vector<Lock> m_beta_lock;
        float m_lock_threshold;

        std::vector<float> m_expression;
        std::vector<float> m_dmpls;

        float m_pose_loss;
        float m_loss;

        static inline int id_counter = 0;
        int m_id;


        virtual bool HasBody() const = 0;

        virtual bool HasHand() const = 0;

        virtual bool HasFoot() const;

        virtual bool HasEyes() const = 0;

        virtual bool HasJaw() const = 0;

        virtual bool HasDMPL() const = 0;

        virtual bool HasExpression() const = 0;

        virtual int BodyEntries() const = 0;

        virtual int HandEntries() const = 0;

        virtual int FootEntries() const = 0;

        virtual int EyeEntries() const = 0;

        virtual int JawEntries() const = 0;

        virtual int BetaEntries() const = 0;

        virtual int DMPLEntries() const = 0;

        virtual int ExpressionEntries() const = 0;

        virtual void setType(const ModelType &t);

        virtual void setConstrained(bool constrained);

    protected:
        virtual std::vector<std::string> JointNames() const = 0;

        virtual std::vector<std::string> HandJointNames() const = 0;

        virtual std::vector<std::string> FootJointNames() const;

        void UpdateLock();

        std::mutex m_lock;

        ModelType m_type;

        bool m_constrained;
    };

    using NetworkParameters = cppgl::NamedHandle<NetworkParametersImpl>;


TWIN_NAMESPACE_END

#endif //TWIN_NETPARAMS_H
#endif