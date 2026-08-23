#pragma once
#include <Spark/Network/Protocol/PackageFactoryBase.h>


class PackageFactory : public spark::network::PackageFactoryBase
{
public:
	virtual spark::network::Package* CreatePackage(UShortType packageID) override;
};

