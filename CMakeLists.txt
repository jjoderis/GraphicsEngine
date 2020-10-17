cmake_minimum_required(VERSION 3.10)

project(GraphicsEngine VERSION 0.1 DESCRIPTION "A graphics engine for practice purposes" LANGUAGES CXX)

add_executable(engine main.cpp)

# TESTING

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    include(CTest)
endif()

if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME AND BUILD_TESTING)
    add_subdirectory(tests)
endif()