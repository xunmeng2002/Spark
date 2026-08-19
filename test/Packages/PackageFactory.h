#pragma once
#include <Spark/Network/Protocol/PackageFactoryBase.h>

namespace spark::network
{
class PackageFactory : public PackageFactoryBase
{
public:
	virtual Package* CreatePackage(UShortType packageID) override;
};
}
