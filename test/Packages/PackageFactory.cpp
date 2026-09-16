// 本文件由 ../Templates/Cpp/Protocol/Packages/PackageFactory.cpp.tpl 生成；请勿手改，改动请改模板后重跑 pumpall.py
#include "PackageFactory.h"
#include "Packages.h"


using namespace Spark::Network;

namespace Spark::Packages
{
Package* PackageFactory::CreatePackage(UInt16Type packageId)
{
	switch (packageId)
	{
	case NotifyComponentConnectStatusPackage::PackageId:
	{
		return NotifyComponentConnectStatusPackage::Allocate();
	}
	case ReqAccountLoginPackage::PackageId:
	{
		return ReqAccountLoginPackage::Allocate();
	}
	case RspAccountLoginPackage::PackageId:
	{
		return RspAccountLoginPackage::Allocate();
	}
	case ReqAccountLogoutPackage::PackageId:
	{
		return ReqAccountLogoutPackage::Allocate();
	}
	case RspAccountLogoutPackage::PackageId:
	{
		return RspAccountLogoutPackage::Allocate();
	}
	case ReqQryAccountPackage::PackageId:
	{
		return ReqQryAccountPackage::Allocate();
	}
	case RspQryAccountPackage::PackageId:
	{
		return RspQryAccountPackage::Allocate();
	}
	case ReqQryHolderAccountPackage::PackageId:
	{
		return ReqQryHolderAccountPackage::Allocate();
	}
	case RspQryHolderAccountPackage::PackageId:
	{
		return RspQryHolderAccountPackage::Allocate();
	}
	case ReqQryCapitalPackage::PackageId:
	{
		return ReqQryCapitalPackage::Allocate();
	}
	case RspQryCapitalPackage::PackageId:
	{
		return RspQryCapitalPackage::Allocate();
	}
	case ReqQryPositionPackage::PackageId:
	{
		return ReqQryPositionPackage::Allocate();
	}
	case RspQryPositionPackage::PackageId:
	{
		return RspQryPositionPackage::Allocate();
	}
	case ReqQryOrderPackage::PackageId:
	{
		return ReqQryOrderPackage::Allocate();
	}
	case RspQryOrderPackage::PackageId:
	{
		return RspQryOrderPackage::Allocate();
	}
	case ReqQryTradePackage::PackageId:
	{
		return ReqQryTradePackage::Allocate();
	}
	case RspQryTradePackage::PackageId:
	{
		return RspQryTradePackage::Allocate();
	}
	case ReqQryInstrumentPackage::PackageId:
	{
		return ReqQryInstrumentPackage::Allocate();
	}
	case RspQryInstrumentPackage::PackageId:
	{
		return RspQryInstrumentPackage::Allocate();
	}
	case ReqQryOptionInstrumentPackage::PackageId:
	{
		return ReqQryOptionInstrumentPackage::Allocate();
	}
	case RspQryOptionInstrumentPackage::PackageId:
	{
		return RspQryOptionInstrumentPackage::Allocate();
	}
	case ReqQryCommissionRatePackage::PackageId:
	{
		return ReqQryCommissionRatePackage::Allocate();
	}
	case RspQryCommissionRatePackage::PackageId:
	{
		return RspQryCommissionRatePackage::Allocate();
	}
	case ReqQryMoneyTransferPackage::PackageId:
	{
		return ReqQryMoneyTransferPackage::Allocate();
	}
	case RspQryMoneyTransferPackage::PackageId:
	{
		return RspQryMoneyTransferPackage::Allocate();
	}
	case ReqInsertOrderPackage::PackageId:
	{
		return ReqInsertOrderPackage::Allocate();
	}
	case RspInsertOrderPackage::PackageId:
	{
		return RspInsertOrderPackage::Allocate();
	}
	case ReqCancelOrderPackage::PackageId:
	{
		return ReqCancelOrderPackage::Allocate();
	}
	case RspCancelOrderPackage::PackageId:
	{
		return RspCancelOrderPackage::Allocate();
	}
	case RtnOrderPackage::PackageId:
	{
		return RtnOrderPackage::Allocate();
	}
	case RtnTradePackage::PackageId:
	{
		return RtnTradePackage::Allocate();
	}
	case RtnMoneyTransferPackage::PackageId:
	{
		return RtnMoneyTransferPackage::Allocate();
	}
	case RtnAccountLogoutPackage::PackageId:
	{
		return RtnAccountLogoutPackage::Allocate();
	}
	case ReqRiskUserLoginPackage::PackageId:
	{
		return ReqRiskUserLoginPackage::Allocate();
	}
	case RspRiskUserLoginPackage::PackageId:
	{
		return RspRiskUserLoginPackage::Allocate();
	}
	case ReqRiskUserLogoutPackage::PackageId:
	{
		return ReqRiskUserLogoutPackage::Allocate();
	}
	case RspRiskUserLogoutPackage::PackageId:
	{
		return RspRiskUserLogoutPackage::Allocate();
	}
	case RtnRiskUserLogoutPackage::PackageId:
	{
		return RtnRiskUserLogoutPackage::Allocate();
	}
	case RtnAccountPackage::PackageId:
	{
		return RtnAccountPackage::Allocate();
	}
	case RtnAccountDeletePackage::PackageId:
	{
		return RtnAccountDeletePackage::Allocate();
	}
	case RtnPositionPackage::PackageId:
	{
		return RtnPositionPackage::Allocate();
	}
	case RtnAccountRiskPackage::PackageId:
	{
		return RtnAccountRiskPackage::Allocate();
	}
	case RtnAccountRiskDeletePackage::PackageId:
	{
		return RtnAccountRiskDeletePackage::Allocate();
	}
	case RtnAccountRiskNotifyPackage::PackageId:
	{
		return RtnAccountRiskNotifyPackage::Allocate();
	}
	case ReqQryRiskGroupAccountPackage::PackageId:
	{
		return ReqQryRiskGroupAccountPackage::Allocate();
	}
	case RspQryRiskGroupAccountPackage::PackageId:
	{
		return RspQryRiskGroupAccountPackage::Allocate();
	}
	case ReqQryRiskGroupCapitalPackage::PackageId:
	{
		return ReqQryRiskGroupCapitalPackage::Allocate();
	}
	case RspQryRiskGroupCapitalPackage::PackageId:
	{
		return RspQryRiskGroupCapitalPackage::Allocate();
	}
	case ReqQryRiskGroupPositionPackage::PackageId:
	{
		return ReqQryRiskGroupPositionPackage::Allocate();
	}
	case RspQryRiskGroupPositionPackage::PackageId:
	{
		return RspQryRiskGroupPositionPackage::Allocate();
	}
	case ReqQryRiskGroupOrderPackage::PackageId:
	{
		return ReqQryRiskGroupOrderPackage::Allocate();
	}
	case RspQryRiskGroupOrderPackage::PackageId:
	{
		return RspQryRiskGroupOrderPackage::Allocate();
	}
	case ReqQryRiskGroupTradePackage::PackageId:
	{
		return ReqQryRiskGroupTradePackage::Allocate();
	}
	case RspQryRiskGroupTradePackage::PackageId:
	{
		return RspQryRiskGroupTradePackage::Allocate();
	}
	case ReqQryRiskGroupAccountRiskPackage::PackageId:
	{
		return ReqQryRiskGroupAccountRiskPackage::Allocate();
	}
	case RspQryRiskGroupAccountRiskPackage::PackageId:
	{
		return RspQryRiskGroupAccountRiskPackage::Allocate();
	}
	case ReqQryRiskGroupAccountRiskNotifyPackage::PackageId:
	{
		return ReqQryRiskGroupAccountRiskNotifyPackage::Allocate();
	}
	case RspQryRiskGroupAccountRiskNotifyPackage::PackageId:
	{
		return RspQryRiskGroupAccountRiskNotifyPackage::Allocate();
	}
	case ReqRiskInsertOrderPackage::PackageId:
	{
		return ReqRiskInsertOrderPackage::Allocate();
	}
	case RspRiskInsertOrderPackage::PackageId:
	{
		return RspRiskInsertOrderPackage::Allocate();
	}
	case ReqRiskCancelOrderPackage::PackageId:
	{
		return ReqRiskCancelOrderPackage::Allocate();
	}
	case RspRiskCancelOrderPackage::PackageId:
	{
		return RspRiskCancelOrderPackage::Allocate();
	}
	case ReqMdUserLoginPackage::PackageId:
	{
		return ReqMdUserLoginPackage::Allocate();
	}
	case RspMdUserLoginPackage::PackageId:
	{
		return RspMdUserLoginPackage::Allocate();
	}
	case ReqMdUserLogoutPackage::PackageId:
	{
		return ReqMdUserLogoutPackage::Allocate();
	}
	case RspMdUserLogoutPackage::PackageId:
	{
		return RspMdUserLogoutPackage::Allocate();
	}
	case ReqMdInitPackage::PackageId:
	{
		return ReqMdInitPackage::Allocate();
	}
	case RspMdInitPackage::PackageId:
	{
		return RspMdInitPackage::Allocate();
	}
	case ReqSubscribeMdPackage::PackageId:
	{
		return ReqSubscribeMdPackage::Allocate();
	}
	case RspSubscribeMdPackage::PackageId:
	{
		return RspSubscribeMdPackage::Allocate();
	}
	case RtnShortMdPackage::PackageId:
	{
		return RtnShortMdPackage::Allocate();
	}
	case RtnExchangeStatusPackage::PackageId:
	{
		return RtnExchangeStatusPackage::Allocate();
	}
	case RtnMdInitCompletedPackage::PackageId:
	{
		return RtnMdInitCompletedPackage::Allocate();
	}
	case ReqAdminUserLoginPackage::PackageId:
	{
		return ReqAdminUserLoginPackage::Allocate();
	}
	case RspAdminUserLoginPackage::PackageId:
	{
		return RspAdminUserLoginPackage::Allocate();
	}
	case ReqAdminUserLogoutPackage::PackageId:
	{
		return ReqAdminUserLogoutPackage::Allocate();
	}
	case RspAdminUserLogoutPackage::PackageId:
	{
		return RspAdminUserLogoutPackage::Allocate();
	}
	case RtnAdminUserLogoutPackage::PackageId:
	{
		return RtnAdminUserLogoutPackage::Allocate();
	}
	case ReqAddRiskUserPackage::PackageId:
	{
		return ReqAddRiskUserPackage::Allocate();
	}
	case RspAddRiskUserPackage::PackageId:
	{
		return RspAddRiskUserPackage::Allocate();
	}
	case ReqUpdateRiskUserPackage::PackageId:
	{
		return ReqUpdateRiskUserPackage::Allocate();
	}
	case RspUpdateRiskUserPackage::PackageId:
	{
		return RspUpdateRiskUserPackage::Allocate();
	}
	case ReqRemoveRiskUserPackage::PackageId:
	{
		return ReqRemoveRiskUserPackage::Allocate();
	}
	case RspRemoveRiskUserPackage::PackageId:
	{
		return RspRemoveRiskUserPackage::Allocate();
	}
	case ReqAddAdminUserPackage::PackageId:
	{
		return ReqAddAdminUserPackage::Allocate();
	}
	case RspAddAdminUserPackage::PackageId:
	{
		return RspAddAdminUserPackage::Allocate();
	}
	case ReqUpdateAdminUserPackage::PackageId:
	{
		return ReqUpdateAdminUserPackage::Allocate();
	}
	case RspUpdateAdminUserPackage::PackageId:
	{
		return RspUpdateAdminUserPackage::Allocate();
	}
	case ReqRemoveAdminUserPackage::PackageId:
	{
		return ReqRemoveAdminUserPackage::Allocate();
	}
	case RspRemoveAdminUserPackage::PackageId:
	{
		return RspRemoveAdminUserPackage::Allocate();
	}
	case ReqAddPrimaryAccountPackage::PackageId:
	{
		return ReqAddPrimaryAccountPackage::Allocate();
	}
	case RspAddPrimaryAccountPackage::PackageId:
	{
		return RspAddPrimaryAccountPackage::Allocate();
	}
	case ReqUpdatePrimaryAccountPackage::PackageId:
	{
		return ReqUpdatePrimaryAccountPackage::Allocate();
	}
	case RspUpdatePrimaryAccountPackage::PackageId:
	{
		return RspUpdatePrimaryAccountPackage::Allocate();
	}
	case ReqRemovePrimaryAccountPackage::PackageId:
	{
		return ReqRemovePrimaryAccountPackage::Allocate();
	}
	case RspRemovePrimaryAccountPackage::PackageId:
	{
		return RspRemovePrimaryAccountPackage::Allocate();
	}
	case ReqAddAccountPackage::PackageId:
	{
		return ReqAddAccountPackage::Allocate();
	}
	case RspAddAccountPackage::PackageId:
	{
		return RspAddAccountPackage::Allocate();
	}
	case ReqUpdateAccountPackage::PackageId:
	{
		return ReqUpdateAccountPackage::Allocate();
	}
	case RspUpdateAccountPackage::PackageId:
	{
		return RspUpdateAccountPackage::Allocate();
	}
	case ReqRemoveAccountPackage::PackageId:
	{
		return ReqRemoveAccountPackage::Allocate();
	}
	case RspRemoveAccountPackage::PackageId:
	{
		return RspRemoveAccountPackage::Allocate();
	}
	case ReqAddBaseCommissionPackage::PackageId:
	{
		return ReqAddBaseCommissionPackage::Allocate();
	}
	case RspAddBaseCommissionPackage::PackageId:
	{
		return RspAddBaseCommissionPackage::Allocate();
	}
	case ReqUpdateBaseCommissionPackage::PackageId:
	{
		return ReqUpdateBaseCommissionPackage::Allocate();
	}
	case RspUpdateBaseCommissionPackage::PackageId:
	{
		return RspUpdateBaseCommissionPackage::Allocate();
	}
	case ReqRemoveBaseCommissionPackage::PackageId:
	{
		return ReqRemoveBaseCommissionPackage::Allocate();
	}
	case RspRemoveBaseCommissionPackage::PackageId:
	{
		return RspRemoveBaseCommissionPackage::Allocate();
	}
	case ReqAddCommissionGroupPackage::PackageId:
	{
		return ReqAddCommissionGroupPackage::Allocate();
	}
	case RspAddCommissionGroupPackage::PackageId:
	{
		return RspAddCommissionGroupPackage::Allocate();
	}
	case ReqUpdateCommissionGroupPackage::PackageId:
	{
		return ReqUpdateCommissionGroupPackage::Allocate();
	}
	case RspUpdateCommissionGroupPackage::PackageId:
	{
		return RspUpdateCommissionGroupPackage::Allocate();
	}
	case ReqRemoveCommissionGroupPackage::PackageId:
	{
		return ReqRemoveCommissionGroupPackage::Allocate();
	}
	case RspRemoveCommissionGroupPackage::PackageId:
	{
		return RspRemoveCommissionGroupPackage::Allocate();
	}
	case ReqAddOptionMarginParamPackage::PackageId:
	{
		return ReqAddOptionMarginParamPackage::Allocate();
	}
	case RspAddOptionMarginParamPackage::PackageId:
	{
		return RspAddOptionMarginParamPackage::Allocate();
	}
	case ReqUpdateOptionMarginParamPackage::PackageId:
	{
		return ReqUpdateOptionMarginParamPackage::Allocate();
	}
	case RspUpdateOptionMarginParamPackage::PackageId:
	{
		return RspUpdateOptionMarginParamPackage::Allocate();
	}
	case ReqRemoveOptionMarginParamPackage::PackageId:
	{
		return ReqRemoveOptionMarginParamPackage::Allocate();
	}
	case RspRemoveOptionMarginParamPackage::PackageId:
	{
		return RspRemoveOptionMarginParamPackage::Allocate();
	}
	case ReqAddTradeOfferPackage::PackageId:
	{
		return ReqAddTradeOfferPackage::Allocate();
	}
	case RspAddTradeOfferPackage::PackageId:
	{
		return RspAddTradeOfferPackage::Allocate();
	}
	case ReqUpdateTradeOfferPackage::PackageId:
	{
		return ReqUpdateTradeOfferPackage::Allocate();
	}
	case RspUpdateTradeOfferPackage::PackageId:
	{
		return RspUpdateTradeOfferPackage::Allocate();
	}
	case ReqRemoveTradeOfferPackage::PackageId:
	{
		return ReqRemoveTradeOfferPackage::Allocate();
	}
	case RspRemoveTradeOfferPackage::PackageId:
	{
		return RspRemoveTradeOfferPackage::Allocate();
	}
	case ReqAddTradeGroupPackage::PackageId:
	{
		return ReqAddTradeGroupPackage::Allocate();
	}
	case RspAddTradeGroupPackage::PackageId:
	{
		return RspAddTradeGroupPackage::Allocate();
	}
	case ReqUpdateTradeGroupPackage::PackageId:
	{
		return ReqUpdateTradeGroupPackage::Allocate();
	}
	case RspUpdateTradeGroupPackage::PackageId:
	{
		return RspUpdateTradeGroupPackage::Allocate();
	}
	case ReqRemoveTradeGroupPackage::PackageId:
	{
		return ReqRemoveTradeGroupPackage::Allocate();
	}
	case RspRemoveTradeGroupPackage::PackageId:
	{
		return RspRemoveTradeGroupPackage::Allocate();
	}
	case ReqAddTradeGroupItemPackage::PackageId:
	{
		return ReqAddTradeGroupItemPackage::Allocate();
	}
	case RspAddTradeGroupItemPackage::PackageId:
	{
		return RspAddTradeGroupItemPackage::Allocate();
	}
	case ReqUpdateTradeGroupItemPackage::PackageId:
	{
		return ReqUpdateTradeGroupItemPackage::Allocate();
	}
	case RspUpdateTradeGroupItemPackage::PackageId:
	{
		return RspUpdateTradeGroupItemPackage::Allocate();
	}
	case ReqRemoveTradeGroupItemPackage::PackageId:
	{
		return ReqRemoveTradeGroupItemPackage::Allocate();
	}
	case RspRemoveTradeGroupItemPackage::PackageId:
	{
		return RspRemoveTradeGroupItemPackage::Allocate();
	}
	case ReqAddRiskGroupPackage::PackageId:
	{
		return ReqAddRiskGroupPackage::Allocate();
	}
	case RspAddRiskGroupPackage::PackageId:
	{
		return RspAddRiskGroupPackage::Allocate();
	}
	case ReqUpdateRiskGroupPackage::PackageId:
	{
		return ReqUpdateRiskGroupPackage::Allocate();
	}
	case RspUpdateRiskGroupPackage::PackageId:
	{
		return RspUpdateRiskGroupPackage::Allocate();
	}
	case ReqRemoveRiskGroupPackage::PackageId:
	{
		return ReqRemoveRiskGroupPackage::Allocate();
	}
	case RspRemoveRiskGroupPackage::PackageId:
	{
		return RspRemoveRiskGroupPackage::Allocate();
	}
	case ReqAddRiskGroupItemPackage::PackageId:
	{
		return ReqAddRiskGroupItemPackage::Allocate();
	}
	case RspAddRiskGroupItemPackage::PackageId:
	{
		return RspAddRiskGroupItemPackage::Allocate();
	}
	case ReqUpdateRiskGroupItemPackage::PackageId:
	{
		return ReqUpdateRiskGroupItemPackage::Allocate();
	}
	case RspUpdateRiskGroupItemPackage::PackageId:
	{
		return RspUpdateRiskGroupItemPackage::Allocate();
	}
	case ReqRemoveRiskGroupItemPackage::PackageId:
	{
		return ReqRemoveRiskGroupItemPackage::Allocate();
	}
	case RspRemoveRiskGroupItemPackage::PackageId:
	{
		return RspRemoveRiskGroupItemPackage::Allocate();
	}
	case ReqAddOrUpdateRiskPackage::PackageId:
	{
		return ReqAddOrUpdateRiskPackage::Allocate();
	}
	case RspAddOrUpdateRiskPackage::PackageId:
	{
		return RspAddOrUpdateRiskPackage::Allocate();
	}
	case ReqRemoveRiskPackage::PackageId:
	{
		return ReqRemoveRiskPackage::Allocate();
	}
	case RspRemoveRiskPackage::PackageId:
	{
		return RspRemoveRiskPackage::Allocate();
	}
	case ReqAddRiskRulePackage::PackageId:
	{
		return ReqAddRiskRulePackage::Allocate();
	}
	case RspAddRiskRulePackage::PackageId:
	{
		return RspAddRiskRulePackage::Allocate();
	}
	case ReqAddRiskRuleItemPackage::PackageId:
	{
		return ReqAddRiskRuleItemPackage::Allocate();
	}
	case RspAddRiskRuleItemPackage::PackageId:
	{
		return RspAddRiskRuleItemPackage::Allocate();
	}
	case ReqAddRiskTradeScopePackage::PackageId:
	{
		return ReqAddRiskTradeScopePackage::Allocate();
	}
	case RspAddRiskTradeScopePackage::PackageId:
	{
		return RspAddRiskTradeScopePackage::Allocate();
	}
	case ReqAddAccountRiskPackage::PackageId:
	{
		return ReqAddAccountRiskPackage::Allocate();
	}
	case RspAddAccountRiskPackage::PackageId:
	{
		return RspAddAccountRiskPackage::Allocate();
	}
	case ReqRemoveAccountRiskPackage::PackageId:
	{
		return ReqRemoveAccountRiskPackage::Allocate();
	}
	case RspRemoveAccountRiskPackage::PackageId:
	{
		return RspRemoveAccountRiskPackage::Allocate();
	}
	case ReqMoneyTransferPackage::PackageId:
	{
		return ReqMoneyTransferPackage::Allocate();
	}
	case RspMoneyTransferPackage::PackageId:
	{
		return RspMoneyTransferPackage::Allocate();
	}
	case ReqAuditOrderPackage::PackageId:
	{
		return ReqAuditOrderPackage::Allocate();
	}
	case RspAuditOrderPackage::PackageId:
	{
		return RspAuditOrderPackage::Allocate();
	}
	case ReqOfferLoginPackage::PackageId:
	{
		return ReqOfferLoginPackage::Allocate();
	}
	case RspOfferLoginPackage::PackageId:
	{
		return RspOfferLoginPackage::Allocate();
	}
	case ReqPrimaryAccountLoginPackage::PackageId:
	{
		return ReqPrimaryAccountLoginPackage::Allocate();
	}
	case RspPrimaryAccountLoginPackage::PackageId:
	{
		return RspPrimaryAccountLoginPackage::Allocate();
	}
	case ReqPrimaryAccountLogoutPackage::PackageId:
	{
		return ReqPrimaryAccountLogoutPackage::Allocate();
	}
	case RtnPrimaryAccountLogoutPackage::PackageId:
	{
		return RtnPrimaryAccountLogoutPackage::Allocate();
	}
	case ReqPrimaryAccountInitPackage::PackageId:
	{
		return ReqPrimaryAccountInitPackage::Allocate();
	}
	case RspPrimaryAccountInitPackage::PackageId:
	{
		return RspPrimaryAccountInitPackage::Allocate();
	}
	case ReqPrimaryAccountQueryPackage::PackageId:
	{
		return ReqPrimaryAccountQueryPackage::Allocate();
	}
	case RspPrimaryAccountQueryPackage::PackageId:
	{
		return RspPrimaryAccountQueryPackage::Allocate();
	}
	case ReqQryOfferOptionInstrumentPackage::PackageId:
	{
		return ReqQryOfferOptionInstrumentPackage::Allocate();
	}
	case RspQryOfferOptionInstrumentPackage::PackageId:
	{
		return RspQryOfferOptionInstrumentPackage::Allocate();
	}
	case RtnOfferOptionInstrumentPackage::PackageId:
	{
		return RtnOfferOptionInstrumentPackage::Allocate();
	}
	case ReqOfferOrderPackage::PackageId:
	{
		return ReqOfferOrderPackage::Allocate();
	}
	case ReqOfferCancelOrderPackage::PackageId:
	{
		return ReqOfferCancelOrderPackage::Allocate();
	}
	case RtnOfferOrderPackage::PackageId:
	{
		return RtnOfferOrderPackage::Allocate();
	}
	case RtnOfferTradePackage::PackageId:
	{
		return RtnOfferTradePackage::Allocate();
	}
	case RtnOfferErrorCancelOrderPackage::PackageId:
	{
		return RtnOfferErrorCancelOrderPackage::Allocate();
	}
	case RtnOfferCapitalPackage::PackageId:
	{
		return RtnOfferCapitalPackage::Allocate();
	}
	case RtnOfferPositionPackage::PackageId:
	{
		return RtnOfferPositionPackage::Allocate();
	}
	default:
		break;
	}
	return nullptr;
}
}
