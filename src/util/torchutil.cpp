#ifdef HAS_DL_MODULE

#include "util/torchutil.h"


torch::Tensor twin::toTorchFloating(cnpy::npz_t &n, const std::string &s, torch::Device d) {
    if (n[s].word_size == 4) {
        return totorch<float>(n, s, d, torch::kFloat32, torch::kFloat32);
    } else {
        return totorch<double>(n, s, d, torch::kFloat64, torch::kFloat32);
    }
}

torch::Tensor twin::toTorchInt(cnpy::npz_t &n, const std::string &s, torch::Device d, torch::ScalarType type) {
    if (n[s].word_size == 4) {
        return totorch<int32_t>(n, s, d, torch::kInt32, type);
    } else {
        return totorch<int64_t>(n, s, d, torch::kInt64, type);
    }
}

#endif