#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Network/Protocol/Package.h>

namespace Spark::Network
{
class PackageFactoryBase
{
public:
    PackageFactoryBase() = default;
    virtual ~PackageFactoryBase() = default;
	virtual Package* CreatePackage(UInt16Type packageID) = 0;
	virtual bool IsInboundPackageAccepted(UInt16Type packageID) { return true; }
};
}
