#include "gtest/gtest.h"

#include "tatami_test/define_non_target_part.hpp"
#include "tatami/tatami.hpp"

TEST(DefineNonTargetBlock, Basic) {
    const int NR = 52, NC = 231;
    tatami::DenseMatrix<double, int, std::vector<double> > mat(NR, NC, std::vector<double>(NR * NC), true);

    auto block = tatami_test::define_non_target_block(mat, true, 0.2, 0.6);
    EXPECT_EQ(block.first, std::round(0.2 * NC));
    EXPECT_EQ(block.second, std::round(0.8 * NC) - std::round(0.2 * NC));

    auto block2 = tatami_test::define_non_target_block(mat, false, 0.4, 0.3);
    EXPECT_EQ(block2.first, std::round(0.4 * NR));
    EXPECT_EQ(block2.second, std::round(0.7 * NR) - std::round(0.4 * NR));

    auto empty = tatami_test::define_non_target_block(0, 0.2, 0.6);
    EXPECT_EQ(empty.first, 0);
    EXPECT_EQ(empty.second, 0);

    auto limited = tatami_test::define_non_target_block(NC, 0.99999, 0.1);
    EXPECT_EQ(limited.first, NC - 1);
    EXPECT_EQ(limited.second, 1);
}

static void check_indices(const std::vector<int>& target) {
    const std::size_t end = target.size();
    for (std::size_t i = 1; i < end; ++i) {
        ASSERT_GT(target[i], target[i-1]);
    }
}

TEST(DefineNonTargetIndices, Basic) {
    const int NR = 199, NC = 99;
    tatami::DenseMatrix<double, int, std::vector<double> > mat(NR, NC, std::vector<double>(NR * NC), true);

    auto indices_ptr = tatami_test::define_non_target_indices(mat, true, 0.1, 0.8, 0.2, 12345);
    ASSERT_GT(indices_ptr->size(), 0);
    check_indices(*indices_ptr);
    auto block = tatami_test::define_non_target_block(mat, true, 0.1, 0.8);
    EXPECT_EQ(indices_ptr->front(), block.first);
    EXPECT_EQ(indices_ptr->back(), block.first + block.second - 1);

    // Auto-fill the seed.
    {
        auto indices_ptr2 = tatami_test::define_non_target_indices(mat, true, 0.1, 0.8, 0.2, {});
        ASSERT_GT(indices_ptr2->size(), 0);
        EXPECT_EQ(indices_ptr->front(), block.first);
        EXPECT_EQ(indices_ptr->back(), block.first + block.second - 1);
        EXPECT_NE(*indices_ptr, *indices_ptr2);
        check_indices(*indices_ptr2);
    }

    // Trying on the other dimension.
    {
        auto indices_ptr2 = tatami_test::define_non_target_indices(mat, false, 0.4, 0.4, 0.33, 67890);
        ASSERT_GT(indices_ptr2->size(), 0);
        check_indices(*indices_ptr2);
        auto block = tatami_test::define_non_target_block(mat, false, 0.4, 0.4);
        EXPECT_EQ(indices_ptr2->front(), block.first);
        EXPECT_EQ(indices_ptr2->back(), block.first + block.second - 1);
    }

    // Single-element range.
    {
        auto indices_ptr2 = tatami_test::define_non_target_indices(90, 1, 0.33, {});
        EXPECT_EQ(indices_ptr2->size(), 1);
        EXPECT_EQ(indices_ptr2->front(), 90);
    }

    // Empty range.
    {
        auto indices_ptr2 = tatami_test::define_non_target_indices(mat, false, 0.5, 0., 0.33, {});
        EXPECT_TRUE(indices_ptr2->empty());
    }
}

TEST(CheckBlock, Basic) {
    const int NR = 52, NC = 31;
    tatami::DenseMatrix<double, int, std::vector<double> > mat(NR, NC, std::vector<double>(NR * NC), true);
    EXPECT_TRUE(tatami_test::check_block(mat, true, 0, 10));
    EXPECT_TRUE(tatami_test::check_block(mat, false, 20, 30));

    EXPECT_FALSE(tatami_test::check_block(mat, true, -1, 30));
    EXPECT_FALSE(tatami_test::check_block(mat, true, 0, -1));
    EXPECT_FALSE(tatami_test::check_block(mat, true, NR, 30));
    EXPECT_FALSE(tatami_test::check_block(mat, true, NR - 10, 30));

    tatami::DenseMatrix<double, int, std::vector<double> > empty(0, 0, std::vector<double>(), true);
    EXPECT_TRUE(tatami_test::check_block(empty, false, 0, 0));
}

static tatami::VectorPtr<int> create_indices(std::vector<int> payload) {
    return std::shared_ptr<std::vector<int> >(new std::vector<int>(std::move(payload)));
}

TEST(CheckIndices, Basic) {
    const int NR = 48, NC = 82;
    tatami::DenseMatrix<double, int, std::vector<double> > mat(NR, NC, std::vector<double>(NR * NC), true);
    EXPECT_TRUE(tatami_test::check_indices(mat, true, create_indices({ 0, 20, 40, 60, 80 })));
    EXPECT_TRUE(tatami_test::check_indices(mat, false, create_indices({ 0, 10, 20, 30, 40 })));
    EXPECT_TRUE(tatami_test::check_indices(mat, true, create_indices({})));

    EXPECT_FALSE(tatami_test::check_indices(mat, true, {}));
    EXPECT_FALSE(tatami_test::check_indices(mat, true, create_indices({ -1, 30 })));
    EXPECT_FALSE(tatami_test::check_indices(mat, true, create_indices({ 21, 10 })));
    EXPECT_FALSE(tatami_test::check_indices(mat, false, create_indices({ 21, 30, 60 })));
}
