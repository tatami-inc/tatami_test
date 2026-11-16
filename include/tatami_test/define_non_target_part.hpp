#ifndef TATAMI_TEST_DEFINE_DIMENSION_PART_HPP
#define TATAMI_TEST_DEFINE_DIMENSION_PART_HPP

#include <vector>
#include <random>
#include <optional>
#include <algorithm>
#include <cassert>

#include "tatami/base/Matrix.hpp"

#include "utils.hpp"

/**
 * @file define_non_target_part.hpp
 * @brief Define a part of the non-target dimension.
 */

namespace tatami_test {

/**
 * Create a random subset of sorted and unique indices, typically corresponding to elements of the non-target dimension.
 *
 * @tparam Index_ Integer type of the dimension extent.
 *
 * @param start Start position of the indices.
 * @param max Open upper bound of the indices.
 * This should be no less than `start`.
 * @param probability Sampling probability.
 * This should lie in `[0, 1]`.
 * @param seed Seed for random number generation.
 * If unset, a suitable value is generated from `start`, `max` and `probability`.
 *
 * @return Pointer to a vector of sorted and unique indices. 
 * If `[start, max)` is not empty, this is guaranteed to contain at least one element, starting with `start`.
 */
template<typename Index_>
tatami::VectorPtr<Index_> define_non_target_indices(const Index_ start, const Index_ max, const double probability, const std::optional<SeedType>& seed) {
    auto ptr = new std::vector<Index_>;
    tatami::VectorPtr<Index_> output(ptr);
    if (start == max) {
        return output;
    }
    assert(start < max);

    SeedType seed0;
    if (seed.has_value()) {
        seed0 = *seed;
    } else  {
        seed0 = static_cast<SeedType>(start) * 13u +
            static_cast<SeedType>(max) * 117u +
            probability * 1234567890u;
    }

    RngEngine rng(seed0);
    ptr->push_back(start);
    std::uniform_real_distribution udist;
    for (Index_ i = start + 1; i < max; ++i) {
        if (udist(rng) < probability) {
            ptr->push_back(i);
        }
    }

    return output;
}

/**
 * Overload of `define_non_target_indices()` that accepts a relative start/max position on the non-target dimension.
 *
 * @tparam Index_ Integer type of the dimension extent.
 *
 * @param extent Extent of the non-target dimension.
 * @param relative_start Start of the indices, as a proportion of the extent of the non-target dimension.
 * Specifically, the (floored) product of this value and `extent` is used as the start position.
 * This should lie in `[0, 1]`.
 * @param relative_max Open upper bound of the indices, as a proportion of the extent of the non-target dimension.
 * Specifically, the (floored) product of this value and `extent` is used as the upper bound.
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
    const double relative_max,
    const double probability,
    const std::optional<SeedType>& seed
) {
    return define_non_target_indices<Index_>(relative_start * extent, relative_max * extent, probability, seed);
}

/**
 * Overload of `define_non_target_indices()` that determines the non-target dimension extent of a matrix.
 *
 * @tparam Value_ Type of matrix value.
 * @tparam Index_ Integer type of the dimension extent.
 *
 * @param matrix Matrix of interst.
 * @param use_row Whether the rows of `matrix` are the target dimension.
 * @param relative_start Start of the indices, as a proportion of the extent of the non-target dimension.
 * Specifically, the (floored) product of this value and `extent` is used as the start position.
 * This should lie in `[0, 1]`.
 * @param relative_max Open upper bound of the indices, as a proportion of the extent of the non-target dimension.
 * Specifically, the (floored) product of this value and `extent` is used as the upper bound.
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
    const double relative_max,
    const double probability,
    const std::optional<SeedType>& seed
) {
    const Index_ nsecondary = (use_row ? matrix.ncol() : matrix.nrow());
    return define_non_target_indices(nsecondary, relative_start, relative_max, probability, seed);
}

/**
 * @cond
 */
// For back-compatibility.
template<typename Index_>
tatami::VectorPtr<Index_> create_indexed_subset(const Index_ extent, const double relative_start, const double probability, const SeedType seed) {
    return define_non_target_indices(extent, relative_start, 1, probability, seed);
}
/**
 * @endcond
 */

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
 */
template<typename Index_>
std::pair<Index_, Index_> define_non_target_block(
    const Index_ extent,
    const double relative_start,
    const double relative_length
) {
    return std::pair<Index_, Index_>(extent * relative_start, extent * relative_length);
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
 * Specifically, the (floored) product of this value and `extent` is used as the start position.
 * This should lie in `[0, 1]`.
 * @param relative_length Length of the block, as a proportion of the extent of the non-target dimension.
 * Specifically, the (floored) product of this value and `extent` is used as the length.
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

}

#endif
