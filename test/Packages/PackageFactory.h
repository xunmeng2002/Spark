// 本文件由 ../Templates/Cpp/Protocol/Packages/PackageFactory.h.tpl 生成；请勿手改，改动请改模板后重跑 pumpall.py
#pragma once
#include <Spark/Network/Protocol/PackageFactoryBase.h>

namespace Spark::Packages
{
class PackageFactory : public Spark::Network::PackageFactoryBase
{
public:
	virtual Spark::Network::Package* CreatePackage(UInt16Type packageID) override;
};
}

