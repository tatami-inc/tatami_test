#ifndef TATAMI_TEST_SIMULATE_COMPRESSED_SPARSE_HPP
#define TATAMI_TEST_SIMULATE_COMPRESSED_SPARSE_HPP

#include <random>
#include <vector>
#include <cstdint>

/**
 * @file simulate_compressed_sparse.hpp
 * @brief Simulate compressed sparse matrix contents.
 */

namespace tatami_test {

/**
 * @brief Options for `simulate_compressed_sparse()`.
 */
struct SimulateCompressedSparseOptions {
    /**
     * Lower bound on the simulated values.
     */
    double lower = 0;

    /**
     * Upper bound on the simulated values.
     */
    double upper = 100;

    /**
     * Density of non-zero values for the simulated values.
     */
    double density = 0.1;

    /**
     * Seed for the PRNG.
     */
    uint64_t seed = 1234567890;
};

/**
 * @brief Result of `simulate_compressed_sparse()`.
 *
 * @tparam Value_ Type of simulated value. 
 * @tparam Index_ Integer type for the index.
 */
template<typename Value_, typename Index_>
struct SimulateCompressedSparseResult {
    /**
     * Vector of values of the non-zero elements.
     * Each entry corresponds to an entry of `SimulateCompressedSparseResult::index`.
     */
    std::vector<Value_> data;

    /**
     * Vector of indices of the non-zero elements.
     * Indices refer the location of the non-zero element on the secondary dimension.
     * Non-zero elements in the same primary dimension element are stored contiguously, ordered by increasing index.
     */
    std::vector<Index_> index;

    /**
     * Vector of length equal to the extent of the primary dimension plus 1.
     * This contains positions on `index` that define the start and end of each primary dimension element.
     * Specifically, the stretch of entries in `index` from `[indptr[i], indptr[i+1])` contains non-zero elements for the primary dimension element `i`.
     */
    std::vector<size_t> indptr;
};

/**
 * Simulate values in a compressed sparse matrix.
 *
 * @tparam Value_ Type of simulated value. 
 * @tparam Index_ Integer type for the index.
 *
 * @param primary Extent of the primary dimension, i.e., the dimension used to compress non-zero elements.
 * @param secondary Extent of the secondary dimension.
 * @param options Simulation options.
 *
 * @return Simulated values that can be used to construct a compressed sparse matrix.
 */
template<typename Value_, typename Index_>
SimulateCompressedSparseResult<Value_, Index_> simulate_compressed_sparse(size_t primary, size_t secondary, const SimulateCompressedSparseOptions& options) {
    std::mt19937_64 rng(options.seed);
    std::uniform_real_distribution<> nonzero(0.0, 1.0);
    std::uniform_real_distribution<> unif(options.lower, options.upper);

    SimulateCompressedSparseResult<Value_, Index_> output;
    output.indptr.resize(primary + 1);
    for (size_t p = 0; p < primary; ++p) {
        size_t idx = output.indptr[p];
        for (size_t s = 0; s < secondary; ++s) {
            if (nonzero(rng) < options.density) {
                output.data.push_back(unif(rng));
                output.index.push_back(s);
                ++idx;
            }
        }
        output.indptr[p + 1] = idx;
    }

    return output;
}

}

#endif
