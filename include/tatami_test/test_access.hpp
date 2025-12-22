#ifndef TATAMI_TEST_TEST_ACCESS_HPP
#define TATAMI_TEST_TEST_ACCESS_HPP

#include <gtest/gtest.h>

#include <vector>
#include <limits>
#include <random>
#include <cmath>
#include <memory>
#include <cstdint>
#include <type_traits>

#include "tatami/utils/new_extractor.hpp"
#include "tatami/utils/ConsecutiveOracle.hpp"
#include "tatami/utils/FixedOracle.hpp"

#include "create_indexed_subset.hpp"
#include "utils.hpp"

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
    const auto n_expected = expected.size();
    ASSERT_EQ(n_expected, observed.size()) << "mismatch in vector length (" << context << ")";
    for (I<decltype(n_expected)> i = 0; i < n_expected; ++i) {
        auto expected_val = expected[i], observed_val = observed[i];
        if (std::isnan(expected_val)) {
            EXPECT_EQ(std::isnan(expected_val), std::isnan(observed_val)) << "mismatching NaNs at position " << i << " (" << context << ")";
        } else {
            EXPECT_EQ(expected_val, observed_val) << "different values at position " << i << " (" << context << ")";
        }
    }
}

template<typename Value_>
void compare_vectors(const std::vector<Value_>& expected, const std::vector<Value_>& observed, const char* context) {
    compare_vectors(expected, observed, std::string(context));
}

template<typename Index_>
SeedType create_seed(const Index_ NR, const Index_ NC, const TestAccessOptions& options) {
    SeedType seed = static_cast<SeedType>(NR) * static_cast<SeedType>(NC);
    seed += 13 * static_cast<SeedType>(options.use_row);
    seed += 57 * static_cast<SeedType>(options.order);
    seed += 101 * static_cast<SeedType>(options.jump);
    return seed;
}

template<typename Index_>
std::vector<Index_> simulate_test_access_sequence(const Index_ NR, const Index_ NC, const TestAccessOptions& options) {
    std::vector<Index_> sequence;
    const auto limit = (options.use_row ? NR : NC);

    RngEngine rng(create_seed(NR, NC, options));
    Index_ start = rng() % options.jump;
    if (start < limit) {
        while (1) {
            sequence.push_back(start);
            const Index_ remainder = limit - start;
            // Make sure this comparison involves two unsigned integers to avoid GCC warnings.
            if (sanisizer::is_less_than_or_equal(remainder, options.jump)) {
                break;
            }
            start += options.jump;
        }
    }

    if (options.order == TestAccessOrder::REVERSE) {
        std::reverse(sequence.begin(), sequence.end());
    } else if (options.order == TestAccessOrder::RANDOM) {
        std::shuffle(sequence.begin(), sequence.end(), rng);
    }

    return sequence;
}

