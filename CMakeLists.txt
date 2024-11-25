cmake_minimum_required(VERSION 3.24)

project(tatami_test
    VERSION 0.1.0
    DESCRIPTION "Testing utilities for tatami ecosystem"
    LANGUAGES CXX)

add_library(tatami_test INTERFACE)
target_include_directories(tatami_test INTERFACE include)
target_compile_features(tatami_test INTERFACE cxx_std_17)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/d144031940543e15423a25ae5a8a74141044862f.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Avoid installing GoogleTest when installing this project.
option(INSTALL_GTEST "Enable installation of googletest." OFF)

FetchContent_MakeAvailable(googletest)
target_link_libraries(tatami_test INTERFACE gtest_main)

# Tests
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    option(TATAMI_TEST_TESTS "Build tatami_test's test suite." ON)
else()
    option(TATAMI_TEST_TESTS "Build tatami_test's test suite." OFF)
endif()

if(TATAMI_TEST_TESTS)
    include(CTest)
    if(BUILD_TESTING)
        add_subdirectory(tests)
    endif() 
endif()
