cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -pedantic-errors")

##########################
#GTest

set(GOOGLETEST_DIR "${PROJECT_SOURCE_DIR}/external/googletest")

# Download and unpack googletest at configure time
configure_file("${GOOGLETEST_DIR}/CMakeLists.txt.in" "${GOOGLETEST_DIR}/googletest-download/CMakeLists.txt")
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
RESULT_VARIABLE result
WORKING_DIRECTORY "${GOOGLETEST_DIR}/googletest-download" )
if(result)
message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
RESULT_VARIABLE result
WORKING_DIRECTORY "${GOOGLETEST_DIR}/googletest-download" )
if(result)
message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gtest and gtest_main targets.
add_subdirectory("${GOOGLETEST_DIR}/googletest-src"
                "${GOOGLETEST_DIR}/googletest-build"
                EXCLUDE_FROM_ALL)

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
include_directories("${gtest_SOURCE_DIR}/include")
endif()

##########################

set(TEST_FILES
    example/example.test.cpp
)

# link test files against gtest_main
add_executable(tests ${TEST_FILES})
target_link_libraries(tests gtest gmock gtest_main)
add_test(NAME example_test COMMAND tests)