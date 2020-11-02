cmake_minimum_required(VERSION 3.11)

project(GraphicsEngine VERSION 0.1 DESCRIPTION "A graphics engine for practice purposes" LANGUAGES CXX)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(EXTERN_DIR "${PROJECT_SOURCE_DIR}/external")
set(APP_DIR "${PROJECT_SOURCE_DIR}/apps")
set(BUILD_DIR "${PROJECT_SOURCE_DIR}/build")

# TESTING
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()

# automatically update git submodules
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

#if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/imgui/CMakeLists.txt")
#    message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
#endif()


add_subdirectory("${EXTERN_DIR}/glad" "${BUILD_DIR}/external/glad")
add_subdirectory("${EXTERN_DIR}/glfw" "${BUILD_DIR}/external/glfw")

# imgui
set(IMGUI_SOURCES
    "${EXTERN_DIR}/imgui/imgui.cpp"
    "${EXTERN_DIR}/imgui/imgui_demo.cpp"
    "${EXTERN_DIR}/imgui/imgui_draw.cpp"
    "${EXTERN_DIR}/imgui/imgui_widgets.cpp"
    "${EXTERN_DIR}/imgui/backends/imgui_impl_opengl3.cpp"
    "${EXTERN_DIR}/imgui/backends/imgui_impl_glfw.cpp"
)

set(IMGUI_INCLUDE_DIR "${EXTERN_DIR}/imgui")
set(MATHLIB_INCLUDE_DIR "${EXTERN_DIR}/mathlib/src")

add_executable(Modeler "${APP_DIR}/modeler.cpp" ${IMGUI_SOURCES})
target_include_directories(Modeler PRIVATE ${IMGUI_INCLUDE_DIR} ${MATHLIB_INCLUDE_DIR})
target_link_libraries(Modeler glfw glad)