#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Protocol/Package.h>

namespace spark::network
{
class PackageFactory
{
public:
	virtual Package* CreatePackage(UShortType packageID);
};
}
