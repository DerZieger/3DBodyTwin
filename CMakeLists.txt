cmake_minimum_required(VERSION 3.16)

PROJECT(3DTwin
    VERSION 0.1
    DESCRIPTION "Optimizer and visualizer for 3D Bodytwin."
    LANGUAGES CXX CUDA
)

if (NOT UNIX)
	message(FATAL_ERROR "Unsupported platform!")
endif()

# ---------------------------------------------------------------------
# Options

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")
option(INSTALL_MISSING_REQUIRED_DEPENDENCIES "Automatically download and install missing dependencies that are required" ON)
option(DL_MODULE "Whether to build deep learning modules or not" OFF)



include(helper_macros)
# ---------------------------------------------------------------------
# path management

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")


# ---------------------------------------------------------------------
# external content

#Eigen and stuff
include(Dependencies_core)
#Cppgl
add_subdirectory(external/cppgl)
add_subdirectory(external/suprcpp)
add_subdirectory(external/vposercpp)
add_subdirectory(external/iir1)
#Deep Learning amd stuff
if(DL_MODULE)
	include(Dependencies_dl)
endif()

# ---------------------------------------------------------------------
# compiler options

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_program(CCACHE_FOUND ccache)

if(CCACHE_FOUND)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
	set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

# setup for included dependencies
set(CMAKE_INCLUDE_DIRECTORIES_BEFORE ON)
set(CMAKE_LINK_DIRECTORIES_BEFORE ON)

if(UNIX)
	# Compiler flags:
	# -03 											More aggressive than -O2 with longer compile times.
	# Recommended for codes that loops involving intensive floating point calculations.
	# -funroll-loops 						Unroll loops whose number of iterations can be determined at compile time or upon entry to the loop
	# -g												Generate debugging information
	# -pthread									Enable multithreading
	# -Wall											Enables warnings, must be enabled before compiler flags that disable some warnings again
	# -Wno-unused-parameter			Disables warnings for unused parameters
	# -Wno-unused-variable			Disables warnings for unused variables
	# -Wno-int-to-pointer-cast 	Suppress warnings from casts to pointer type of an integer of a different size.
	# -march=native							Allows use of instructions that require the listed architecture.
	# if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
	# else()
	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
	# endif()
	if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
		# if you want to have .profraw files use this flag: -fprofile-generate
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -v -fvectorize")

		# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --cuda-path=/usr/local/cuda --cuda-gpu-arch=sm_86")
	endif()

	set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -O3 -Wno-unused-parameter -Wno-unused-variable -Wno-int-to-pointer-cast -march=native -funroll-loops -finline-functions")
	set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-deprecated-declarations -pthread")

	# linker flags
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ffunction-sections -fdata-sections")

	# position independent code is required for shared libs. For GCC this is -fPIC
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
else()
	# Windows stuff
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES -D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING /bigobj")
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
endif()


include(GenerateExportHeader)
# #####################################################################
# ######################## ADDITIONAL STUFF ###########################
# #####################################################################

# Let's nicely support folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# ---------------------------------------------------------------------
# traverse source tree

add_subdirectory(src)
# #####################################################################
# ######################## VERBOSE OUTPUT ############################
# #####################################################################
message(STATUS "\nCompiler Flags:")

message(STATUS "PROJECT_CXX_FLAGS: ${PROJECT_CXX_FLAGS}")
message(STATUS "PROJECT_PRIVATE_CXX_FLAGS: ${PROJECT_PRIVATE_CXX_FLAGS}")
message(STATUS "PROJECT_LD_FLAGS: ${PROJECT_LD_FLAGS}")

message(STATUS "CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "CMAKE_CXX_FLAGS_DEBUG: ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "CMAKE_CXX_FLAGS_RELWITHDEBINFO: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "CMAKE_CXX_FLAGS_RELEASE: ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")

if(CUDA_FOUND)
	message(STATUS "\nCUDA Compiler Flags:")
	message(STATUS "CMAKE_CUDA_FLAGS: ${CMAKE_CUDA_FLAGS}")
	message(STATUS "CMAKE_CUDA_FLAGS_DEBUG: ${CMAKE_CUDA_FLAGS_DEBUG}")
	message(STATUS "CMAKE_CUDA_FLAGS_RELWITHDEBINFO: ${CMAKE_CUDA_FLAGS_RELWITHDEBINFO}")
	message(STATUS "CMAKE_CUDA_FLAGS_RELEASE: ${CMAKE_CUDA_FLAGS_RELEASE}")
	message(STATUS "CMAKE_CUDA_STANDARD: ${CMAKE_CUDA_STANDARD}")
endif()
