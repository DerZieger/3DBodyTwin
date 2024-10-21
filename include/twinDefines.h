#pragma once

#ifndef TWIN_NAMESPACE_BEGIN
#define TWIN_NAMESPACE_BEGIN \
    namespace twin         \
    {
#endif

#ifndef TWIN_NAMESPACE_END
#define TWIN_NAMESPACE_END }
#endif

#ifndef HD
#ifdef HAS_DL_MODULE
#define HD __host__ __device__
#else
#define HD
#endif
#endif