#include "tatami_test/test_access.hpp"
#include "tatami_test/simulate_vector.hpp"
#include "tatami/tatami.hpp"

static std::vector<double> manual_transpose(size_t NR, size_t NC, const std::vector<double>& contents) {
    std::vector<double> transposed(NR * NC);
    for (size_t r = 0; r < NR; ++r) {
        for (size_t c = 0; c < NC; ++c) {
            transposed[c * NR + r] = contents[r * NC + c];
        }
    }
    return transposed;
}

class TestTestAccess : public ::testing::TestWithParam<tatami_test::StandardTestAccessOptions> {};

TEST_P(TestTestAccess, Parametrized) {
    auto options = tatami_test::convert_test_access_options(GetParam());

    size_t NR = 100, NC = 200;
    auto simulated = tatami_test::simulate_vector<double>(NR * NC, tatami_test::SimulateVectorOptions());
    tatami::DenseMatrix<double, int> mat(NR, NC, simulated, true);
    auto transposed = manual_transpose(NR, NC, simulated); // Manual transposition for comparison.
    tatami::DenseMatrix<double, int> ref(NR, NC, transposed, false);

    tatami_test::test_full_access(mat, ref, options);

    tatami_test::test_block_access(mat, ref, 0, 0.7, options);
    tatami_test::test_block_access(mat, ref, 0.27, 0.6, options);
    tatami_test::test_block_access(mat, ref, 0.51, 0.4, options);

    tatami_test::test_indexed_access(mat, ref, 0, 0.1, options);
    tatami_test::test_indexed_access(mat, ref, 0.3, 0.2, options);
    tatami_test::test_indexed_access(mat, ref, 0.7, 0.5, options);
}

INSTANTIATE_TEST_SUITE_P(
    TestAccess,
    TestTestAccess,
    tatami_test::standard_test_access_options_combinations()
);

TEST(TestAccess, Simple) {
    size_t NR = 199, NC = 99;
    auto simulated = tatami_test::simulate_vector<double>(NR * NC, tatami_test::SimulateVectorOptions());
    tatami::DenseMatrix<double, int> mat(NR, NC, simulated, true);
    auto transposed = manual_transpose(NR, NC, simulated); // Manual transposition for comparison.
    tatami::DenseMatrix<double, int> ref(NR, NC, transposed, false);

    tatami_test::test_simple_row_access(mat, ref);
    tatami_test::test_simple_column_access(mat, ref);
}

TEST(TestAccess, HandlesNaN) {
    size_t NR = 57, NC = 243;
    auto simulated = tatami_test::simulate_vector<double>(NR * NC, tatami_test::SimulateVectorOptions());
    simulated.front() = std::numeric_limits<double>::quiet_NaN();
    simulated[1000] = std::numeric_limits<double>::quiet_NaN();
    simulated.back() = std::numeric_limits<double>::quiet_NaN();

    tatami::DenseMatrix<double, int> mat(NR, NC, simulated, true);
    auto transposed = manual_transpose(NR, NC, simulated); // Manual transposition for comparison.
    tatami::DenseMatrix<double, int> ref(NR, NC, transposed, false);

    tatami_test::test_simple_row_access(mat, ref);
    tatami_test::test_simple_column_access(mat, ref);
}

