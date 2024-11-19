#ifndef TATAMI_TEST_SIMULATE_VECTOR_HPP
#define TATAMI_TEST_SIMULATE_VECTOR_HPP

#include <random>
#include <vector>
#include <cstdint>

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
    uint64_t seed = 1234567890;
};

/**
 * Simulate a vector of values from a uniform distribution.
 *
 * @tparam Type_ Type of value to be simulated.
 * @param length Length of the array of values to simulate.
 * @param options Simulation options.
 *
 * @return Vector of simulated values.
 */
template<typename Type_>
std::vector<Type_> simulate_vector(size_t length, const SimulateVectorOptions& options) {
    std::vector<Type_> output(length);
    std::mt19937_64 rng(options.seed);
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

}

#endif
