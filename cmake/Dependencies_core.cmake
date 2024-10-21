# Defines the following output variables:
#
# CORE_INCLUDES:    The list of required include directories
# CORE_LIBS:        The list of required libraries for link_target
# CORE_TARGETS:     The list of required targets
# MODULE_CORE:      True if all required dependencies are found.
#

unset(PACKAGE_INCLUDES)
unset(LIB_TARGETS)
unset(LIBS)
unset(MODULE_CORE)

set(DEBUG_INSTALLATION ON)
set(FORCE_MANUAL_INSTALLATION OFF)
set(BUILD_SHARED_LIBS ON)

include(FetchContent)

# allow no updates -> faster
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# Git submodule auto update
# https://cliutils.gitlab.io/modern-cmake/chapters/projects/submodule.html
#find_package(Git QUIET)
#
#
#if(Git_FOUND AND NOT SUBMODULES_INITIALIZED)
#	# Update submodules as needed
#	option(GIT_SUBMODULE "Check submodules during build" ON)
#
#	if(GIT_SUBMODULE)
#		message(STATUS "Running update --init --recursive")
#		execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
#			WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/.."
#			RESULT_VARIABLE GIT_SUBMOD_RESULT)
#
#		if(NOT GIT_SUBMOD_RESULT EQUAL "0")
#			message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
#		endif()
#
#		message(STATUS "Submodule update done")
#		set(SUBMODULES_INITIALIZED True CACHE BOOL "Whether submodules are initialized" FORCE)
#	endif()
#endif()
#
# # -----------------------------------------------------
# # -----------------------------------------------------

#openmp
find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif ()
PackageHelper(OpenMP "${OPENMP_FOUND}" "" "")



# # ALL HEADER-ONLY LIBRARIES in external/include
set(PACKAGE_INCLUDES ${PACKAGE_INCLUDES} "${PROJECT_SOURCE_DIR}/external/include")
set(PACKAGE_INCLUDES ${PACKAGE_INCLUDES} "${PROJECT_SOURCE_DIR}/external/thirdparty/include")

# # -----------------------------------------------------
# # -----------------------------------------------------

# # Eigen3
set(Eigen3_BASE_PATH "${PROJECT_SOURCE_DIR}/external/thirdparty/eigen3")
find_package(Eigen3 3.4.0 QUIET HINTS "${Eigen3_BASE_PATH}/build")

# download and install manually
if((NOT Eigen3_FOUND AND INSTALL_MISSING_REQUIRED_DEPENDENCIES) OR FORCE_MANUAL_INSTALLATION)
	unset(EIGEN3_FOUND)

	if(NOT eigen3_POPULATED)
		message("Manually download, configure and build Eigen3 library...")
		FetchContent_Declare(
			Eigen3
			GIT_REPOSITORY "https://gitlab.com/libeigen/eigen.git"
			GIT_TAG "origin/3.4"
			CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${Eigen3_BASE_PATH}"
			PREFIX "${Eigen3_BASE_PATH}"
			DOWNLOAD_DIR "${Eigen3_BASE_PATH}/download"
			SOURCE_DIR "${Eigen3_BASE_PATH}/source"
			BINARY_DIR "${Eigen3_BASE_PATH}/build"
			SUBBUILD_DIR "${Eigen3_BASE_PATH}/subbuild"
		)

		FetchContent_Populate(Eigen3)
	endif()

	set(eigen3_POPULATED ${eigen3_POPULATED} CACHE BOOL "Whether eigen3 lib was manually loaded and installed" FORCE)

	execute_process(COMMAND "${CMAKE_COMMAND}" "-B" "${Eigen3_BASE_PATH}/build" "-S" "." "-DCMAKE_INSTALL_PREFIX=${Eigen3_BASE_PATH}/install" WORKING_DIRECTORY "${Eigen3_BASE_PATH}/source" ERROR_VARIABLE CONFIGURE_ERROR)
	execute_process(COMMAND "${CMAKE_COMMAND}" "--build" "."  "-j" "8" WORKING_DIRECTORY "${Eigen3_BASE_PATH}/build" ERROR_VARIABLE BUILD_ERROR)
	execute_process(COMMAND "${CMAKE_COMMAND}" "--install" "." WORKING_DIRECTORY "${Eigen3_BASE_PATH}/build" ERROR_VARIABLE INSTALL_ERROR)

	if(DEBUG_INSTALLATION)
		message(STATUS "Configure Error: ${CONFIGURE_ERROR}")
		message(STATUS "Build Error: ${BUILD_ERROR}")
		message(STATUS "Install Error: ${INSTALL_ERROR}")
	endif()

	set(Eigen3_DIR "${Eigen3_BASE_PATH}/build")
	find_package(Eigen3 3.4.0 QUIET REQUIRED PATHS ${Eigen3_DIR})

	unset(Eigen3_BASE_PATH)
