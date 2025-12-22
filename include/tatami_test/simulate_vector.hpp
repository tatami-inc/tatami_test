#ifndef TATAMI_TEST_SIMULATE_VECTOR_HPP
#define TATAMI_TEST_SIMULATE_VECTOR_HPP

#include <random>
#include <vector>
#include <cstddef>

#include "sanisizer/sanisizer.hpp"

#include "utils.hpp"

/**
 * @file simulate_vector.hpp
 * @brief Simulate a random vector.
 */

namespace tatami_test {

/**
 * @brief Options for `simulate_vector()`.
 */
struct SimulateVectorOptions {
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
    double density = 1;

    /**
     * Seed for the PRNG.
     */
    SeedType seed = 1234567890;
};

/**
 * Simulate a vector of values from a uniform distribution.
 *
 * @tparam Type_ Type of value to be simulated.
 * @tparam Length_ Integer type of the length of the output vector.
 *
 * @param length Length of the array of values to simulate.
 * @param options Simulation options.
 *
 * @return Vector of simulated values.
 */
template<typename Type_, typename Length_>
std::vector<Type_> simulate_vector(const Length_ length, const SimulateVectorOptions& options) {
    auto output = sanisizer::create<std::vector<Type_> >(length);
    RngEngine rng(options.seed);
    std::uniform_real_distribution<> unif(options.lower, options.upper);

    if (options.density == 1) {
        for (auto& v : output) {
            v = unif(rng);
        }
    } else {
        std::uniform_real_distribution<> nonzero(0.0, 1.0);
        for (auto& v : output) {
            if (nonzero(rng) <= options.density) {
                v = unif(rng);
            }
        }
    }

    return output;
}

/**
 * Overload of `simulate_vector()` for simulating the contents of a dense random matrix. 
 *
 * @tparam Type_ Type of value to be simulated.
 * @tparam Index_ Integer type of the dimension extents.
 *
 * @param nrow Number of rows in the matrix.
 * @param ncol Number of columns in the matrix.
 * @param options Simulation options.
 *
 * @return Vector of simulated values of length equal to the product of `nrow` and `ncol`.
 */
template<typename Type_, typename Index_>
std::vector<Type_> simulate_vector(const Index_ nrow, const Index_ ncol, const SimulateVectorOptions& options) {
    return simulate_vector<Type_>(sanisizer::product<typename std::vector<Type_>::size_type>(nrow, ncol), options);
}

}

#endif
