#include "tatami_test/ForcedOracleWrapper.hpp"
#include "tatami_test/test_access.hpp"
#include "tatami_test/simulate_vector.hpp"
#include "tatami/tatami.hpp"

class ForcedOracleWrapperTest : public ::testing::TestWithParam<tatami_test::StandardTestAccessOptions> {};

TEST_P(ForcedOracleWrapperTest, Parametrized) {
    auto options = tatami_test::convert_test_access_options(GetParam());

    size_t NR = 100, NC = 200;
    auto simulated = tatami_test::simulate_vector<double>(NR * NC, tatami_test::SimulateVectorOptions());
    auto mat = std::make_shared<tatami::DenseMatrix<double, int, decltype(simulated)> >(NR, NC, simulated, true);
    tatami_test::ForcedOracleWrapper<double, int> wrapped(mat); 
    EXPECT_TRUE(wrapped.uses_oracle(true));

    tatami_test::test_full_access(wrapped, *mat, options);
    tatami_test::test_block_access(wrapped, *mat, 0.17, 0.3, options);
    tatami_test::test_indexed_access(wrapped, *mat, 0.05, 0.2, options);
}

INSTANTIATE_TEST_SUITE_P(
    ForcedOracleWrapper,
    ForcedOracleWrapperTest,
    tatami_test::standard_test_access_options_combinations()
);
