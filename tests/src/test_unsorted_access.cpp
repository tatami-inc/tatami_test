#include "tatami_test/test_unsorted_access.hpp"
#include "tatami_test/simulate_compressed_sparse.hpp"
#include "tatami/tatami.hpp"

class TestTestUnsortedAccess : public ::testing::TestWithParam<tatami_test::StandardTestAccessOptions> {};

TEST_P(TestTestUnsortedAccess, Parametrized) {
    auto options = tatami_test::convert_test_access_options(GetParam());

    size_t NR = 100, NC = 200;
    auto simulated = tatami_test::simulate_compressed_sparse<double, int>(NR, NC, tatami_test::SimulateCompressedSparseOptions());
    tatami::CompressedSparseMatrix<double, int> mat(NR, NC, std::move(simulated.data), std::move(simulated.index), std::move(simulated.indptr), true);

    tatami_test::test_unsorted_full_access(mat, options);

    tatami_test::test_unsorted_block_access(mat, 0, 0.7, options);
    tatami_test::test_unsorted_block_access(mat, 0.27, 0.6, options);
    tatami_test::test_unsorted_block_access(mat, 0.51, 0.4, options);

    tatami_test::test_unsorted_indexed_access(mat, 0, 0.1, options);
    tatami_test::test_unsorted_indexed_access(mat, 0.3, 0.2, options);
    tatami_test::test_unsorted_indexed_access(mat, 0.7, 0.5, options);
}

INSTANTIATE_TEST_SUITE_P(
    TestUnsortedAccess,
    TestTestUnsortedAccess,
    tatami_test::standard_test_access_options_combinations()
);

TEST(TestUnsortedAccess, HandlesNaN) {
    size_t NR = 100, NC = 200;
    auto simulated = tatami_test::simulate_compressed_sparse<double, int>(NR, NC, tatami_test::SimulateCompressedSparseOptions());
    simulated.data.front() = std::numeric_limits<double>::quiet_NaN();
    simulated.data.back() = std::numeric_limits<double>::quiet_NaN();
    tatami::CompressedSparseMatrix<double, int> mat(NR, NC, std::move(simulated.data), std::move(simulated.index), std::move(simulated.indptr), true);

    tatami_test::TestAccessOptions options;
    options.use_row = true;
    tatami_test::test_unsorted_full_access(mat, options);
    options.use_row = false;
    tatami_test::test_unsorted_full_access(mat, options);
}
