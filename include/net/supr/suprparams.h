#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_SUPRPARAMS_H
#define TWIN_SUPRPARAMS_H

#include "twinDefines.h"
#include "net/netParams.h"

TWIN_NAMESPACE_BEGIN

    class SuprParametersImpl : public NetworkParametersImpl {
    public:
        explicit SuprParametersImpl(const std::string &cname, const ModelType &t = ModelType::human, bool constrained = false);

        bool HasBody() const override;

        bool HasHand() const override;

        bool HasFoot() const override;

        bool HasEyes() const override;

        bool HasJaw() const override;

        bool HasDMPL() const override;

        bool HasExpression() const override;

        int BodyEntries() const override;

        int HandEntries() const override;

        int FootEntries() const override;

        int EyeEntries() const override;

        int JawEntries() const override;

        int BetaEntries() const override;

        int DMPLEntries() const override;

        int ExpressionEntries() const override;

    protected:
        std::vector<std::string> JointNames() const override;

        std::vector<std::string> HandJointNames() const override;

        std::vector<std::string> FootJointNames() const override;
    };

    using SuprParameters = cppgl::NamedHandle<SuprParametersImpl>;

TWIN_NAMESPACE_END

#endif //TWIN_SUPRPARAMS_H
#endif