#ifndef TATAMI_TEST_CREATE_INDEXED_SUBSET_HPP
#define TATAMI_TEST_CREATE_INDEXED_SUBSET_HPP

#include <vector>
#include <random>
#include <cstdint>

#include "tatami/tatami.hpp"

/**
 * @file create_indexed_subset.hpp
 * @brief Create an indexed subset of dimension elements.
 */

namespace tatami_test {

/**
 * Create a random subset of sorted and unique indices, typically corresponding to elements of the non-target dimension.
 *
 * @tparam Index_ Integer type for the dimension elements.
 *
 * @param extent Extent of the non-target dimension.
 * @param relative_start Start of the indexed subset, as a proportion of the extent of the non-target dimension.
 * The (floored) product of this value and `extent` is used as the index of the first element in the indexed subset.
 * This should lie in `[0, 1)`.
 * @param probability Probability of sampling elements into the indexed subset.
 * This should lie in `[0, 1]`.
 * Only elements after the first index (as defined by `relative_start`) are considered.
 * @param seed Seed for the PRNG.
 *
 * @return Pointer to a vector of indices of sampled elements.
 * The vector will be empty if `extent = 0`, otherwise it is guaranteed to contain at least one element corresponding to `relative_start`.
 */
template<typename Index_>
tatami::VectorPtr<Index_> create_indexed_subset(Index_ extent, double relative_start, double probability, uint64_t seed) {
    auto ptr = new std::vector<Index_>;
    tatami::VectorPtr<Index_> output(ptr);

    Index_ start = extent * relative_start;
    if (start < extent) {
        auto& indices = *ptr;
        indices.push_back(start);
        std::mt19937_64 rng(seed);
        std::uniform_real_distribution udist;
        for (Index_ i = start + 1; i < extent; ++i) {
            if (udist(rng) < probability) {
                indices.push_back(i);
            }
        }
    }

    return output;
}

}

#endif
