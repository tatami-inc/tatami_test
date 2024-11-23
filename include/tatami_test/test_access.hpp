#ifndef TATAMI_TEST_TEST_ACCESS_HPP
#define TATAMI_TEST_TEST_ACCESS_HPP

#include <gtest/gtest.h>

#include "tatami/utils/new_extractor.hpp"
#include "tatami/utils/ConsecutiveOracle.hpp"
#include "tatami/utils/FixedOracle.hpp"

#include "fetch.hpp"

#include <vector>
#include <limits>
#include <random>
#include <cmath>
#include <memory>
#include <cstdint>

/**
 * @file test_access.hpp
 * @brief Test access patterns on a `tatami::Matrix`.
 */

namespace tatami_test {

/**
 * Order for accessing rows/columns during `tatami::Matrix` access tests. 
 *
 * - `FORWARD`: rows/columns are accessed in strictly increasing order.
 * - `REVERSE`: rows/columns are accessed in strictly decreasing order.
 * - `RANDOM`: rows/columns are accessed in random order.
 */
enum class TestAccessOrder : char { FORWARD, REVERSE, RANDOM };

/**
 * @brief Options for `test_full_access()` and friends.
 */
struct TestAccessOptions {
    /**
     * Whether to use an oracle.
     */
    bool use_oracle = false;

    /**
     * Whether to test row access. 
     * If `false`, column access is tested instead.
     */
    bool use_row = true;

    /**
     * Ordering of row/column accesses in the test.
     */
    TestAccessOrder order = TestAccessOrder::FORWARD;

    /**
     * Minimum distance between rows/columns to be accessed in the test.
     */
    int jump = 1;

