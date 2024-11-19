#include <gtest/gtest.h>

#include <algorithm>
#include "tatami_test/simulate_compressed_sparse.hpp"

TEST(SimulateCompressedSparse, Basic) {
    {
        size_t primary = 20, secondary = 50;
        auto res = tatami_test::simulate_compressed_sparse<double, int>(primary, secondary, []{ 
            tatami_test::SimulateCompressedSparseOptions opt;
            opt.lower = 0;
            opt.upper = 10;
            return opt;
        }());

        EXPECT_EQ(res.indptr.back(), res.data.size());
        EXPECT_EQ(res.indptr.back(), res.index.size());

        for (auto x : res.data) {
            EXPECT_GE(x, 0);
            EXPECT_LE(x, 10);
        }

        for (size_t p = 0; p < primary; ++p) {
            auto pstart = res.indptr[p], pend = res.indptr[p + 1];
            EXPECT_TRUE(std::is_sorted(res.index.begin() + pstart, res.index.begin() + pend));

            for (size_t s = pstart; s < pend; ++s) {
                EXPECT_GE(res.index[s], 0);
                EXPECT_LT(res.index[s], secondary);
            }
        }
    }

    {
        size_t primary = 40, secondary = 25;
        auto res = tatami_test::simulate_compressed_sparse<double, int>(primary, secondary, []{ 
            tatami_test::SimulateCompressedSparseOptions opt;
            opt.lower = -100;
            opt.upper = -20;
            return opt;
        }());

        EXPECT_EQ(res.indptr.back(), res.data.size());
        EXPECT_EQ(res.indptr.back(), res.index.size());

        for (auto x : res.data) {
            EXPECT_GE(x, -100);
            EXPECT_LE(x, -20);
        }

        for (size_t p = 0; p < primary; ++p) {
            auto pstart = res.indptr[p], pend = res.indptr[p + 1];
            EXPECT_TRUE(std::is_sorted(res.index.begin() + pstart, res.index.begin() + pend));

            for (size_t s = pstart; s < pend; ++s) {
                EXPECT_GE(res.index[s], 0);
                EXPECT_LT(res.index[s], secondary);
            }
        }
    }

    {
        size_t primary = 30, secondary = 40;
        auto res = tatami_test::simulate_compressed_sparse<double, int>(primary, secondary, []{ 
            tatami_test::SimulateCompressedSparseOptions opt;
            opt.seed = 12345;
            return opt;
        }());
        auto res2 = tatami_test::simulate_compressed_sparse<double, int>(primary, secondary, []{ 
            tatami_test::SimulateCompressedSparseOptions opt;
            opt.seed = 67890;
            return opt;
        }());
        EXPECT_NE(res.data, res2.data);
        EXPECT_NE(res.index, res2.index);
        EXPECT_NE(res.indptr, res2.indptr);
    }
}
