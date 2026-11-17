#include "gtest/gtest.h"

#include "tatami_test/create_seed.hpp"

TEST(CreateSeed, Single) {
    EXPECT_GT(tatami_test::create_seed(0), 0);
    EXPECT_NE(tatami_test::create_seed(0), tatami_test::create_seed(1));

    EXPECT_GT(tatami_test::create_seed(0.0), 0);
    EXPECT_NE(tatami_test::create_seed(0.0), tatami_test::create_seed(1.0));

    EXPECT_GT(tatami_test::create_seed(false), 0);
    EXPECT_NE(tatami_test::create_seed(false), tatami_test::create_seed(true));
}

TEST(CreateSeed, Containers) {
    EXPECT_GT(tatami_test::create_seed(std::make_pair(0, 1)), 0);
    EXPECT_NE(tatami_test::create_seed(std::make_pair(0, 1)), tatami_test::create_seed(std::make_pair(1, 0)));

    EXPECT_GT(tatami_test::create_seed(std::vector{ 0, 1, 2 }), 0);
    EXPECT_NE(tatami_test::create_seed(std::vector{ 0, 1, 2 }), tatami_test::create_seed(std::vector{ 2, 1, 0 }));

    EXPECT_GT(tatami_test::create_seed(std::make_tuple(0.0, 1, true)), 0);
    EXPECT_NE(tatami_test::create_seed(std::make_tuple(0.0, 1, true)), tatami_test::create_seed(std::make_tuple(0, 1.0, 1)));
}

TEST(CreateSeed, Multiple) {
    EXPECT_GT(tatami_test::create_seed(0.0, 1, true), 0);
    EXPECT_NE(tatami_test::create_seed(0.0, 1, true), tatami_test::create_seed(std::make_tuple(true, 1, 0.0)));

    EXPECT_GT(tatami_test::create_seed(0.0, 1, std::vector<double>{ 2.0, 1.5 }), 0);
    EXPECT_NE(
        tatami_test::create_seed(0.0, 1, std::vector<double>{ 2.0, 1.5 }), 
        tatami_test::create_seed(std::vector<double>{ 2.0, 1.5 }, 0.0, 1)
    );
}
