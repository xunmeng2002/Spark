#include <gtest/gtest.h>
#include <Spark/Core/Utility/DoubleUtility.h>

#include <limits>
#include <cmath>
using namespace spark::core;
// ============================================================
// DoubleUtility 测试
// 浮点数比较：epsilon = DBL_EPSILON ≈ 2.22e-16
// ============================================================

static constexpr double kEps = std::numeric_limits<double>::epsilon();

// ---------- DoubleEqual ----------

TEST(DoubleUtilityTest, DoubleEqual_ExactEqual)
{
    EXPECT_TRUE(DoubleUtility::DoubleEqual(1.0, 1.0));
    EXPECT_TRUE(DoubleUtility::DoubleEqual(-1.0, -1.0));
    EXPECT_TRUE(DoubleUtility::DoubleEqual(0.0, 0.0));
}

TEST(DoubleUtilityTest, DoubleEqual_WithinEpsilon)
{
    // 差值刚好在 epsilon 内
    EXPECT_TRUE(DoubleUtility::DoubleEqual(1.0, 1.0 + kEps * 0.5));
    EXPECT_TRUE(DoubleUtility::DoubleEqual(1.0 - kEps * 0.5, 1.0));
}

TEST(DoubleUtilityTest, DoubleEqual_BarelyOutside)
{
    // 差值刚好超过 epsilon
    EXPECT_FALSE(DoubleUtility::DoubleEqual(1.0, 1.0 + kEps * 2.0));
}

TEST(DoubleUtilityTest, DoubleEqual_OppositeSign)
{
    EXPECT_FALSE(DoubleUtility::DoubleEqual(1.0, -1.0));
}

// ---------- DoubleNotEqual ----------

TEST(DoubleUtilityTest, DoubleNotEqual)
{
    EXPECT_TRUE(DoubleUtility::DoubleNotEqual(1.0, 2.0));
    EXPECT_FALSE(DoubleUtility::DoubleNotEqual(1.0, 1.0));
}

// ---------- DoubleLess ----------

TEST(DoubleUtilityTest, DoubleLess)
{
    EXPECT_TRUE(DoubleUtility::DoubleLess(1.0, 2.0));
    EXPECT_FALSE(DoubleUtility::DoubleLess(2.0, 1.0));
}

TEST(DoubleUtilityTest, DoubleLess_NearEqual)
{
    // 差值在 epsilon 内时不算 less
    EXPECT_FALSE(DoubleUtility::DoubleLess(1.0, 1.0 + kEps * 0.5));
}

// ---------- DoubleLessEqual ----------

TEST(DoubleUtilityTest, DoubleLessEqual)
{
    EXPECT_TRUE(DoubleUtility::DoubleLessEqual(1.0, 2.0));
    EXPECT_TRUE(DoubleUtility::DoubleLessEqual(1.0, 1.0));
    EXPECT_FALSE(DoubleUtility::DoubleLessEqual(2.0, 1.0));
}

TEST(DoubleUtilityTest, DoubleLessEqual_NearEqual)
{
    // 差值在 epsilon 内也算 equal
    EXPECT_TRUE(DoubleUtility::DoubleLessEqual(1.0, 1.0 + kEps * 0.5));
}

// ---------- DoubleGreat ----------

TEST(DoubleUtilityTest, DoubleGreat)
{
    EXPECT_TRUE(DoubleUtility::DoubleGreat(2.0, 1.0));
    EXPECT_FALSE(DoubleUtility::DoubleGreat(1.0, 2.0));
}

TEST(DoubleUtilityTest, DoubleGreat_NearEqual)
{
    EXPECT_FALSE(DoubleUtility::DoubleGreat(1.0 + kEps * 0.5, 1.0));
}

// ---------- DoubleGreatEqual ----------

TEST(DoubleUtilityTest, DoubleGreatEqual)
{
    EXPECT_TRUE(DoubleUtility::DoubleGreatEqual(2.0, 1.0));
    EXPECT_TRUE(DoubleUtility::DoubleGreatEqual(1.0, 1.0));
    EXPECT_FALSE(DoubleUtility::DoubleGreatEqual(1.0, 2.0));
}

TEST(DoubleUtilityTest, DoubleGreatEqual_NearEqual)
{
    EXPECT_TRUE(DoubleUtility::DoubleGreatEqual(1.0 + kEps * 0.5, 1.0));
}

// ---------- GetDoubleValue ----------

TEST(DoubleUtilityTest, GetDoubleValue_Normal)
{
    EXPECT_DOUBLE_EQ(DoubleUtility::GetDoubleValue(3.14), 3.14);
    EXPECT_DOUBLE_EQ(DoubleUtility::GetDoubleValue(-2.5), -2.5);
    EXPECT_DOUBLE_EQ(DoubleUtility::GetDoubleValue(0.0), 0.0);
}

TEST(DoubleUtilityTest, GetDoubleValue_NaN)
{
    EXPECT_DOUBLE_EQ(DoubleUtility::GetDoubleValue(std::nan("")), 0.0);
}

TEST(DoubleUtilityTest, GetDoubleValue_Infinity)
{
    EXPECT_DOUBLE_EQ(DoubleUtility::GetDoubleValue(std::numeric_limits<double>::infinity()), 0.0);
    EXPECT_DOUBLE_EQ(DoubleUtility::GetDoubleValue(-std::numeric_limits<double>::infinity()), 0.0);
}