endif()

PackageHelperTarget(Eigen3::Eigen EIGEN3_FOUND)


# # -----------------------------------------------------
# # -----------------------------------------------------

# Json
set(json_BASE_PATH "${PROJECT_SOURCE_DIR}/external/thirdparty/include/json")

if(NOT EXISTS "${json_BASE_PATH}")
	FetchContent_Declare(
		json
		GIT_REPOSITORY "https://github.com/nlohmann/json.git"
		CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${json_BASE_PATH}"
		PREFIX "${json_BASE_PATH}"
		DOWNLOAD_DIR "${json_BASE_PATH}/download"
		SOURCE_DIR "${json_BASE_PATH}/source"
		BINARY_DIR "${json_BASE_PATH}/build"
		SUBBUILD_DIR "${json_BASE_PATH}/subbuild"
	)
	#set(JSON_MultipleHeaders OFF)
	FetchContent_Populate(json)
endif()

set(json_BASE_PATH ${PROJECT_SOURCE_DIR}/external/thirdparty/include/json)
IncludeHelper(json_INCLUDE_DIRS nlohmann/json.hpp "${json_BASE_PATH}/source/single_include/" json_FOUND)
PackageHelper(json "${json_FOUND}" "${json_INCLUDE_DIRS}" "")

if(NOT json_FOUND)
	message(FATAL_ERROR "Could not find json.")
endif()

# # -----------------------------------------------------
# # -----------------------------------------------------


# argparse
include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)
PackageHelper(argparse "${argparse_FOUND}" "${argparse_INCLUDE_DIRS}" "${argparse_LIBRARIES}")


# # -----------------------------------------------------
# # -----------------------------------------------------


# zlib
set(zlib_BASE_PATH "${PROJECT_SOURCE_DIR}/external/thirdparty/zlib")

if(UNIX)
	set(zlib_libs libz.so)
	LibraryHelper(zlib_LIBRARIES zlib_libs "${zlib_BASE_PATH}/install/lib" zlib_FOUND)
elseif(WIN32)
	LibraryHelper(zlib_LIBRARIES zlib.lib "${zlib_BASE_PATH}/install/lib" zlib_FOUND)
	set(ZLIB_LIBRARY ${zlib_LIBRARIES})
endif()

IncludeHelper(zlib_INCLUDE_DIRS zlib.h "${zlib_BASE_PATH}/install/include" zlib_FOUND)
set(zlib_DIR "${zlib_BASE_PATH}/install")
set(ZLIB_DIR "${zlib_BASE_PATH}/install")

