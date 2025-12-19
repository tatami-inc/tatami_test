#ifndef TATAMI_TEST_TEST_UNSORTED_ACCESS_HPP
#define TATAMI_TEST_TEST_UNSORTED_ACCESS_HPP

#include <gtest/gtest.h>

#include <vector>
#include <limits>
#include <random>
#include <cmath>

#include "tatami/utils/new_extractor.hpp"
#include "tatami/utils/ConsecutiveOracle.hpp"
#include "tatami/utils/FixedOracle.hpp"

#include "create_indexed_subset.hpp"
#include "test_access.hpp"

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
void test_unsorted_access_base(const tatami::Matrix<Value_, Index_>& matrix, const TestAccessOptions& options, const Index_ extent, Args_... args) {
    const auto NR = matrix.nrow();
    const auto NC = matrix.ncol();

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

    sanisizer::as_size_type<std::vector<Value_> >(extent);
    std::vector<Value_> mat_vbuffer(extent), mat_vstore1(extent), mat_vstore2(extent); 
    sanisizer::as_size_type<std::vector<Index_> >(extent);
    std::vector<Index_> mat_ibuffer(extent), mat_istore1(extent), mat_istore2(extent);

    std::vector<std::pair<Index_, Value_> > collected;
    std::vector<Value_> sorted_v;
    std::vector<Index_> sorted_i;

    for (const auto i : sequence) {
        // Regular sparse retrieval.
        {
            std::fill(mat_ibuffer.begin(), mat_ibuffer.end(), 0);
            std::fill(mat_vbuffer.begin(), mat_vbuffer.end(), 0);
            const auto vbuf = mat_vbuffer.data();
            const auto ibuf = mat_ibuffer.data();

            auto observed = [&]() {
                if constexpr(use_oracle_) {
                    return swork->fetch(vbuf, ibuf);
                } else {
                    return swork->fetch(i, vbuf, ibuf);
                }
            }();

            mat_vstore1.clear();
            mat_vstore1.insert(mat_vstore1.end(), observed.value, observed.value + observed.number);
            mat_istore1.clear();
            mat_istore1.insert(mat_istore1.end(), observed.index, observed.index + observed.number);
        }

        // Unsorted sparse retrieval with both values and indices.
        {
            std::fill(mat_ibuffer.begin(), mat_ibuffer.end(), 0);
            std::fill(mat_vbuffer.begin(), mat_vbuffer.end(), 0);
            const auto vbuf = mat_vbuffer.data();
            const auto ibuf = mat_ibuffer.data();

            auto observed_uns = [&]() {
                if constexpr(use_oracle_) {
                    return swork_uns->fetch(vbuf, ibuf);
                } else {
                    return swork_uns->fetch(i, vbuf, ibuf);
                }
            }();

            // Poor man's zip + unzip with sorting.
            collected.clear();
            for (I<decltype(observed_uns.number)> i = 0; i < observed_uns.number; ++i) {
                collected.emplace_back(observed_uns.index[i], observed_uns.value[i]);
            }
            std::sort(collected.begin(), collected.end());
            sorted_i.clear();
            sorted_v.clear();
            for (const auto& p : collected) {
                sorted_i.push_back(p.first);
                sorted_v.push_back(p.second);
            }
            ASSERT_EQ(mat_istore1, sorted_i);
            compare_vectors(mat_vstore1, sorted_v, "unsorted sparse");

            mat_vstore1.clear();
            mat_vstore1.insert(mat_vstore1.end(), observed_uns.value, observed_uns.value + observed_uns.number);
            mat_istore1.clear();
            mat_istore1.insert(mat_istore1.end(), observed_uns.index, observed_uns.index + observed_uns.number);
        }

        // Unsorted sparse retrieval with indices.
        {
            std::fill(mat_ibuffer.begin(), mat_ibuffer.end(), 0);
            const auto ibuf = mat_ibuffer.data();

            auto observed_i = [&]() {
                if constexpr(use_oracle_) {
                    return swork_uns_i->fetch(static_cast<Value_*>(NULL), ibuf);
                } else {
                    return swork_uns_i->fetch(i, static_cast<Value_*>(NULL), ibuf);
                }
            }();

            ASSERT_TRUE(observed_i.value == NULL);
            mat_istore2.clear();
            mat_istore2.insert(mat_istore2.end(), observed_i.index, observed_i.index + observed_i.number);
            ASSERT_EQ(mat_istore1, mat_istore2);
        }

        // Unsorted sparse retrieval with values.
        {
            std::fill(mat_vbuffer.begin(), mat_vbuffer.end(), 0);
            const auto vbuf = mat_vbuffer.data();

            auto observed_v = [&]() {
                if constexpr(use_oracle_) {
                    return swork_uns_v->fetch(vbuf, static_cast<Index_*>(NULL));
                } else {
                    return swork_uns_v->fetch(i, vbuf, static_cast<Index_*>(NULL));
                }
            }();

            ASSERT_TRUE(observed_v.index == NULL);
            mat_vstore2.clear();
            mat_vstore2.insert(mat_vstore2.end(), observed_v.value, observed_v.value + observed_v.number);
            compare_vectors(mat_vstore1, mat_vstore2, "unsorted sparse, values only");
        }

        {
            auto observed_n = [&]() {
                if constexpr(use_oracle_) {
                    return swork_uns_n->fetch(static_cast<Value_*>(NULL), static_cast<Index_*>(NULL));
                } else {
                    return swork_uns_n->fetch(i, static_cast<Value_*>(NULL), static_cast<Index_*>(NULL));
                }
            }();

            ASSERT_TRUE(observed_n.value == NULL);
            ASSERT_TRUE(observed_n.index == NULL);
            ASSERT_EQ(mat_vstore1.size(), observed_n.number);
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
    const Index_ nsecondary = (options.use_row ? matrix.ncol() : matrix.nrow());
    auto index_ptr = create_indexed_subset(
        nsecondary,
        relative_start,
        probability,
        static_cast<SeedType>(
            create_seed(matrix.nrow(), matrix.ncol(), options)
            + static_cast<SeedType>(1001 * probability)
            + static_cast<SeedType>(13 * relative_start)
        )
    );
    const Index_ num_indices = index_ptr->size();
    internal::test_unsorted_access_base<use_oracle_>(matrix, options, num_indices, std::move(index_ptr));
}

#ifndef TATAMI_STRICT_SIGNATURES
template<bool use_oracle_, typename Value_, typename Index_, typename ... Args_>
void test_unsorted_access_base(const tatami::Matrix<Value_, Index_>&, Args_...) = delete;

template<bool use_oracle_, typename Value_, typename Index_, typename ... Args_>
void test_unsorted_full_access(const tatami::Matrix<Value_, Index_>&, Args_...) = delete;

template<bool use_oracle_, typename Value_, typename Index_, typename ... Args_>
void test_unsorted_block_access(const tatami::Matrix<Value_, Index_>&, Args_...) = delete;

template<bool use_oracle_, typename Value_, typename Index_, typename ... Args_>
void test_unsorted_indexed_access(const tatami::Matrix<Value_, Index_>&, Args_...) = delete;
#endif

}
/**
 * @endcond
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

/**
 * @cond
 */
#ifndef TATAMI_STRICT_SIGNATURES
template<typename Value_, typename Index_, typename ... Args_>
void test_unsorted_full_access(const tatami::Matrix<Value_, Index_>&, Args_...) = delete;

template<typename Value_, typename Index_, typename ... Args_>
void test_unsorted_block_access(const tatami::Matrix<Value_, Index_>&, Args_...) = delete;

template<typename Value_, typename Index_, typename ... Args_>
void test_unsorted_indexed_access(const tatami::Matrix<Value_, Index_>&, Args_...) = delete;
#endif
/**
 * @endcond
 */

}

#endif
