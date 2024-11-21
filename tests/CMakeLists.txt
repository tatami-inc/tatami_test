add_executable(
    libtest 
    src/simulate_vector.cpp
    src/simulate_compressed_sparse.cpp
    src/throws_error.cpp
    src/test_access.cpp
    src/test_unsorted_access.cpp
    src/ReversedIndicesWrapper.cpp
    src/ForcedOracleWrapper.cpp
)

target_link_libraries(
    libtest
    tatami_test
)

include(FetchContent)
FetchContent_Declare(
  tatami
  GIT_REPOSITORY https://github.com/tatami-inc/tatami
  GIT_TAG master # ^3.0.0
)
FetchContent_MakeAvailable(tatami)
target_link_libraries(libtest tatami)

target_compile_options(libtest PRIVATE -Wall -Werror -Wpedantic -Wextra)

option(CODE_COVERAGE "Enable coverage testing" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(libtest PRIVATE -O0 -g --coverage)
    target_link_options(libtest PRIVATE --coverage)
endif()

include(GoogleTest)
gtest_discover_tests(libtest)