template<bool use_oracle_, typename Index_>
tatami::MaybeOracle<use_oracle_, Index_> create_oracle(const std::vector<Index_>& sequence, const TestAccessOptions& options) {
    if constexpr(use_oracle_) {
        std::shared_ptr<tatami::Oracle<Index_> > oracle;
        if (options.jump == 1 && options.order == TestAccessOrder::FORWARD) {
            oracle.reset(new tatami::ConsecutiveOracle<Index_>(0, sanisizer::Cast(sequence.size())));
        } else {
            oracle.reset(new tatami::FixedViewOracle<Index_>(sequence.data(), sanisizer::Cast(sequence.size())));
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
    const Index_ extent,
    const SparseExpand_ sparse_expand, 
    Args_... args
) {
    const auto NR = matrix.nrow();
    ASSERT_EQ(NR, reference.nrow());
    const auto NC = matrix.ncol();
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

    sanisizer::as_size_type<std::vector<Value_> >(extent);
    std::vector<Value_> mat_dense_buffer(extent), ref_dense_buffer(extent), mat_vbuffer(extent), mat_vstore1(extent), mat_vstore2(extent); 
    sanisizer::as_size_type<std::vector<Index_> >(extent);
    std::vector<Index_> mat_ibuffer(extent), mat_istore1(extent), mat_istore2(extent);
    bool has_sparse = false;

    for (const auto i : sequence) {
        {
            std::fill(ref_dense_buffer.begin(), ref_dense_buffer.end(), 0);
            const auto ref_buf = ref_dense_buffer.data();
            const auto ref_ptr = refwork->fetch(Fix(i), ref_buf);
            tatami::copy_n(ref_ptr, extent, ref_buf);
        }

        // Dense retrieval. 
        {
            std::fill(mat_dense_buffer.begin(), mat_dense_buffer.end(), 0);
            const auto mat_buf = mat_dense_buffer.data();
            const auto mat_ptr = [&]() {
                if constexpr(use_oracle_) {
                    return pwork->fetch(mat_buf);
                } else {
                    return pwork->fetch(Fix(i), mat_buf);
                }
            }();
            tatami::copy_n(mat_ptr, extent, mat_buf);
            compare_vectors(ref_dense_buffer, mat_dense_buffer, "dense retrieval");
        }

        // Sparse retrieval with both values and indices.
        {
            std::fill(mat_vbuffer.begin(), mat_vbuffer.end(), 0);
            std::fill(mat_ibuffer.begin(), mat_ibuffer.end(), 0);
            const auto vbuf = mat_vbuffer.data();
            const auto ibuf = mat_ibuffer.data();

            const auto observed = [&]() {
                if constexpr(use_oracle_) {
                    return swork->fetch(vbuf, ibuf);
                } else {
                    return swork->fetch(Fix(i), vbuf, ibuf);
                }
            }();
            compare_vectors(ref_dense_buffer, sparse_expand(observed), "sparse retrieval");

            if (!has_sparse && sanisizer::is_less_than(observed.number, extent)) {
                has_sparse = true;
            }

            bool is_increasing = true;
            for (I<decltype(observed.number)> i = 1; i < observed.number; ++i) {
                if (observed.index[i] <= observed.index[i-1]) {
                    is_increasing = false;
                    break;
                }
            }
            ASSERT_TRUE(is_increasing);

            mat_vstore1.clear();
            mat_vstore1.insert(mat_vstore1.end(), observed.value, observed.value + observed.number);
            mat_istore1.clear();
            mat_istore1.insert(mat_istore1.end(), observed.index, observed.index + observed.number);
        }

        // Sparse retrieval with indices only.
        {
            std::fill(mat_ibuffer.begin(), mat_ibuffer.end(), 0);
            const auto ibuf = mat_ibuffer.data();

            auto observed_i = [&]() {
                if constexpr(use_oracle_) {
                    return swork_i->fetch(NULL, ibuf);
                } else {
                    return swork_i->fetch(Fix(i), NULL, ibuf);
                }
            }();

            ASSERT_TRUE(observed_i.value == NULL);
            mat_istore2.clear();
            mat_istore2.insert(mat_istore2.end(), observed_i.index, observed_i.index + observed_i.number);
            ASSERT_EQ(mat_istore1, mat_istore2);
        }

        // Sparse retrieval with values only.
        {
            std::fill(mat_vbuffer.begin(), mat_vbuffer.end(), 0);
            const auto vbuf = mat_vbuffer.data();

            auto observed_v = [&]() {
                if constexpr(use_oracle_) {
                    return swork_v->fetch(vbuf, NULL);
                } else {
                    return swork_v->fetch(Fix(i), vbuf, NULL);
                }
            }();

            ASSERT_TRUE(observed_v.index == NULL);
            mat_vstore2.clear();
            mat_vstore2.insert(mat_vstore2.end(), observed_v.value, observed_v.value + observed_v.number);
            compare_vectors(mat_vstore1, mat_vstore2, "sparse retrieval with values only");
        }

        // Sparse retrieval with neither indices or values.
        {
            auto observed_n = [&]() {
                if constexpr(use_oracle_) {
                    return swork_n->fetch(NULL, NULL);
                } else {
                    return swork_n->fetch(Fix(i), NULL, NULL);
                }
            }();

            ASSERT_TRUE(observed_n.value == NULL);
            ASSERT_TRUE(observed_n.index == NULL);
            ASSERT_EQ(mat_vstore1.size(), observed_n.number);
        } 
    }

    if (options.check_sparse && matrix.is_sparse()) {
        EXPECT_TRUE(has_sparse);
    }
}

template<bool use_oracle_, typename Value_, typename Index_>
void test_full_access(
    const tatami::Matrix<Value_, Index_>& matrix, 
    const tatami::Matrix<Value_, Index_>& reference,
    const TestAccessOptions& options
) {
    const Index_ nsecondary = (options.use_row ? reference.ncol() : reference.nrow());
    auto expected = sanisizer::create<std::vector<Value_> >(nsecondary);

    test_access_base<use_oracle_>(
        matrix,
        reference,
        options,
        nsecondary,
        [&](const tatami::SparseRange<Value_, Index_>& svec) -> const std::vector<Value_>& {
            std::fill(expected.begin(), expected.end(), 0);
            for (I<decltype(svec.number)> i = 0; i < svec.number; ++i) {
                expected[svec.index[i]] = svec.value[i];
            }
            return expected;
        }
    );
}

template<bool use_oracle_, typename Value_, typename Index_>
void test_block_access(
    const tatami::Matrix<Value_, Index_>& matrix, 
    const tatami::Matrix<Value_, Index_>& reference,
    const double relative_start,
    const double relative_length,
    const TestAccessOptions& options
) {
    const Index_ nsecondary = (options.use_row ? reference.ncol() : reference.nrow());
    const Index_ start = nsecondary * relative_start;
    const Index_ length = nsecondary * relative_length;
    auto expected = sanisizer::create<std::vector<Value_> >(length);

    test_access_base<use_oracle_>(
        matrix, 
        reference, 
        options,
        length,
        [&](const tatami::SparseRange<Value_, Index_>& svec) -> const std::vector<Value_>& {
            std::fill(expected.begin(), expected.end(), 0);
            for (I<decltype(svec.number)> i = 0; i < svec.number; ++i) {
                expected[svec.index[i] - start] = svec.value[i];
            }
            return expected;
        },
        start,
        length
    );
}

template<bool use_oracle_, typename Value_, typename Index_>
void test_indexed_access(
    const tatami::Matrix<Value_, Index_>& matrix, 
    const tatami::Matrix<Value_, Index_>& reference,
    const double relative_start,
    const double probability,
    const TestAccessOptions& options
) {
    const Index_ nsecondary = (options.use_row ? reference.ncol() : reference.nrow());
    auto index_ptr = create_indexed_subset(
        nsecondary,
        relative_start,
        probability,
        static_cast<SeedType>(
            create_seed(matrix.nrow(), matrix.ncol(), options)
            + static_cast<SeedType>(999 * probability)
            + static_cast<SeedType>(85 * relative_start)
        )
    );

    const Index_ num_indices = index_ptr->size();
    constexpr std::size_t placeholder = -1;
    auto reposition = sanisizer::create<std::vector<std::size_t> >(nsecondary, placeholder);
    {
        const auto& indices = *index_ptr;
        for (Index_ i = 0; i < num_indices; ++i) {
            reposition[indices[i]] = i;
        }
    }

    auto expected = sanisizer::create<std::vector<Value_> >(num_indices);

    test_access_base<use_oracle_>(
        matrix,
        reference,
        options,
        num_indices,
        [&](const tatami::SparseRange<Value_, Index_>& svec) -> const std::vector<Value_>& {
            std::fill(expected.begin(), expected.end(), 0);
            for (I<decltype(svec.number)> i = 0; i < svec.number; ++i) {
                expected[reposition[svec.index[i]]] = svec.value[i];
            }
            return expected;
        },
        std::move(index_ptr)
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
    const TestAccessOptions& options
) {
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
    const TestAccessOptions& options
) {
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
    const TestAccessOptions& options
) {
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
