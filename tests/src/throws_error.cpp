#include "tatami_test/throws_error.hpp"

TEST(ThrowsError, Basic) {
    tatami_test::throws_error([]() { throw std::runtime_error("oops"); }, "oops");
}
