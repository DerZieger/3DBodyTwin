#ifdef HAS_DL_MODULE

#include "net/vposer.h"

#include "cnpy.h"
#include "torch/script.h"


twin::VPoserImpl::VPoserImpl(std::string name, const std::string &path, int num_joints, int repr) : rotrepr(repr), n_features(rotrepr * num_joints), name(std::move(name)) {
    vp = std::make_shared<VPoserCPP>(path, num_joints, repr);
}

NormalDistribution twin::VPoserImpl::Encode(const torch::Tensor &body_pose, bool has_root_orient) {


    using namespace torch::indexing;

    torch::Tensor main = body_pose.index({Slice(), (has_root_orient ? Slice(rotrepr, n_features + rotrepr) : Slice(None, n_features))});//Shift in tensor to ignore root orientation

    return vp->Encode(main);

}

torch::Tensor twin::VPoserImpl::Decode(const torch::Tensor &zin) {
    torch::Tensor rotmat = vp->Decode(zin)["pose_body"].reshape({-1, n_features});
    return rotmat;
}


torch::Tensor
twin::VPoserImpl::Decode(const torch::Tensor &zin, const torch::Tensor &body_pose, bool is_hand, bool has_root_orient) {
    if(is_hand)
        return body_pose;
    using namespace torch::indexing;
    torch::Tensor main = this->Decode(zin);

    if (has_root_orient) {
        torch::Tensor root_orient = body_pose.index({Slice(), Slice(None, rotrepr)});
        torch::Tensor rest = body_pose.index({Slice(), Slice(n_features + rotrepr, None)});
        torch::Tensor ret = torch::cat({root_orient, main, rest}, 1);
        return ret;
    } else {
        torch::Tensor rest = body_pose.index({Slice(), Slice(n_features, None)});
        torch::Tensor ret = torch::cat({main, rest}, 1);
        std::cout<<ret<<std::endl;
        return ret;
    }
}

void twin::VPoserImpl::to(const torch::Device &d) {
    vp->to(d);
}

void twin::VPoserImpl::zero_grad() {
    vp->zero_grad();
}

#endif