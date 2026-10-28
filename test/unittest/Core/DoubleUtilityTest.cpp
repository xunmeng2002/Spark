#include <Spark/Core/Utility/DoubleUtility.h>
#include "doctest/doctest.h"

#include <limits>
#include <cmath>
using namespace Spark::Core;
// ============================================================
// DoubleUtility 测试
// 浮点数比较：epsilon = DBL_EPSILON ≈ 2.22e-16
// ============================================================

static constexpr double Epsilon = std::numeric_limits<double>::epsilon();

// ---------- DoubleEqual ----------

TEST_CASE("DoubleUtilityTest.DoubleEqual_ExactEqual")
{
    CHECK(DoubleUtility::DoubleEqual(1.0, 1.0));
    CHECK(DoubleUtility::DoubleEqual(-1.0, -1.0));
    CHECK(DoubleUtility::DoubleEqual(0.0, 0.0));
}

TEST_CASE("DoubleUtilityTest.DoubleEqual_WithinEpsilon")
{
    // 差值刚好在 epsilon 内
    CHECK(DoubleUtility::DoubleEqual(1.0, 1.0 + Epsilon * 0.5));
    CHECK(DoubleUtility::DoubleEqual(1.0 - Epsilon * 0.5, 1.0));
}

TEST_CASE("DoubleUtilityTest.DoubleEqual_BarelyOutside")
{
    // 差值刚好超过 epsilon
    CHECK_FALSE(DoubleUtility::DoubleEqual(1.0, 1.0 + Epsilon * 2.0));
}

TEST_CASE("DoubleUtilityTest.DoubleEqual_OppositeSign")
{
    CHECK_FALSE(DoubleUtility::DoubleEqual(1.0, -1.0));
}

// ---------- DoubleNotEqual ----------

TEST_CASE("DoubleUtilityTest.DoubleNotEqual")
{
    CHECK(DoubleUtility::DoubleNotEqual(1.0, 2.0));
    CHECK_FALSE(DoubleUtility::DoubleNotEqual(1.0, 1.0));
}

// ---------- DoubleLess ----------

TEST_CASE("DoubleUtilityTest.DoubleLess")
{
    CHECK(DoubleUtility::DoubleLess(1.0, 2.0));
    CHECK_FALSE(DoubleUtility::DoubleLess(2.0, 1.0));
}

TEST_CASE("DoubleUtilityTest.DoubleLess_NearEqual")
{
    // 差值在 epsilon 内时不算 less
    CHECK_FALSE(DoubleUtility::DoubleLess(1.0, 1.0 + Epsilon * 0.5));
}

// ---------- DoubleLessEqual ----------

TEST_CASE("DoubleUtilityTest.DoubleLessEqual")
{
    CHECK(DoubleUtility::DoubleLessEqual(1.0, 2.0));
    CHECK(DoubleUtility::DoubleLessEqual(1.0, 1.0));
    CHECK_FALSE(DoubleUtility::DoubleLessEqual(2.0, 1.0));
}

TEST_CASE("DoubleUtilityTest.DoubleLessEqual_NearEqual")
{
    // 差值在 epsilon 内也算 equal
    CHECK(DoubleUtility::DoubleLessEqual(1.0, 1.0 + Epsilon * 0.5));
}

// ---------- DoubleGreat ----------

TEST_CASE("DoubleUtilityTest.DoubleGreat")
{
    CHECK(DoubleUtility::DoubleGreat(2.0, 1.0));
    CHECK_FALSE(DoubleUtility::DoubleGreat(1.0, 2.0));
}

TEST_CASE("DoubleUtilityTest.DoubleGreat_NearEqual")
{
    CHECK_FALSE(DoubleUtility::DoubleGreat(1.0 + Epsilon * 0.5, 1.0));
}

// ---------- DoubleGreatEqual ----------

TEST_CASE("DoubleUtilityTest.DoubleGreatEqual")
{
    CHECK(DoubleUtility::DoubleGreatEqual(2.0, 1.0));
    CHECK(DoubleUtility::DoubleGreatEqual(1.0, 1.0));
    CHECK_FALSE(DoubleUtility::DoubleGreatEqual(1.0, 2.0));
}

TEST_CASE("DoubleUtilityTest.DoubleGreatEqual_NearEqual")
{
    CHECK(DoubleUtility::DoubleGreatEqual(1.0 + Epsilon * 0.5, 1.0));
}

// ---------- GetDoubleValue ----------

TEST_CASE("DoubleUtilityTest.GetDoubleValue_Normal")
{
    CHECK_EQ(DoubleUtility::GetDoubleValue(3.14), 3.14);
    CHECK_EQ(DoubleUtility::GetDoubleValue(-2.5), -2.5);
    CHECK_EQ(DoubleUtility::GetDoubleValue(0.0), 0.0);
}

TEST_CASE("DoubleUtilityTest.GetDoubleValue_NaN")
{
    CHECK_EQ(DoubleUtility::GetDoubleValue(std::nan("")), 0.0);
}

TEST_CASE("DoubleUtilityTest.GetDoubleValue_Infinity")
{
    CHECK_EQ(DoubleUtility::GetDoubleValue(std::numeric_limits<double>::infinity()), 0.0);
    CHECK_EQ(DoubleUtility::GetDoubleValue(-std::numeric_limits<double>::infinity()), 0.0);
}
