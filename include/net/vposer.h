
#pragma once
#ifdef HAS_DL_MODULE
#ifndef TWIN_VPOSER_H
#define TWIN_VPOSER_H

#include "twinDefines.h"
#include <torch/cuda.h>
#include <torch/types.h>
#include "named_handle.h"
#include <torch/nn.h>
#include <vposercpp.h>


TWIN_NAMESPACE_BEGIN
    struct VPoserImpl{
    public:
        VPoserImpl(std::string name,const std::string &path, int num_joints = 21, int repr = 3);

        NormalDistribution Encode(const torch::Tensor &body_pose, bool has_root_orient = true);

        torch::Tensor Decode(const torch::Tensor &zin);

        torch::Tensor Decode(const torch::Tensor &zin, const torch::Tensor &body_pose, bool is_hand=false, bool has_root_orient = true);

        void to(const torch::Device &d);

        void zero_grad();

        std::shared_ptr<VPoserCPP> vp;
        int  rotrepr, n_features;
        std::string name;//Needs a name to use namedhandle
    };

    using VPoser = cppgl::NamedHandle<VPoserImpl>;
TWIN_NAMESPACE_END
#endif //TWIN_VPOSER_H
#endif