# download and install manually
if((NOT zlib_FOUND AND INSTALL_MISSING_REQUIRED_DEPENDENCIES) OR FORCE_MANUAL_INSTALLATION)
	unset(zlib_FOUND)

	if(NOT zlib_POPULATED)
		message("Manually download, configure and build zlib library...")
		FetchContent_Declare(
			zlib
			GIT_REPOSITORY "https://github.com/madler/zlib.git"
			CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${zlib_BASE_PATH}/install"
			PREFIX "${zlib_BASE_PATH}"
			DOWNLOAD_DIR "${zlib_BASE_PATH}/download"
			SOURCE_DIR "${zlib_BASE_PATH}/source"
			BINARY_DIR "${zlib_BASE_PATH}/build"
			SUBBUILD_DIR "${zlib_BASE_PATH}/subbuild"
		)


		FetchContent_Populate(zlib)

	endif()

	execute_process(COMMAND "${CMAKE_COMMAND}" "-B" "${zlib_BASE_PATH}/build" "-S" "." "-DCMAKE_INSTALL_PREFIX=${zlib_BASE_PATH}/install" WORKING_DIRECTORY "${zlib_BASE_PATH}/source" ERROR_VARIABLE CONFIGURE_ERROR)
	execute_process(COMMAND "${CMAKE_COMMAND}" "--build" "."  "-j" "8" WORKING_DIRECTORY "${zlib_BASE_PATH}/build" ERROR_VARIABLE BUILD_ERROR)
	execute_process(COMMAND "${CMAKE_COMMAND}" "--install" "." WORKING_DIRECTORY "${zlib_BASE_PATH}/build" ERROR_VARIABLE INSTALL_ERROR)

	if(DEBUG_INSTALLATION)
		message(STATUS "Configure Error: ${CONFIGURE_ERROR}")
		message(STATUS "Build Error: ${BUILD_ERROR}")
		message(STATUS "Install Error: ${INSTALL_ERROR}")
	endif()

	set(zlib_POPULATED ${zlib_POPULATED} CACHE BOOL "Whether zlib lib was manually loaded and installed" FORCE)

	set(zlib_DIR "${zlib_BASE_PATH}/install")
	set(ZLIB_DIR "${zlib_BASE_PATH}/install")

	if(UNIX)
		set(zlib_libs libz.so)
		LibraryHelper(zlib_LIBRARIES zlib_libs "${zlib_DIR}/lib" zlib_FOUND)
	elseif(WIN32)
		LibraryHelper(zlib_LIBRARIES zlib.lib "${zlib_DIR}/lib" zlib_FOUND)
		set(ZLIB_LIBRARY ${zlib_LIBRARIES})
	endif()

	IncludeHelper(zlib_INCLUDE_DIRS zlib.h "${zlib_DIR}/include" zlib_FOUND)
	set(ZLIB_INCLUDE_DIR ${zlib_INCLUDE_DIRS})

	if(NOT zlib_FOUND)
		message(FATAL_ERROR "Could not find zlib.")
	endif()

	unset(zlib_BASE_PATH)
endif()

PackageHelper(zlib "${zlib_FOUND}" "${zlib_INCLUDE_DIRS}" "${zlib_LIBRARIES}")
unset(zlib_libs)

# # -----------------------------------------------------
# # -----------------------------------------------------


# cnpy
fetchcontent_declare(
        cnpy
        GIT_REPOSITORY "https://github.com/rogersce/cnpy.git")
FetchContent_MakeAvailable(cnpy)

PackageHelper(cnpy TRUE "${cnpy_SOURCE_DIR}" "${cnpy_LIBRARIES}")

# # -----------------------------------------------------
# # -----------------------------------------------------


# glog
set(GLOG_BASE_PATH "${PROJECT_SOURCE_DIR}/external/thirdparty/glog")
find_package(glog QUIET HINTS "${GLOG_BASE_PATH}/install/lib")

