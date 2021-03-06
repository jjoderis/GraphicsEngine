cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)

project(GraphicsEngine VERSION 0.1 DESCRIPTION "A graphics engine for practice purposes" LANGUAGES CXX)

set(SRC_DIR "${PROJECT_SOURCE_DIR}/src")
set(EXTERN_DIR "${PROJECT_SOURCE_DIR}/external")
set(APP_DIR "${PROJECT_SOURCE_DIR}/apps")
set(BUILD_DIR "${PROJECT_SOURCE_DIR}/build")

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


add_subdirectory(src/)
add_subdirectory(apps/)

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

# TESTING
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()