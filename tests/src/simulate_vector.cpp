#include <gtest/gtest.h>

#include "tatami_test/simulate_vector.hpp"

TEST(SimulateVector, Dense) {
    {
        auto res = tatami_test::simulate_vector<double>(1000, []{ 
            tatami_test::SimulateVectorOptions opt;
            opt.lower = 0;
            opt.upper = 10;
            return opt;
        }());
        for (auto x : res) {
            EXPECT_GE(x, 0);
            EXPECT_LE(x, 10);
        }
    }

    {
        auto res = tatami_test::simulate_vector<double>(1000, []{ 
            tatami_test::SimulateVectorOptions opt;
            opt.lower = -100;
            opt.upper = -20;
            return opt;
        }());
        for (auto x : res) {
            EXPECT_GE(x, -100);
            EXPECT_LE(x, -20);
        }
    }

    {
        auto res = tatami_test::simulate_vector<double>(1000, []{ 
            tatami_test::SimulateVectorOptions opt;
            opt.seed = 12345;
            return opt;
        }());
        auto res2 = tatami_test::simulate_vector<double>(1000, []{ 
            tatami_test::SimulateVectorOptions opt;
            opt.seed = 67890;
            return opt;
        }());
        EXPECT_NE(res, res2);
    }
}

TEST(SimulateVector, Sparse) {
    {
        auto res = tatami_test::simulate_vector<double>(1000, []{ 
            tatami_test::SimulateVectorOptions opt;
            opt.density = 0;
            return opt;
        }());
        EXPECT_EQ(res, std::vector<double>(1000));
    }

    {
        auto res = tatami_test::simulate_vector<double>(1000, []{ 
            tatami_test::SimulateVectorOptions opt;
            opt.lower = -10;
            opt.upper = 10;
            opt.density = 0.1;
            return opt;
        }());
        int count = 0;
        for (auto x : res) {
            EXPECT_GE(x, -10);
            EXPECT_LE(x, 10);
            count += (x != 0);
        }
        EXPECT_GT(count, 0);
        EXPECT_LT(count, res.size() * 0.2);
    }
}
