#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_SUPR_H
#define TWIN_SUPR_H

#include "twinDefines.h"
#include "net/network.h"
#include <torch/types.h>
#include <cppgl.h>
#include "net/supr/suprparams.h"
#include "suprcpp.h"

TWIN_NAMESPACE_BEGIN


    class SUPRImpl : public NetworkImpl {
    public:
        SUPRImpl(const std::string &cname, const std::string &filename, Gender s);

        ~SUPRImpl();


        std::tuple<torch::Tensor, torch::Tensor, torch::Tensor> VPosed(torch::Tensor full_pose, torch::Tensor trans, torch::Tensor beta) override;

        torch::Device GetHost() const;

        torch::Device GetDevice() const;

        torch::Tensor VTemplate() override;

        torch::Tensor Shapedirs();

        torch::Tensor Faces() override;

        torch::Tensor JReg();

        torch::Tensor Kintree() override;

        torch::Tensor Weights();

        torch::Tensor Posedirs();

        std::vector<long> Parents() override;

        void Inference();

        void Inference(NetworkParameters &p);

        std::string m_filename;

        void drawGUI() override;

        NetworkParameters params;//This must be a SuprParameter internally

        void LoadGender(const std::string &path, Gender s) override;

        void LoadGender(const std::string &path, Gender s, bool forceUnconstrained);

        static void drawClassGUI();

        bool VposerPossible() const override;

        void to(const torch::Device &d);

    private:
        int m_num_verts;
        bool m_constrained;
        bool m_constposs;
        int m_numpose;

        std::shared_ptr<SUPRCPP> sp;
        torch::OrderedDict<std::string, torch::Tensor> m_bufs;

        static bool c_gui;
    };

    using SUPR = cppgl::NamedHandle<SUPRImpl>;

TWIN_NAMESPACE_END
#endif //TWIN_SUPR_H

#endif