if((NOT glog_FOUND AND INSTALL_MISSING_REQUIRED_DEPENDENCIES) OR FORCE_MANUAL_INSTALLATION)
	unset(glog_FOUND)
	message("Manually download, configure and build glog library...")

	if(NOT glog_POPULATED)
		FetchContent_Declare(
			glog
			GIT_REPOSITORY "https://github.com/google/glog.git"
			GIT_TAG "v0.5.0"
			PREFIX "${GLOG_BASE_PATH}"
			DOWNLOAD_DIR "${GLOG_BASE_PATH}/download"
			SOURCE_DIR "${GLOG_BASE_PATH}/source"
			BINARY_DIR "${GLOG_BASE_PATH}/build"
			SUBBUILD_DIR "${GLOG_BASE_PATH}/subbuild"
		)

		FetchContent_Populate(glog)
	endif()

	execute_process(COMMAND "${CMAKE_COMMAND}" "-B" "${GLOG_BASE_PATH}/build" "-S" "." "-DCMAKE_INSTALL_PREFIX=${GLOG_BASE_PATH}/install" WORKING_DIRECTORY "${GLOG_BASE_PATH}/source" ERROR_VARIABLE CONFIGURE_ERROR)
	execute_process(COMMAND "${CMAKE_COMMAND}" "--build" "."  "-j" "8" WORKING_DIRECTORY "${GLOG_BASE_PATH}/build" ERROR_VARIABLE BUILD_ERROR)
	execute_process(COMMAND "${CMAKE_COMMAND}" "--install" "." WORKING_DIRECTORY "${GLOG_BASE_PATH}/build" ERROR_VARIABLE INSTALL_ERROR)

	if(DEBUG_INSTALLATION)
		message(STATUS "Configure Error: ${CONFIGURE_ERROR}")
		message(STATUS "Build Error: ${BUILD_ERROR}")
		message(STATUS "Install Error: ${INSTALL_ERROR}")
	endif()

	set(glog_POPULATED ${glog_POPULATED} CACHE BOOL "Whether glog was manually loaded and installed" FORCE)

	set(glog_DIR "${GLOG_BASE_PATH}/install/lib")
	find_package(glog REQUIRED)
	find_package(glog REQUIRED PATHS ${glog_DIR})

	unset(GLOG_BASE_PATH)
endif()

PackageHelperTarget(glog::glog glog_FOUND)

# # -----------------------------------------------------
# # -----------------------------------------------------


#OpenSim

set(OPENSIM_INSTALL_DIR "~/proj/opensim/install" CACHE PATH "Path to OpenSim Install.")
find_package(OpenSim 4.0 REQUIRED)
find_package(OpenSim 4.0 REQUIRED PATHS "${OPENSIM_INSTALL_DIR}")
PackageHelper(OpenSim ${OPENSIM_FOUND} "${OPENSIM_INCLUDE_DIRS}/OpenSim" "${OPENSIM_LIBRARIES}" )
PackageHelper(OpenSim ${OPENSIM_FOUND} "${OPENSIMSIMBODY_INCLUDE_DIRS}" "${OPENSIMSIMBODY_LIBRARIES}" )
PackageHelperTarget(osimTools OPENSIM_FOUND)

# # -----------------------------------------------------
# # -----------------------------------------------------

#ezc3d

set(ezc3d_BASE_PATH "${PROJECT_SOURCE_DIR}/external/thirdparty/ezc3d")

if(UNIX)
	set(ezc3d_libs libezc3d.so)
	LibraryHelper(ezc3d_LIBRARIES ezc3d_libs "${ezc3d_BASE_PATH}/install/lib" ezc3d_FOUND)
elseif(WIN32)
	LibraryHelper(ezc3d_LIBRARIES ezc3d.lib "${ezc3d_BASE_PATH}/install/lib" ezc3d_FOUND)
	set(ezc3d_LIBRARY ${ezc3d_LIBRARIES})
endif()

IncludeHelper(ezc3d_INCLUDE_DIRS ezc3d/ezc3d_all.h "${ezc3d_BASE_PATH}/install/include/" ezc3d_FOUND)
set(ezc3d_DIR "${ezc3d_BASE_PATH}/install")
set(EZC3D_DIR "${ezc3d_BASE_PATH}/install")

