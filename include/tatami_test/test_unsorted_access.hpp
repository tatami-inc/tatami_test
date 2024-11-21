#ifndef TATAMI_TEST_TEST_UNSORTED_ACCESS_HPP
#define TATAMI_TEST_TEST_UNSORTED_ACCESS_HPP

#include <gtest/gtest.h>

#include "tatami/utils/new_extractor.hpp"
#include "tatami/utils/ConsecutiveOracle.hpp"
#include "tatami/utils/FixedOracle.hpp"

#include "fetch.hpp"
#include "test_access.hpp"

#include <vector>
#include <limits>
#include <random>
#include <cmath>

/**
 * @file test_unsorted_access.hpp
 * @brief Test unsorted sparse access on a `tatami::Matrix`.
 */

namespace tatami_test {

/**
 * @cond
 */
namespace internal {

template<bool use_oracle_, typename Value_, typename Index_, typename ...Args_>
void test_unsorted_access_base(const tatami::Matrix<Value_, Index_>& matrix, const TestAccessOptions& options, Index_ extent, Args_... args) {
    auto NR = matrix.nrow();
    auto NC = matrix.ncol();

    auto sequence = simulate_test_access_sequence(NR, NC, options);
    auto oracle = create_oracle<use_oracle_>(sequence, options);
    auto swork = tatami::new_extractor<true, use_oracle_>(&matrix, options.use_row, oracle, args...);

    tatami::Options opt;
    opt.sparse_ordered_index = false;
    auto swork_uns = tatami::new_extractor<true, use_oracle_>(&matrix, options.use_row, oracle, args..., opt);

    opt.sparse_extract_index = false;
    auto swork_uns_v = tatami::new_extractor<true, use_oracle_>(&matrix, options.use_row, oracle, args..., opt);

    opt.sparse_extract_value = false;
    auto swork_uns_n = tatami::new_extractor<true, use_oracle_>(&matrix, options.use_row, oracle, args..., opt);

    opt.sparse_extract_index = true;
    auto swork_uns_i = tatami::new_extractor<true, use_oracle_>(&matrix, options.use_row, oracle, args..., opt);

    // Looping over rows/columns and checking extraction for various unsorted combinations.
    for (auto i : sequence) {
        auto observed = [&]() {
            if constexpr(use_oracle_) {
                return fetch(*swork, extent);
            } else {
                return fetch(*swork, i, extent);
            }
        }();

        auto observed_uns = [&]() {
            if constexpr(use_oracle_) {
                return fetch(*swork_uns, extent);
            } else {
                return fetch(*swork_uns, i, extent);
            }
        }();

        {
            // Poor man's zip + unzip.
            std::vector<std::pair<Index_, Value_> > collected;
            collected.reserve(observed.value.size());
            for (Index_ i = 0, end = observed_uns.value.size(); i < end; ++i) {
                collected.emplace_back(observed_uns.index[i], observed_uns.value[i]);
            }
            std::sort(collected.begin(), collected.end());

            std::vector<Value_> sorted_v;
            std::vector<Index_> sorted_i;
            sorted_v.reserve(collected.size());
            sorted_i.reserve(collected.size());
            for (const auto& p : collected) {
                sorted_i.push_back(p.first);
                sorted_v.push_back(p.second);
            }

            ASSERT_EQ(observed.index, sorted_i);
            compare_vectors(observed.value, sorted_v, "unsorted sparse");
        }

        {
            std::vector<int> indices(extent);
            auto observed_i = [&]() {
                if constexpr(use_oracle_) {
                    return swork_uns_i->fetch(NULL, indices.data());
                } else {
                    return swork_uns_i->fetch(i, NULL, indices.data());
                }
            }();
            ASSERT_TRUE(observed_i.value == NULL);

            tatami::copy_n(observed_i.index, observed_i.number, indices.data());
            indices.resize(observed_i.number);
            ASSERT_EQ(observed_uns.index, indices);
        }

        {
            std::vector<double> values(extent);
            auto observed_v = [&]() {
                if constexpr(use_oracle_) {
                    return swork_uns_v->fetch(values.data(), NULL);
                } else {
                    return swork_uns_v->fetch(i, values.data(), NULL);
                }
            }();
            ASSERT_TRUE(observed_v.index == NULL);

            tatami::copy_n(observed_v.value, observed_v.number, values.data());
            values.resize(observed_v.number);
            compare_vectors(observed_uns.value, values, "unsorted sparse, values only");
        }

        {
            auto observed_n = [&]() {
                if constexpr(use_oracle_) {
                    return swork_uns_n->fetch(NULL, NULL);
                } else {
                    return swork_uns_n->fetch(i, NULL, NULL);
                }
            }();
            ASSERT_TRUE(observed_n.value == NULL);
            ASSERT_TRUE(observed_n.index == NULL);
            ASSERT_EQ(observed.value.size(), observed_n.number);
        }
    }
}

template<bool use_oracle_, typename Value_, typename Index_>
void test_unsorted_full_access(const tatami::Matrix<Value_, Index_>& matrix, const TestAccessOptions& options) {
    Index_ nsecondary = (options.use_row ? matrix.ncol() : matrix.nrow());
    internal::test_unsorted_access_base<use_oracle_>(matrix, options, nsecondary);
}

template<bool use_oracle_, typename Value_, typename Index_>
void test_unsorted_block_access(const tatami::Matrix<Value_, Index_>& matrix, double relative_start, double relative_length, const TestAccessOptions& options) {
    Index_ nsecondary = (options.use_row ? matrix.ncol() : matrix.nrow());
    Index_ start = nsecondary * relative_start;
    Index_ length = nsecondary * relative_length;
    internal::test_unsorted_access_base<use_oracle_>(matrix, options, nsecondary, start, length);
}

template<bool use_oracle_, typename Value_, typename Index_>
void test_unsorted_indexed_access(const tatami::Matrix<Value_, Index_>& matrix, double relative_start, double probability, const TestAccessOptions& options) {
    Index_ nsecondary = (options.use_row ? matrix.ncol() : matrix.nrow());
    auto indices = create_indexed_subset(nsecondary, relative_start, probability, create_seed(matrix.nrow(), matrix.ncol(), options));
    internal::test_unsorted_access_base<use_oracle_>(matrix, options, static_cast<Index_>(indices.size()), indices);
}

}
/**
 * @cond
 */

/**
 * Test unsorted sparse access to the full extent of a row/column.
 * Any discrepancies between sorted and unsorted accesses on `matrix` will raise a GoogleTest error.
 * This is intended for `tatami::Matrix` subclasses where `tatami::Options::sparse_ordered_index = false` has an effect.
 * Subclasses implementing delayed operations should consider tests with `UnorderedWrapper` instances to check that unordered access in the seed is handled correctly.
 *
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param matrix Matrix for which to test access.
 * @param options Further options for testing.
 */
template<typename Value_, typename Index_>
void test_unsorted_full_access(const tatami::Matrix<Value_, Index_>& matrix, const TestAccessOptions& options) {
    if (options.use_oracle) {
        internal::test_unsorted_full_access<true>(matrix, options);
    } else {
        internal::test_unsorted_full_access<false>(matrix, options);
    }
}

/**
 * Test unsorted sparse access to a contiguous block of a row/column.
 * Any discrepancies between sorted and unsorted accesses on `matrix` will raise a GoogleTest error.
 * This is intended for `tatami::Matrix` subclasses where `tatami::Options::sparse_ordered_index = false` has an effect.
 * Subclasses implementing delayed operations should consider tests with `UnorderedWrapper` instances to check that unordered access in the seed is handled correctly.
 *
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param matrix Matrix for which to test access.
 * @param relative_start Start of the block, as a proportion of the extent of the non-target dimension.
 * The (floored) product of this value and the non-target extent is used as the index of the first row/column of the block.
 * This should lie in `[0, 1)`.
 * @param relative_length Length of the block, as a proportion of the extent of the non-target dimension.
 * This should lie in `[0, 1)`, and the sum of `relative_start` and `relative_length` should be no greater than 1.
 * The (floored) product of this value and the non-target extent is used as the number of rows/columns in the block.
 * @param options Further options for testing.
 */
template<typename Value_, typename Index_>
void test_unsorted_block_access(const tatami::Matrix<Value_, Index_>& matrix, double relative_start, double relative_length, const TestAccessOptions& options) {
    if (options.use_oracle) {
        internal::test_unsorted_block_access<true>(matrix, relative_start, relative_length, options);
    } else {
        internal::test_unsorted_block_access<false>(matrix, relative_start, relative_length, options);
    }
}

/**
 * Test unsorted sparse access to an indexed subset of a row/column.
 * Any discrepancies between sorted and unsorted accesses on `matrix` will raise a GoogleTest error.
 * This function is intended for `tatami::Matrix` subclasses where `tatami::Options::sparse_ordered_index = false` has an effect.
 * Subclasses implementing delayed operations should consider tests with `UnorderedWrapper` instances to check that unordered access in the seed is handled correctly.
 *
 * @tparam Value_ Type of the data.
 * @tparam Index_ Integer type for the row/column index.
 *
 * @param matrix Matrix for which to test access.
 * @param relative_start Start of the indexed subset, as a proportion of the extent of the non-target dimension.
 * The (floored) product of this value and the non-target extent is used as the index of the first row/column in the indexed subset.
 * This should lie in `[0, 1)`.
 * @param probability Probability of sampling rows/columns when simulating the indexed subset.
 * This should lie in `[0, 1]`.
 * Only rows/columns after the first index (as defined by `relative_start`) are considered.
 * @param options Further options for testing.
 */
template<typename Value_, typename Index_>
void test_unsorted_indexed_access(const tatami::Matrix<Value_, Index_>& matrix, double relative_start, double probability, const TestAccessOptions& options) {
    if (options.use_oracle) {
        internal::test_unsorted_indexed_access<true>(matrix, relative_start, probability, options);
    } else {
        internal::test_unsorted_indexed_access<false>(matrix, relative_start, probability, options);
    }
}

}

#endif
