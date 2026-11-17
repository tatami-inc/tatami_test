#ifndef TATAMI_TEST_CREATE_SEED_HPP
#define TATAMI_TEST_CREATE_SEED_HPP

#include <tuple>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <algorithm>

#include "utils.hpp"

/**
 * @file create_seed.hpp
 * @brief create a seed.
 */

namespace tatami_test {

/**
 * @cond
 */
// Copied from the old boost::hash_combine.
// SeedType should be unsigned and at least 64 bits so this shouldn't overflow.
inline SeedType combine_hash(SeedType old_seed, SeedType val) {
    static_assert(std::numeric_limits<SeedType>::digits >= 64);
    static_assert(std::is_unsigned<SeedType>::value);
    return val + 0x9e3779b9 + (old_seed<<6) + (old_seed>>2);
}

inline SeedType combine_hash(SeedType old_seed, int val) {
    // SeedType should also be unsigned, so integer casts to SeedType are always well-defined.
    return combine_hash(old_seed, static_cast<SeedType>(val));
}

inline SeedType combine_hash(SeedType old_seed, double val) {
    SeedType dummy;
    std::copy_n(reinterpret_cast<const char*>(&val), std::min(sizeof(SeedType), sizeof(val)), reinterpret_cast<char*>(&dummy));
    return combine_hash(old_seed, dummy);
}

inline SeedType combine_hash(SeedType old_seed, bool val) {
    return combine_hash(old_seed, static_cast<SeedType>(val * 57));
}

template<typename Type_>
SeedType combine_hash(SeedType old_seed, const std::vector<Type_>& val) {
    for (const auto& x : val) {
        old_seed = combine_hash(old_seed, x);
    }
    return old_seed;
}

template<typename First_, typename Second_>
SeedType combine_hash(SeedType old_seed, const std::pair<First_, Second_>& val) {
    return combine_hash(combine_hash(old_seed, val.first), val.second);
}

template<std::size_t member_ = 0, typename ... Args_>
SeedType combine_hash(SeedType old_seed, const std::tuple<Args_...>& val) {
    if constexpr(member_ == std::tuple_size_v<I<decltype(val)> >) {
        return old_seed;
    } else {
        return combine_hash<member_ + 1>(
            combine_hash(old_seed, std::get<member_>(val)),
            val
        );
    }
}

template<typename First_, typename Second_, typename ... Rest_>
SeedType combine_hash(SeedType old_seed, const First_& first, const Second_& second, Rest_&&... rest) {
    return combine_hash(combine_hash(old_seed, first), second, std::forward<Rest_>(rest)...);
}
/**
 * @endcond
 */

/**
 * Create an RNG seed from some input values, typically test parameters.
 * This provides deterministic variation in the RNG stream across test scenarios.
 *
 * @tparam Args_ One or more integer, floating-point or boolean types;
 * or any `std::pair`, `std::tuple` or `std::vector` consisting of any combination of such types;
 * or a `TestAccessOptions` object, using its `use_oracle`, `order` and `jump` members.
 *
 * @param args At least one value.
 *
 * @return A seed for `RngEngine`.
 */
template<typename... Args_>
SeedType create_seed(Args_&&... args) {
    return combine_hash(6259, std::forward<Args_>(args)...);
}

}

#endif
