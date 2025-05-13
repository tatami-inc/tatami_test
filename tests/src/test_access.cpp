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

class TestAccessTest : public ::testing::TestWithParam<tatami_test::StandardTestAccessOptions> {};

TEST_P(TestAccessTest, Parametrized) {
    auto options = tatami_test::convert_test_access_options(GetParam());

    size_t NR = 100, NC = 200;
    auto simulated = tatami_test::simulate_vector<double>(NR * NC, tatami_test::SimulateVectorOptions());
    tatami::DenseMatrix<double, int, decltype(simulated)> mat(NR, NC, simulated, true);
    auto transposed = manual_transpose(NR, NC, simulated); // Manual transposition for comparison.
    tatami::DenseMatrix<double, int, decltype(simulated)> ref(NR, NC, transposed, false);

    tatami_test::test_full_access(mat, ref, options);

    tatami_test::test_block_access(mat, ref, 0, 0.7, options);
    tatami_test::test_block_access(mat, ref, 0.27, 0.6, options);
    tatami_test::test_block_access(mat, ref, 0.51, 0.4, options);

    tatami_test::test_indexed_access(mat, ref, 0, 0.1, options);
    tatami_test::test_indexed_access(mat, ref, 0.3, 0.2, options);
    tatami_test::test_indexed_access(mat, ref, 0.7, 0.5, options);
}

TEST_P(TestAccessTest, Empty) {
    auto options = tatami_test::convert_test_access_options(GetParam());

    {
        size_t NR = 10, NC = 0;
        tatami::DenseMatrix<double, int, std::vector<double> > mat(NR, NC, std::vector<double>(), true);
        tatami::DenseMatrix<double, int, std::vector<double> > ref(NR, NC, std::vector<double>(0), false);

        tatami_test::test_block_access(mat, ref, 0, 0, options);
        tatami_test::test_indexed_access(mat, ref, 0, 1, options);
    }

    {
        size_t NR = 0, NC = 10;
        tatami::DenseMatrix<double, int, std::vector<double> > mat(NR, NC, std::vector<double>(), true);
        tatami::DenseMatrix<double, int, std::vector<double> > ref(NR, NC, std::vector<double>(), false);

        tatami_test::test_block_access(mat, ref, 0, 0, options);
        tatami_test::test_indexed_access(mat, ref, 0, 1, options);
    }
}

INSTANTIATE_TEST_SUITE_P(
    TestAccess,
    TestAccessTest,
    tatami_test::standard_test_access_options_combinations()
);

TEST(TestAccess, Simple) {
    size_t NR = 199, NC = 99;
    auto simulated = tatami_test::simulate_vector<double>(NR * NC, tatami_test::SimulateVectorOptions());
    tatami::DenseMatrix<double, int, decltype(simulated)> mat(NR, NC, simulated, true);
    auto transposed = manual_transpose(NR, NC, simulated); // Manual transposition for comparison.
    tatami::DenseMatrix<double, int, decltype(simulated)> ref(NR, NC, transposed, false);

    tatami_test::test_simple_row_access(mat, ref);
    tatami_test::test_simple_column_access(mat, ref);
}

TEST(TestAccess, HandlesNaN) {
    size_t NR = 57, NC = 243;
    auto simulated = tatami_test::simulate_vector<double>(NR * NC, tatami_test::SimulateVectorOptions());
    simulated.front() = std::numeric_limits<double>::quiet_NaN();
    simulated[1000] = std::numeric_limits<double>::quiet_NaN();
    simulated.back() = std::numeric_limits<double>::quiet_NaN();

    tatami::DenseMatrix<double, int, decltype(simulated)> mat(NR, NC, simulated, true);
    auto transposed = manual_transpose(NR, NC, simulated); // Manual transposition for comparison.
    tatami::DenseMatrix<double, int, decltype(transposed)> ref(NR, NC, transposed, false);

    tatami_test::test_simple_row_access(mat, ref);
    tatami_test::test_simple_column_access(mat, ref);
}

class SimulateTestAccessSequenceTest : public ::testing::TestWithParam<int> {};

TEST_P(SimulateTestAccessSequenceTest, Forward) {
    auto jump = GetParam();
    auto simulated = tatami_test::internal::simulate_test_access_sequence(10, 20, [&]{
        tatami_test::TestAccessOptions options;
        options.use_row = true;
        options.jump = jump;
        return options;
    }());
    EXPECT_LT(simulated.front(), jump);
    for (std::size_t i = 1; i < simulated.size(); ++i) {
        EXPECT_EQ(simulated[i] - simulated[i-1], jump);
    }
    EXPECT_GE(simulated.back() + jump, 10);
}

TEST_P(SimulateTestAccessSequenceTest, Reverse) {
    auto jump = GetParam();
    auto simulated = tatami_test::internal::simulate_test_access_sequence(10, 20, [&]{
        tatami_test::TestAccessOptions options;
        options.use_row = false;
        options.order = tatami_test::TestAccessOrder::REVERSE;
        options.jump = jump;
        return options;
    }());
    EXPECT_LT(simulated.back(), jump);
    for (std::size_t i = 1; i < simulated.size(); ++i) {
        EXPECT_EQ(simulated[i - 1] - simulated[i], jump);
    }
    EXPECT_GE(simulated.front() + jump, 20);
}

INSTANTIATE_TEST_SUITE_P(
    TestAccess,
    SimulateTestAccessSequenceTest,
    ::testing::Values(1,2,3,4,5,6,7)
);
