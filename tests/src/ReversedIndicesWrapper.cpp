#include "tatami_test/test_unsorted_access.hpp"
#include "tatami_test/simulate_compressed_sparse.hpp"
#include "tatami_test/ReversedIndicesWrapper.hpp"
#include "tatami/tatami.hpp"

class TestReversedIndicesWrapper : public ::testing::TestWithParam<tatami_test::StandardTestAccessOptions> {};

TEST_P(TestReversedIndicesWrapper, Parametrized) {
    auto options = tatami_test::convert_test_access_options(GetParam());

    size_t NR = 152, NC = 198;
    auto simulated = tatami_test::simulate_compressed_sparse<double, int>(NR, NC, tatami_test::SimulateCompressedSparseOptions());
    auto mat = std::make_shared<tatami::CompressedSparseMatrix<double, int> >(NR, NC, std::move(simulated.data), std::move(simulated.index), std::move(simulated.indptr), true);
    tatami_test::ReversedIndicesWrapper<double, int> wrapped(std::move(mat));

    tatami_test::test_unsorted_full_access(wrapped, options);
    tatami_test::test_unsorted_block_access(wrapped, 0.27, 0.6, options);
    tatami_test::test_unsorted_indexed_access(wrapped, 0.1, 0.25, options);
}

INSTANTIATE_TEST_SUITE_P(
    ReversedIndicesWrapper,
    TestReversedIndicesWrapper,
    tatami_test::standard_test_access_options_combinations()
);
