# target name to generate here
set(TARGET viewer)

# glob source files
set(INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
file(GLOB_RECURSE HEADERS
        "${INCLUDE_DIR}/*.h"
        "${INCLUDE_DIR}/*.hpp"
)
file(GLOB_RECURSE SOURCES "*.cpp")


set(LIB_DEPENDENCIES
        ${CORE_LIBS} ${CORE_TARGETS}
        ${CUDA_LIBRARIES}
        ${DL_TARGETS} ${DL_LIBS}
)

# additional include directories that are not already satisfied by
# the targets in LIB_DEPENDENCIES
set(LIB_INCLUDES
        ${CUDA_INCLUDE_DIRS}
        ${CORE_INCLUDES}
        ${DL_INCLUDES}
)
# define target
add_executable(${TARGET} ${SOURCES} ${HEADERS})
# forces executables to be compiled to /example/ folder, to allow relative paths for shaders
set_target_properties(${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${PROJECT_SOURCE_DIR}")

target_include_directories(${TARGET} PUBLIC ${INCLUDE_DIR} ${LIB_INCLUDES})
# built libs
target_link_libraries(${TARGET} PUBLIC cppgl supr vposer argparse cnpy iir::iir ${LIB_DEPENDENCIES} )

if(DL_MODULE)
    target_compile_definitions(${TARGET} PUBLIC HAS_DL_MODULE)
endif()

source_group(
        TREE "${INCLUDE_DIR}"
        PREFIX "Header Files"
        FILES ${HEADERS}
)
source_group(
        TREE "${CMAKE_CURRENT_SOURCE_DIR}"
        PREFIX "Source Files"
        FILES ${SOURCES}
)

