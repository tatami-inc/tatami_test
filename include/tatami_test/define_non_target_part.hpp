#ifndef TATAMI_TEST_DEFINE_DIMENSION_PART_HPP
#define TATAMI_TEST_DEFINE_DIMENSION_PART_HPP

#include <vector>
#include <random>
#include <optional>
#include <algorithm>
#include <cassert>

#include "tatami/base/Matrix.hpp"
#include "sanisizer/sanisizer.hpp"

#include "create_seed.hpp"
#include "utils.hpp"

/**
 * @file define_non_target_part.hpp
 * @brief Define a part of the non-target dimension.
 */

namespace tatami_test {

/**
 * Create a block of a non-target dimension, given its relative start and length.
 *
 * @tparam Index_ Integer type of the dimension extent.
 *
 * @param extent Extent of the non-target dimension.
 * @param relative_start Start of the block, as a proportion of the extent of the non-target dimension.
 * Specifically, the (floored) product of this value and `extent` is used as the start position.
 * This should lie in `[0, 1]`.
 * @param relative_length Length of the block, as a proportion of the extent of the non-target dimension.
 * Specifically, the (floored) product of this value and `extent` is used as the length.
 * This should lie in `[0, 1]`.
 *
 * @return Pair containing the start and length of the block.
 * The start is guaranteed to be less than `extent` unless `extent = 0` in which case the start is also zero.
 * The sum of the start and length is guaranteed to be less than or equal to `extent`.
 */
template<typename Index_>
std::pair<Index_, Index_> define_non_target_block(
    const Index_ extent,
    const double relative_start,
    const double relative_length
) {
    if (!extent) {
        return std::pair<Index_, Index_>(0, 0);
    } else {
        // Use rounding to avoid excessive truncation of near-integer values.
        // Note that this requires some care to ensure that start + length does not exceed extent;
        // instead, we compute the rounded endpoint, and then subtract the start to get the length.
        Index_ actual_start = std::min(extent - 1, static_cast<Index_>(std::round(relative_start * extent)));
        Index_ actual_end = std::min(extent, static_cast<Index_>(std::round((relative_start + relative_length) * extent)));
        return std::pair<Index_, Index_>(actual_start, actual_end - actual_start);
    }
}

/**
 * Overload of `define_non_target_block()` that determines the non-target dimension extent of a matrix.
 *
 * @tparam Value_ Type of matrix value.
 * @tparam Index_ Integer type of the dimension extent.
 *
 * @param matrix Matrix of interst.
 * @param use_row Whether the rows of `matrix` are the target dimension.
 * @param relative_start Start of the block, as a proportion of the extent of the non-target dimension.
 * This should lie in `[0, 1]`.
 * @param relative_length Length of the block, as a proportion of the extent of the non-target dimension.
 * This should lie in `[0, 1]`.
 *
 * @return Pair containing the start and length of the block.
 */
template<typename Value_, typename Index_>
std::pair<Index_, Index_> define_non_target_block(
    const tatami::Matrix<Value_, Index_>& matrix,
    bool use_row,
    const double relative_start,
    const double relative_length
) {
    const Index_ nsecondary = (use_row ? matrix.ncol() : matrix.nrow());
    return define_non_target_block(nsecondary, relative_start, relative_length);
}


/**
 * Create a random subset of sorted and unique indices, typically corresponding to elements of the non-target dimension.
 *
 * @tparam Index_ Integer type of the dimension extent.
 *
 * @param start Start of range from which to sample indices.
 * @param length Length of the range from which to sample indices.
 * @param probability Sampling probability.
 * This should lie in `[0, 1]`.
 * @param seed Seed for random number generation.
 * If unset, a suitable value is generated from `start`, `length` and `probability`.
 *
 * @return Pointer to a vector of sorted and unique indices. 
 * If `length > 0`, this is guaranteed to contain at least one element,
 * where the first and last elements are guaranteed to be `start` and `start + length - 1`, respectively.
 */
template<typename Index_>
tatami::VectorPtr<Index_> define_non_target_indices(const Index_ start, const Index_ length, const double probability, const std::optional<SeedType>& seed) {
    auto ptr = new std::vector<Index_>;
    tatami::VectorPtr<Index_> output(ptr);
    if (length == 0) {
        return output;
    }
    const Index_ max = start + length;

    SeedType seed0;
    if (seed.has_value()) {
        seed0 = *seed;
    } else  {
        seed0 = create_seed(start, length, probability);
    }

    RngEngine rng(seed0);
    ptr->push_back(start);
    std::uniform_real_distribution udist;
    for (Index_ i = start + 1; i < max; ++i) {
        if (udist(rng) < probability) {
            ptr->push_back(i);
        }
    }
    if (ptr->back() != static_cast<Index_>(max - 1)) {
        ptr->push_back(max - 1);
    }

    return output;
}

/**
 * Overload of `define_non_target_indices()` that accepts a relative start/max position on the non-target dimension.
 *
 * @tparam Index_ Integer type of the dimension extent.
 *
 * @param extent Extent of the non-target dimension.
 * @param relative_start Start of the range from which to sample indices, as a proportion of the extent of the non-target dimension.
 * This should lie in `[0, 1)`.
 * @param relative_length Length of the range from which to sample indices, as a proportion of the extent of the non-target dimension.
 * This should lie in `[0, 1]`.
 * @param probability Sampling probability.
 * This should lie in `[0, 1]`.
 * @param seed Seed for random number generation.
 *
 * @return Pointer to a vector of indices, see `define_non_target_indices()` for details.
 */
template<typename Index_>
tatami::VectorPtr<Index_> define_non_target_indices(
    const Index_ extent,
    const double relative_start,
    const double relative_length,
    const double probability,
    const std::optional<SeedType>& seed
) {
    auto converted = define_non_target_block(extent, relative_start, relative_length);
    return define_non_target_indices(converted.first, converted.second, probability, seed);
}

/**
 * Overload of `define_non_target_indices()` that determines the non-target dimension extent of a matrix.
 *
 * @tparam Value_ Type of matrix value.
 * @tparam Index_ Integer type of the dimension extent.
 *
 * @param matrix Matrix of interst.
 * @param use_row Whether the rows of `matrix` are the target dimension.
 * @param relative_start Start of the range from which to sample indices, as a proportion of the extent of the non-target dimension.
 * This should lie in `[0, 1)`.
 * @param relative_length Length of the range from which to sample indices, as a proportion of the extent of the non-target dimension.
 * This should lie in `[0, 1]`.
 * @param probability Sampling probability.
 * This should lie in `[0, 1]`.
 * @param seed Seed for random number generation.
 *
 * @return Pointer to a vector of indices, see `define_non_target_indices()` for details.
 */
template<typename Value_, typename Index_>
tatami::VectorPtr<Index_> define_non_target_indices(
    const tatami::Matrix<Value_, Index_>& matrix,
    bool use_row,
    const double relative_start,
    const double relative_length,
    const double probability,
    const std::optional<SeedType>& seed
) {
    const Index_ nsecondary = (use_row ? matrix.ncol() : matrix.nrow());
    return define_non_target_indices(nsecondary, relative_start, relative_length, probability, seed);
}

/**
 * @cond
 */
// For back-compatibility.
template<typename Index_>
tatami::VectorPtr<Index_> create_indexed_subset(const Index_ extent, const double relative_start, const double probability, const SeedType seed) {
    return define_non_target_indices(extent, relative_start, 1 - relative_start, probability, seed);
}
/**
 * @endcond
 */

/** 
 * @cond
 */
template<typename Value_, typename Index_>
bool check_block(const tatami::Matrix<Value_, Index_>& matrix, bool use_row, Index_ start, Index_ length) {
    const Index_ nsecondary = (use_row ? matrix.ncol() : matrix.nrow());
    return start >= 0 && 
        (start < nsecondary || (nsecondary == 0 && start == 0)) &&
        length >= 0
        && sanisizer::is_less_than_or_equal(length, nsecondary - start);
}

template<typename Value_, typename Index_>
bool check_indices(const tatami::Matrix<Value_, Index_>& matrix, bool use_row, const tatami::VectorPtr<Index_>& indices_ptr) {
    if (!indices_ptr) {
        return false;
    }

    const auto num_indices = indices_ptr->size();
    if (num_indices == 0) {
        return true;
    }

    auto previous = indices_ptr->front();
    if (previous < 0) {
        return false;
    }

    for (I<decltype(num_indices)> i = 1; i < num_indices; ++i) {
        if ((*indices_ptr)[i] <= previous) {
            return false;
        }
        previous = (*indices_ptr)[i];
    }
    const Index_ nsecondary = (use_row ? matrix.ncol() : matrix.nrow());
    return indices_ptr->back() < nsecondary;
}
/** 
* @endcond
*/

}

#endif
