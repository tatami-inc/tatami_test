#ifndef TATAMI_TEST_UTILS_HPP
#define TATAMI_TEST_UTILS_HPP

#include <type_traits>
#include <random>

/**
 * @file utils.hpp
 * @brief Miscellaneous utilities.
 */

namespace tatami_test {

/**
 * @cond
 */
template<typename Input_>
using I = std::remove_reference_t<std::remove_cv_t<Input_> >;
/**
 * @endcond
 */

/**
 * Type of the pseudo-random number generator. 
 */
typedef std::mt19937_64 RngEngine;


/**
 * Type of the seed for `RngEngine`.
 */
typedef typename RngEngine::result_type SeedType;

}

#endif