# download and install manually
if(NOT ezc3d_FOUND OR FORCE_MANUAL_INSTALLATION)
	unset(ezc3d_FOUND)

	if(NOT ezc3d_POPULATED)
		message("Manually download, configure and build ezc3d library...")
		FetchContent_Declare(
				ezc3d
				GIT_REPOSITORY "https://github.com/pyomeca/ezc3d.git"
				GIT_TAG "dev"
				CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${ezc3d_BASE_PATH}/install"
				PREFIX "${ezc3d_BASE_PATH}"
				DOWNLOAD_DIR "${ezc3d_BASE_PATH}/download"
				SOURCE_DIR "${ezc3d_BASE_PATH}/source"
				BINARY_DIR "${ezc3d_BASE_PATH}/build"
				SUBBUILD_DIR "${ezc3d_BASE_PATH}/subbuild"
		)


		FetchContent_Populate(ezc3d)

	endif()

	execute_process(COMMAND "${CMAKE_COMMAND}" "-B" "${ezc3d_BASE_PATH}/build" "-S" "." "-DCMAKE_INSTALL_PREFIX=${ezc3d_BASE_PATH}/install" "-DBUILD_EXAMPLE=OFF" WORKING_DIRECTORY "${ezc3d_BASE_PATH}/source" ERROR_VARIABLE CONFIGURE_ERROR)
	execute_process(COMMAND "${CMAKE_COMMAND}" "--build" "."  "-j" "8" WORKING_DIRECTORY "${ezc3d_BASE_PATH}/build" ERROR_VARIABLE BUILD_ERROR)
	execute_process(COMMAND "${CMAKE_COMMAND}" "--install" "." WORKING_DIRECTORY "${ezc3d_BASE_PATH}/build" ERROR_VARIABLE INSTALL_ERROR)

	if(DEBUG_INSTALLATION)
		message(STATUS "Configure Error: ${CONFIGURE_ERROR}")
		message(STATUS "Build Error: ${BUILD_ERROR}")
		message(STATUS "Install Error: ${INSTALL_ERROR}")
	endif()

	set(ezc3d_POPULATED ${ezc3d_POPULATED} CACHE BOOL "Whether ezc3d lib was manually loaded and installed" FORCE)

	set(ezc3d_DIR "${ezc3d_BASE_PATH}/install")
	set(EZC3D_DIR "${ezc3d_BASE_PATH}/install")

	if(UNIX)
		set(ezc3d_libs libezc3d.so)
		LibraryHelper(ezc3d_LIBRARIES ezc3d_libs "${ezc3d_DIR}/lib" ezc3d_FOUND)
	elseif(WIN32)
		LibraryHelper(ezc3d_LIBRARIES ezc3d.lib "${ezc3d_DIR}/lib" ezc3d_FOUND)
		set(ezc3d_LIBRARY ${ezc3d_LIBRARIES})
	endif()

	IncludeHelper(ezc3d_INCLUDE_DIRS ezc3d/ezc3d.h "${ezc3d_DIR}/include/" ezc3d_FOUND)
	set(ezc3d_INCLUDE_DIR ${ezc3d_INCLUDE_DIRS})

	if(NOT ezc3d_FOUND)
		message(FATAL_ERROR "Could not find ezc3d.")
	endif()

	unset(ezc3d_BASE_PATH)
endif()

PackageHelper(ezc3d "${ezc3d_FOUND}" "${ezc3d_INCLUDE_DIRS}" "${ezc3d_LIBRARIES}")
unset(ezc3d_libs)

# # -----------------------------------------------------
# # -----------------------------------------------------

#tinyfd

set(tinyfd_BASE_PATH "${PROJECT_SOURCE_DIR}/external/thirdparty/tinyfd")

if(UNIX)
	set(tinyfd_libs libTinyFileDialogs.so)
	LibraryHelper(tinyfd_LIBRARIES tinyfd_libs "${tinyfd_BASE_PATH}/install/lib" tinyfd_FOUND)
elseif(WIN32)
	LibraryHelper(tinyfd_LIBRARIES libTinyFileDialogs.lib "${tinyfd_BASE_PATH}/install/lib" tinyfd_FOUND)
	set(tinyfd_LIBRARY ${tinyfd_LIBRARIES})
endif()

IncludeHelper(tinyfd_INCLUDE_DIRS tinyfiledialogs.h "${tinyfd_BASE_PATH}/install/include/" tinyfd_FOUND)
set(tinyfd_DIR "${tinyfd_BASE_PATH}/install")
set(TINYFD_DIR "${tinyfd_BASE_PATH}/install")

