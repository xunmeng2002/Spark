#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Protocol/Package.h>

namespace spark::network
{
class PackageFactoryBase
{
public:
	virtual Package* CreatePackage(UShortType packageID) = 0;
};
}
