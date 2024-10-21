
#pragma once
#ifdef HAS_DL_MODULE

#ifndef TWIN_NETUTIL_H
#define TWIN_NETUTIL_H

#include "twinDefines.h"
#include "cnpy.h"
#include <torch/types.h>
#include <nlohmann/json.hpp>

TWIN_NAMESPACE_BEGIN
    //converts numpy data file into at::tensors for torch
    template<typename T>
    static torch::Tensor totorch(cnpy::npz_t &n, const std::string &s, torch::Device d, torch::ScalarType type, torch::ScalarType type2) {
        cnpy::NpyArray v = n[s];
        std::vector<int64_t> shape;
        for (unsigned long i: v.shape) shape.push_back(static_cast<long>(i));
        T *dat = v.data<T>();
        torch::Tensor ret = torch::from_blob(dat, shape, type).to(type2).to(d).detach().clone();
        return ret;
    }

    //converts json data file into at::tensors for torch
    template<typename T>
    static torch::Tensor totorch(nlohmann::json &j, const std::string &s, torch::Device d, torch::ScalarType type, torch::ScalarType type2) {
        nlohmann::json jdata = j[s];
        std::vector<int64_t> shape = jdata["shape"].get<std::vector<int64_t>>();
        std::vector<T> data = jdata["data"].get<std::vector<T>>();
        T *dat = data.data();
        torch::Tensor ret = torch::from_blob(dat, shape, type).to(type2).to(d).detach().clone();
        return ret;
    }

    torch::Tensor toTorchFloating(cnpy::npz_t &n, const std::string &s, torch::Device d);

    torch::Tensor toTorchInt(cnpy::npz_t &n, const std::string &s, torch::Device d, torch::ScalarType type);
TWIN_NAMESPACE_END
#endif //TWIN_NETUTIL_H
#endif