    /**
     * Whether to check that "sparse" matrices actually have density below 1.
     */
    bool check_sparse = true;
};

/**
 * Contents of `TestAccessOptions` as a tuple.
 * This is required for GoogleTest's parametrized generators, see `standard_test_access_options_combinations()`.
 */
typedef std::tuple<bool, bool, TestAccessOrder, int> StandardTestAccessOptions;

/**
 * Convert from tuple-like options into a `TestAccessOptions` object.
 * This allows `TEST_P` bodies to easily convert the `GetParam()`-supplied tuple into options for `test_full_access()` and friends.
 *
 * @param x Options as a tuple.
 * @return The same options as a `TestAccessOptions` object.
 */
inline TestAccessOptions convert_test_access_options(const StandardTestAccessOptions& x) {
    TestAccessOptions output;
    output.use_row = std::get<0>(x);
    output.use_oracle = std::get<1>(x);
    output.order = std::get<2>(x);
    output.jump = std::get<3>(x);
    return output;
}

/**
 * @return A parametrized GoogleTest generator for all `TestAccessOptions` combinations.
 * This should be used inside a `INSTANTIATE_TEST_SUITE_P` macro, which ensures that `GetParam()` in the `TEST_P` body returns a `StandardTestAccessOptions` instance.
 */
inline auto standard_test_access_options_combinations() {
    return ::testing::Combine(
        ::testing::Values(true, false), /* whether to access the rows. */
        ::testing::Values(true, false), /* whether to use an oracle. */
        ::testing::Values(TestAccessOrder::FORWARD, TestAccessOrder::REVERSE, TestAccessOrder::RANDOM), /* access order. */
        ::testing::Values(1, 3) /* jump between rows/columns. */
    );
}

/**
 * @cond
 */
namespace internal {

template<typename Value_>
void compare_vectors(const std::vector<Value_>& expected, const std::vector<Value_>& observed, const std::string& context) {
    size_t n_expected = expected.size();
    ASSERT_EQ(n_expected, observed.size()) << "mismatch in vector length (" << context << ")";
    for (size_t i = 0; i < n_expected; ++i) {
        auto expected_val = expected[i], observed_val = observed[i];
        if (std::isnan(expected_val)) {
            EXPECT_EQ(std::isnan(expected_val), std::isnan(observed_val)) << "mismatching NaNs at position " << i << " (" << context << ")";
        } else {
            EXPECT_EQ(expected_val, observed_val) << "different values at position " << i << " (" << context << ")";
        }
    }
}

template<typename Index_>
uint64_t create_seed(Index_ NR, Index_ NC, const TestAccessOptions& options) {
    uint64_t seed = static_cast<uint64_t>(NR) * static_cast<uint64_t>(NC);
    seed += 13 * static_cast<uint64_t>(options.use_row);
    seed += 57 * static_cast<uint64_t>(options.order);
    seed += 101 * static_cast<uint64_t>(options.jump);
    return seed;
}

template<typename Index_>
std::vector<Index_> simulate_test_access_sequence(Index_ NR, Index_ NC, const TestAccessOptions& options) {
    std::vector<Index_> sequence;
    auto limit = (options.use_row ? NR : NC);

    if (options.order == TestAccessOrder::REVERSE) {
        for (int i = limit; i > 0; i -= options.jump) {
            sequence.push_back(i - 1);
        }
    } else {
        for (int i = 0; i < limit; i += options.jump) {
            sequence.push_back(i);
        }
        if (options.order == TestAccessOrder::RANDOM) {
            std::mt19937_64 rng(create_seed(NR, NC, options));
            std::shuffle(sequence.begin(), sequence.end(), rng);
        }
    }

    return sequence;
}

template<bool use_oracle_, typename Index_>
tatami::MaybeOracle<use_oracle_, Index_> create_oracle(const std::vector<Index_>& sequence, const TestAccessOptions& options) {
    if constexpr(use_oracle_) {
        std::shared_ptr<tatami::Oracle<Index_> > oracle;
        if (options.jump == 1 && options.order == TestAccessOrder::FORWARD) {
            oracle.reset(new tatami::ConsecutiveOracle<Index_>(0, sequence.size()));
        } else {
            oracle.reset(new tatami::FixedViewOracle<Index_>(sequence.data(), sequence.size()));
        }
        return oracle;
    } else {
        return false;
    }
}

template<bool use_oracle_, typename Value_, typename Index_, class SparseExpand_, typename ...Args_>
void test_access_base(
    const tatami::Matrix<Value_, Index_>& matrix, 
    const tatami::Matrix<Value_, Index_>& reference, 
    const TestAccessOptions& options, 
    Index_ extent,
    SparseExpand_ sparse_expand, 
    Args_... args) 
{
    auto NR = matrix.nrow();
    ASSERT_EQ(NR, reference.nrow());
    auto NC = matrix.ncol();
    ASSERT_EQ(NC, reference.ncol());

    auto refwork = (options.use_row ? reference.dense_row(args...) : reference.dense_column(args...));

    auto sequence = simulate_test_access_sequence(NR, NC, options);
    auto oracle = create_oracle<use_oracle_>(sequence, options);

    auto pwork = tatami::new_extractor<false, use_oracle_>(&matrix, options.use_row, oracle, args...);
    auto swork = tatami::new_extractor<true, use_oracle_>(&matrix, options.use_row, oracle, args...);

    tatami::Options opt;
    opt.sparse_extract_index = false;
    auto swork_v = tatami::new_extractor<true, use_oracle_>(&matrix, options.use_row, oracle, args..., opt);

    opt.sparse_extract_value = false;
    auto swork_n = tatami::new_extractor<true, use_oracle_>(&matrix, options.use_row, oracle, args..., opt);

    opt.sparse_extract_index = true;
    auto swork_i = tatami::new_extractor<true, use_oracle_>(&matrix, options.use_row, oracle, args..., opt);

    size_t sparse_counter = 0;

    // Looping over rows/columns and checking extraction against the reference.
    for (auto i : sequence) {
        auto expected = fetch(*refwork, i, extent);

        // Checking dense retrieval first.
        {
            auto observed = [&]() {
                if constexpr(use_oracle_) {
                    return fetch(*pwork, extent);
                } else {
                    return fetch(*pwork, i, extent);
                }
            }();
            compare_vectors(expected, observed, "dense retrieval");
        }

        // Various flavors of sparse retrieval.
        {
            auto observed = [&]() {
                if constexpr(use_oracle_) {
                    return fetch(*swork, extent);
                } else {
                    return fetch(*swork, i, extent);
                }
            }();
            compare_vectors(expected, sparse_expand(observed), "sparse retrieval");

            sparse_counter += observed.value.size();
            {
                bool is_increasing = true;
                for (size_t i = 1; i < observed.index.size(); ++i) {
                    if (observed.index[i] <= observed.index[i-1]) {
                        is_increasing = false;
                        break;
                    }
                }
                ASSERT_TRUE(is_increasing);
            }

            std::vector<Index_> indices(extent);
            auto observed_i = [&]() {
                if constexpr(use_oracle_) {
                    return swork_i->fetch(NULL, indices.data());
                } else {
                    return swork_i->fetch(i, NULL, indices.data());
                }
            }();
            ASSERT_TRUE(observed_i.value == NULL);
            tatami::copy_n(observed_i.index, observed_i.number, indices.data());
            indices.resize(observed_i.number);
            ASSERT_EQ(observed.index, indices);

            std::vector<Value_> values(extent);
            auto observed_v = [&]() {
                if constexpr(use_oracle_) {
                    return swork_v->fetch(values.data(), NULL);
                } else {
                    return swork_v->fetch(i, values.data(), NULL);
                }
            }();
            ASSERT_TRUE(observed_v.index == NULL);
            tatami::copy_n(observed_v.value, observed_v.number, values.data());
            values.resize(observed_v.number);
            compare_vectors(values, observed.value, "sparse retrieval with values only");

            auto observed_n = [&]() {
                if constexpr(use_oracle_) {
                    return swork_n->fetch(NULL, NULL);
                } else {
                    return swork_n->fetch(i, NULL, NULL);
                }
            }();
            ASSERT_TRUE(observed_n.value == NULL);
            ASSERT_TRUE(observed_n.index == NULL);
            ASSERT_EQ(observed.value.size(), observed_n.number);
        } 
    }

    if (options.check_sparse && matrix.is_sparse()) {
        EXPECT_TRUE(sparse_counter < static_cast<size_t>(NR) * static_cast<size_t>(NC));
    }
}

template<bool use_oracle_, typename Value_, typename Index_>
void test_full_access(
    const tatami::Matrix<Value_, Index_>& matrix, 
    const tatami::Matrix<Value_, Index_>& reference,
    const TestAccessOptions& options)
{
    Index_ nsecondary = (options.use_row ? reference.ncol() : reference.nrow());
    test_access_base<use_oracle_>(
        matrix,
        reference,
        options,
        nsecondary,
        [&](const auto& svec) -> auto {
            std::vector<Value_> output(nsecondary);
            size_t nnz = svec.index.size();
            for (size_t i = 0; i < nnz; ++i) {
                output[svec.index[i]] = svec.value[i];
            }
            return output;
        }
    );
}

template<bool use_oracle_, typename Value_, typename Index_>
void test_block_access(
    const tatami::Matrix<Value_, Index_>& matrix, 
    const tatami::Matrix<Value_, Index_>& reference,
    double relative_start,
    double relative_length,
    const TestAccessOptions& options)
{
    Index_ nsecondary = (options.use_row ? reference.ncol() : reference.nrow());
    Index_ start = nsecondary * relative_start;
    Index_ length = nsecondary * relative_length;
    test_access_base<use_oracle_>(
        matrix, 
        reference, 
        options,
        length,
        [&](const auto& svec) -> auto {
            std::vector<Value_> output(length);
            size_t nnz = svec.index.size();
            for (size_t i = 0; i < nnz; ++i) {
                output[svec.index[i] - start] = svec.value[i];
            }
            return output;
        },
        start,
        length
    );
}

template<typename Index_>
std::vector<Index_> create_indexed_subset(Index_ nsecondary, double relative_start, double probability, uint64_t base_seed) {
    Index_ start = nsecondary * relative_start;
    if (start >= nsecondary) {
        return std::vector<Index_>();
    }

    std::vector<Index_> indices { start };
    std::mt19937_64 rng(base_seed + 999 * probability + 888 * start);
    std::uniform_real_distribution udist;
    for (Index_ i = start + 1; i < nsecondary; ++i) {
        if (udist(rng) < probability) {
            indices.push_back(i);
        }
    }
    return indices;
}

template<bool use_oracle_, typename Value_, typename Index_>
void test_indexed_access(
    const tatami::Matrix<Value_, Index_>& matrix, 
    const tatami::Matrix<Value_, Index_>& reference,
    double relative_start,
    double probability,
    const TestAccessOptions& options)
{
    Index_ nsecondary = (options.use_row ? reference.ncol() : reference.nrow());
    auto indices = create_indexed_subset(nsecondary, relative_start, probability, create_seed(matrix.nrow(), matrix.ncol(), options));
    std::vector<size_t> reposition(nsecondary, -1);
    for (size_t i = 0, end = indices.size(); i < end; ++i) {
        reposition[indices[i]] = i;
    }

    test_access_base<use_oracle_>(
        matrix,
        reference,
        options,
        static_cast<Index_>(indices.size()),
        [&](const auto& svec) -> auto {
            std::vector<Value_> expected(indices.size());
            size_t nnz = svec.index.size();
            for (size_t i = 0; i < nnz; ++i) {
                expected[reposition[svec.index[i]]] = svec.value[i];
            }
            return expected;
        },
        indices
    );
}

}
/**
 * @endcond
 */

/**
 * Test access to the full extent of a row/column.
 * Any discrepancies between `matrix` and `reference` will raise a GoogleTest error.
 *
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param matrix Matrix for which to test access.
 * @param reference Reference matrix containing the same values as `matrix`.
 * This typically uses a "known-good" representation like a `tatami::DenseRowMatrix`.
 * @param options Further options for testing.
 */
template<typename Value_, typename Index_>
void test_full_access(
    const tatami::Matrix<Value_, Index_>& matrix,
    const tatami::Matrix<Value_, Index_>& reference,
    const TestAccessOptions& options)
{
    if (options.use_oracle) {
        internal::test_full_access<true>(matrix, reference, options);
    } else {
        internal::test_full_access<false>(matrix, reference, options);
    }
}

/**
 * Test access to a contiguous block of a row/column.
 * Any discrepancies between `matrix` and `reference` will raise a GoogleTest error.
 *
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param matrix Matrix for which to test access.
 * @param reference Reference matrix containing the same values as `matrix`.
 * This typically uses a "known-good" representation like a `tatami::DenseRowMatrix`.
 * @param relative_start Start of the block, as a proportion of the extent of the non-target dimension.
 * The (floored) product of this value and the non-target extent is used as the index of the first row/column of the block.
 * This should lie in `[0, 1)`.
 * @param relative_length Length of the block, as a proportion of the extent of the non-target dimension.
 * This should lie in `[0, 1)`, and the sum of `relative_start` and `relative_length` should be no greater than 1.
 * The (floored) product of this value and the non-target extent is used as the number of rows/columns in the block.
 * @param options Further options for testing.
 */
template<typename Value_, typename Index_>
void test_block_access(
    const tatami::Matrix<Value_, Index_>& matrix, 
    const tatami::Matrix<Value_, Index_>& reference,
    double relative_start,
    double relative_length,
    const TestAccessOptions& options)
{
    if (options.use_oracle) {
        internal::test_block_access<true>(matrix, reference, relative_start, relative_length, options);
    } else {
        internal::test_block_access<false>(matrix, reference, relative_start, relative_length, options);
    }
}

/**
 * Test access to an indexed subset of a row/column.
 * Any discrepancies between `matrix` and `reference` will raise a GoogleTest error.
 *
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param matrix Matrix for which to test access.
 * @param reference Reference matrix containing the same values as `matrix`.
 * This typically uses a "known-good" representation like a `tatami::DenseRowMatrix`.
 * @param relative_start Start of the indexed subset, as a proportion of the extent of the non-target dimension.
 * The (floored) product of this value and the non-target extent is used as the index of the first row/column in the indexed subset.
 * This should lie in `[0, 1)`.
 * @param probability Probability of sampling rows/columns when simulating the indexed subset.
 * This should lie in `[0, 1]`.
 * Only rows/columns after the first index (as defined by `relative_start`) are considered.
 * @param options Further options for testing.
 */
template<typename Value_, typename Index_>
void test_indexed_access(
    const tatami::Matrix<Value_, Index_>& matrix,
    const tatami::Matrix<Value_, Index_>& reference,
    double relative_start,
    double probability,
    const TestAccessOptions& options)
{
    if (options.use_oracle) {
        internal::test_indexed_access<true>(matrix, reference, relative_start, probability, options);
    } else {
        internal::test_indexed_access<false>(matrix, reference, relative_start, probability, options);
    }
}

/**
 * Equivalent to `test_full_access()` with `TestAccessOptions::use_row = false`.
 * All other options are set to their defaults.
 * This is intended for quick testing of matrix access when a full parametrized test suite is not required.
 *
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param matrix Matrix for which to test access.
 * @param reference Reference matrix containing the same values as `matrix`.
 * This typically uses a "known-good" representation like a `tatami::DenseRowMatrix`.
 */
template<typename Value_, typename Index_>
void test_simple_column_access(const tatami::Matrix<Value_, Index_>& matrix, const tatami::Matrix<Value_, Index_>& reference) {
    TestAccessOptions options;
    options.use_row = false;
    test_full_access(matrix, reference, options);
}

/**
 * Equivalent to `test_full_access()` with `TestAccessOptions::use_row = true`.
 * All other options are set to their defaults.
 * This is intended for quick testing of matrix access when a full parametrized test suite is not required.
 *
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param matrix Matrix for which to test access.
 * @param reference Reference matrix containing the same values as `matrix`.
 * This typically uses a "known-good" representation like a `tatami::DenseRowMatrix`.
 */
template<typename Value_, typename Index_>
void test_simple_row_access(const tatami::Matrix<Value_, Index_>& matrix, const tatami::Matrix<Value_, Index_>& reference) {
    TestAccessOptions options;
    options.use_row = false;
    test_full_access(matrix, reference, options);
}

}

#endif
