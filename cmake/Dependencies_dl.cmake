# Defines the following output variables:
#
# DL_INCLUDES:    The list of required include directories
# DL_LIBS:        The list of required libraries for link_target
# DL_TARGETS:     The list of required targets
# MODULE_DL:      True if all required dependencies are found.
#

unset(PACKAGE_INCLUDES)
unset(LIB_TARGETS)
unset(LIBS)
unset(MODULE_DL)

set(DEBUG_INSTALLATION ON)
set(FORCE_MANUAL_INSTALLATION OFF)
set(BUILD_SHARED_LIBS ON)

include(FetchContent)

# allow no updates -> faster
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)


# # -----------------------------------------------------
# # -----------------------------------------------------
option(CAFFE2_USE_CUDNN "Whether torch should use cudnn" ON)
##Torch
#execute_process(COMMAND python -c "import torch;print(torch.utils.cmake_prefix_path)" OUTPUT_VARIABLE TORCHPREFIX OUTPUT_STRIP_TRAILING_WHITESPACE)
#list(APPEND CMAKE_PREFIX_PATH "${TORCHPREFIX}")
find_package(Torch REQUIRED)
#LibraryHelper(TORCH_LIBRARIES libnvToolsExt.so "${CUDA_TOOLKIT_ROOT_DIR}/lib" Torch_FOUND)
#list(REMOVE_ITEM TORCH_LIBRARIES LIBNVTOOLSEXT-NOTFOUND)
#message(STATUS "Libstorch: ${TORCH_LIBRARIES}")
#PackageHelper(Torch "${Torch_FOUND}" "${TORCH_INCLUDE_DIRS}" "${TORCH_LIBRARIES}")
PackageHelperTarget(torch TORCH_FOUND)
#message(STATUS "TORCH_FLAGS: ${TORCH_CXX_FLAGS}, LIBS: ${TORCH_LIBRARIES}, INCLUDES: ${TORCH_INCLUDES}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TORCH_CXX_FLAGS} -frecord-gcc-switches")
set(LIBS ${LIBS} ${TORCH_LIBRARIES})

# # -----------------------------------------------------
# # -----------------------------------------------------

# # CUDA
find_package(CUDA 11.8 REQUIRED)

#(CUDA_LIBRARIES ${CUDA_LIBRARIES} cuda cublas)
set(CUDA_LIBRARIES ${CUDA_LIBRARIES} cuda)
PackageHelper(CUDA "${CUDA_FOUND}" "${CUDA_INCLUDE_DIRS}" "${CUDA_LIBRARIES}")

OPTION(CUDA_PROFILING "Enables the PROFILING #define" OFF)
OPTION(CUDA_DEBUG "Enables CUDA Debugging in Nsight" OFF)
OPTION(FULL_SPEED "Enables All Optimizations" OFF)


if (CUDA_PROFILING)
    add_definitions(-DCUDA_PROFILING)
    SET(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -lineinfo")
    SET(CMAKE_BUILD_TYPE RelWithDebInfo)
endif ()

if (CUDA_DEBUG)
    SET(CMAKE_CUDA_FLAGS "${CMAKE_CUDA_FLAGS} -G")
    SET(CMAKE_BUILD_TYPE DEB1G)
    add_definitions(-DCUDA_DEBUG)
else ()
    add_definitions(-DCUDA_NDEBUG)
endif ()


if (NOT MSVC)
    list(APPEND CMAKE_CUDA_FLAGS "-Xcompiler=-fopenmp")
    list(APPEND CMAKE_CUDA_FLAGS "-Xcompiler=-march=native")
endif ()

list(APPEND CMAKE_CUDA_FLAGS "-Xcudafe=--diag_suppress=field_without_dll_interface")
list(APPEND CMAKE_CUDA_FLAGS "-Xcudafe=--diag_suppress=base_class_has_different_dll_interface")
list(APPEND CMAKE_CUDA_FLAGS "-Xcudafe=--diag_suppress=dll_interface_conflict_none_assumed")
list(APPEND CMAKE_CUDA_FLAGS "-Xcudafe=--diag_suppress=dll_interface_conflict_dllexport_assumed")


list(APPEND CMAKE_CUDA_FLAGS "-use_fast_math")
list(APPEND CMAKE_CUDA_FLAGS "--expt-relaxed-constexpr")
list(APPEND CMAKE_CUDA_FLAGS "-Xcudafe=--diag_suppress=esa_on_defaulted_function_ignored")


# 30 GTX 7xx
# 52 GTX 9xx
# 61 GTX 10xx
# 75 RTX 20xx
# 86 RTX 30xx
#    if (${CUDA_VERSION} VERSION_LESS "11")
#        set(CMAKE_CUDA_ARCH "30-virtual" "52-virtual" CACHE STRING "The cuda architecture used for compiling .cu files")
#    else ()
#        # CUDA 11 and later doesn't support 30 anymore
#        set(CMAKE_CUDA_ARCH "52-virtual" "75-virtual" CACHE STRING "The cuda architecture used for compiling .cu files")
#    endif ()

message(STATUS "CUDA FLAGS: ${CMAKE_CUDA_FLAGS}")


#list(APPEND CMAKE_PREFIX_PATH ${CUDA_TOOLKIT_ROOT_DIR}/lib)

# # -----------------------------------------------------
# # -----------------------------------------------------

set(DL_INCLUDES ${PACKAGE_INCLUDES})
set(DL_LIBS ${LIBS})
set(DL_TARGETS ${LIB_TARGETS})
set(MODULE_DL 1)
