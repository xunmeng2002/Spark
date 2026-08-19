#pragma once
#include <Spark/Core/CoreExport.h>
#include <limits>

namespace spark::core
{
class CORE_EXPORTS DoubleUtility
{
public:
	static bool DoubleEqual(const double& left, const double& right);
	static bool DoubleNotEqual(const double& left, const double& right);
	static bool DoubleLess(const double& left, const double& right);
	static bool DoubleLessEqual(const double& left, const double& right);
	static bool DoubleGreat(const double& left, const double& right);
	static bool DoubleGreatEqual(const double& left, const double& right);

	static double GetDoubleValue(const double& value);
};
}