# download and install manually
if(NOT tinyfd_FOUND OR FORCE_MANUAL_INSTALLATION)
	unset(tinyfd_FOUND)

	if(NOT tinyfd_POPULATED)
		message("Manually download, configure and build tinyfd library...")
		FetchContent_Declare(
				tinyfd
				GIT_REPOSITORY "https://github.com/DerZieger/tinyfiledialogs.git"
				CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${tinyfd_BASE_PATH}/install"
				GIT_TAG "main"
				PREFIX "${tinyfd_BASE_PATH}"
				DOWNLOAD_DIR "${tinyfd_BASE_PATH}/download"
				SOURCE_DIR "${tinyfd_BASE_PATH}/source"
				BINARY_DIR "${tinyfd_BASE_PATH}/build"
				SUBBUILD_DIR "${tinyfd_BASE_PATH}/subbuild"
		)


		FetchContent_Populate(tinyfd)

	endif()

	execute_process(COMMAND "${CMAKE_COMMAND}" "-B" "${tinyfd_BASE_PATH}/build" "-S" "." "-DCMAKE_INSTALL_PREFIX=${tinyfd_BASE_PATH}/install" WORKING_DIRECTORY "${tinyfd_BASE_PATH}/source" ERROR_VARIABLE CONFIGURE_ERROR)
	execute_process(COMMAND "${CMAKE_COMMAND}" "--build" "."  "-j" "8" WORKING_DIRECTORY "${tinyfd_BASE_PATH}/build" ERROR_VARIABLE BUILD_ERROR)
	execute_process(COMMAND "${CMAKE_COMMAND}" "--install" "." WORKING_DIRECTORY "${tinyfd_BASE_PATH}/build" ERROR_VARIABLE INSTALL_ERROR)

	if(DEBUG_INSTALLATION)
		message(STATUS "Configure Error: ${CONFIGURE_ERROR}")
		message(STATUS "Build Error: ${BUILD_ERROR}")
		message(STATUS "Install Error: ${INSTALL_ERROR}")
	endif()

	set(tinyfd_POPULATED ${tinyfd_POPULATED} CACHE BOOL "Whether tinyfd lib was manually loaded and installed" FORCE)

	set(tinyfd_DIR "${tinyfd_BASE_PATH}/install")
	set(TINYFD_DIR "${tinyfd_BASE_PATH}/install")

	if(UNIX)
		set(tinyfd_libs libTinyFileDialogs.so)
		LibraryHelper(tinyfd_LIBRARIES tinyfd_libs "${tinyfd_BASE_PATH}/install/lib" tinyfd_FOUND)
	elseif(WIN32)
		LibraryHelper(tinyfd_LIBRARIES libTinyFileDialogs.lib "${tinyfd_BASE_PATH}/install/lib" tinyfd_FOUND)
		set(tinyfd_LIBRARY ${tinyfd_LIBRARIES})
	endif()


	IncludeHelper(tinyfd_INCLUDE_DIRS tinyfiledialogs.h "${tinyfd_BASE_PATH}/install/include/" tinyfd_FOUND)
	set(tinyfd_INCLUDE_DIR ${tinyfd_INCLUDE_DIRS})

	if(NOT tinyfd_FOUND)
		message(FATAL_ERROR "Could not find tinyfd.")
	endif()

	unset(tinyfd_BASE_PATH)
endif()

PackageHelper(tinyfd "${tinyfd_FOUND}" "${tinyfd_INCLUDE_DIRS}" "${tinyfd_LIBRARIES}")
unset(tinyfd_libs)

# # -----------------------------------------------------
# # -----------------------------------------------------



unset(DEBUG_INSTALLATION)
unset(BUILD_SHARED_LIBS)

set(CORE_INCLUDES ${PACKAGE_INCLUDES})
set(CORE_LIBS ${LIBS})
set(CORE_TARGETS ${LIB_TARGETS})
set(MODULE_CORE 1)
