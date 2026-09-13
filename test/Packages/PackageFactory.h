#pragma once
#include <Spark/Network/Protocol/PackageFactoryBase.h>

namespace spark::packages
{
class PackageFactory : public spark::network::PackageFactoryBase
{
public:
	virtual spark::network::Package* CreatePackage(UInt16Type packageID) override;
};
}

