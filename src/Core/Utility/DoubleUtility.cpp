#include <Spark/Core/Utility/DoubleUtility.h>
#include <cmath>

using namespace std;

constexpr double epsilon = std::numeric_limits<double>::epsilon();
bool DoubleUtility::DoubleEqual(const double& left, const double& right)
{
	return std::abs(left - right) <= epsilon;
}
bool DoubleUtility::DoubleNotEqual(const double& left, const double& right)
{
	return !DoubleEqual(left, right);
}
bool DoubleUtility::DoubleLess(const double& left, const double& right)
{
	return left < right - epsilon;
}
bool DoubleUtility::DoubleLessEqual(const double& left, const double& right)
{
	return left < right + epsilon;
}
bool DoubleUtility::DoubleGreat(const double& left, const double& right)
{
	return left > right + epsilon;
}
bool DoubleUtility::DoubleGreatEqual(const double& left, const double& right)
{
	return left > right - epsilon;
}

double DoubleUtility::GetDoubleValue(const double& value)
{
	if (isinf(value) || isnan(value))
	{
		return 0.0;
	}
	return value;
}
