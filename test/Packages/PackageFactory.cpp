#include <Spark/Network/Protocol/PackageFactory.h>
#include "Packages.h"


Package* PackageFactory::CreatePackage(UShortType packageID)
{
	switch (packageID)
	{
	case NotifyComponentConnectStatusPackage::PackageID:
	{
		return NotifyComponentConnectStatusPackage::Allocate();
	}
	case ReqAccountLoginPackage::PackageID:
	{
		return ReqAccountLoginPackage::Allocate();
	}
	case RspAccountLoginPackage::PackageID:
	{
		return RspAccountLoginPackage::Allocate();
	}
	case ReqAccountLogoutPackage::PackageID:
	{
		return ReqAccountLogoutPackage::Allocate();
	}
	case RspAccountLogoutPackage::PackageID:
	{
		return RspAccountLogoutPackage::Allocate();
	}
	case ReqQryAccountPackage::PackageID:
	{
		return ReqQryAccountPackage::Allocate();
	}
	case RspQryAccountPackage::PackageID:
	{
		return RspQryAccountPackage::Allocate();
	}
	case ReqQryHolderAccountPackage::PackageID:
	{
		return ReqQryHolderAccountPackage::Allocate();
	}
	case RspQryHolderAccountPackage::PackageID:
	{
		return RspQryHolderAccountPackage::Allocate();
	}
	case ReqQryCapitalPackage::PackageID:
	{
		return ReqQryCapitalPackage::Allocate();
	}
	case RspQryCapitalPackage::PackageID:
	{
		return RspQryCapitalPackage::Allocate();
	}
	case ReqQryPositionPackage::PackageID:
	{
		return ReqQryPositionPackage::Allocate();
	}
	case RspQryPositionPackage::PackageID:
	{
		return RspQryPositionPackage::Allocate();
	}
	case ReqQryOrderPackage::PackageID:
	{
		return ReqQryOrderPackage::Allocate();
	}
	case RspQryOrderPackage::PackageID:
	{
		return RspQryOrderPackage::Allocate();
	}
	case ReqQryTradePackage::PackageID:
	{
		return ReqQryTradePackage::Allocate();
	}
	case RspQryTradePackage::PackageID:
	{
		return RspQryTradePackage::Allocate();
	}
	case ReqQryInstrumentPackage::PackageID:
	{
		return ReqQryInstrumentPackage::Allocate();
	}
	case RspQryInstrumentPackage::PackageID:
	{
		return RspQryInstrumentPackage::Allocate();
	}
	case ReqQryOptionInstrumentPackage::PackageID:
	{
		return ReqQryOptionInstrumentPackage::Allocate();
	}
	case RspQryOptionInstrumentPackage::PackageID:
	{
		return RspQryOptionInstrumentPackage::Allocate();
	}
	case ReqQryCommissionRatePackage::PackageID:
	{
		return ReqQryCommissionRatePackage::Allocate();
	}
	case RspQryCommissionRatePackage::PackageID:
	{
		return RspQryCommissionRatePackage::Allocate();
	}
	case ReqQryMoneyTransferPackage::PackageID:
	{
		return ReqQryMoneyTransferPackage::Allocate();
	}
	case RspQryMoneyTransferPackage::PackageID:
	{
		return RspQryMoneyTransferPackage::Allocate();
	}
	case ReqInsertOrderPackage::PackageID:
	{
		return ReqInsertOrderPackage::Allocate();
	}
	case RspInsertOrderPackage::PackageID:
	{
		return RspInsertOrderPackage::Allocate();
	}
	case ReqCancelOrderPackage::PackageID:
	{
		return ReqCancelOrderPackage::Allocate();
	}
	case RspCancelOrderPackage::PackageID:
	{
		return RspCancelOrderPackage::Allocate();
	}
	case RtnOrderPackage::PackageID:
	{
		return RtnOrderPackage::Allocate();
	}
	case RtnTradePackage::PackageID:
	{
		return RtnTradePackage::Allocate();
	}
	case RtnMoneyTransferPackage::PackageID:
	{
		return RtnMoneyTransferPackage::Allocate();
	}
	case RtnAccountLogoutPackage::PackageID:
	{
		return RtnAccountLogoutPackage::Allocate();
	}
	case ReqRiskUserLoginPackage::PackageID:
	{
		return ReqRiskUserLoginPackage::Allocate();
	}
	case RspRiskUserLoginPackage::PackageID:
	{
		return RspRiskUserLoginPackage::Allocate();
	}
	case ReqRiskUserLogoutPackage::PackageID:
	{
		return ReqRiskUserLogoutPackage::Allocate();
	}
	case RspRiskUserLogoutPackage::PackageID:
	{
		return RspRiskUserLogoutPackage::Allocate();
	}
	case RtnRiskUserLogoutPackage::PackageID:
	{
		return RtnRiskUserLogoutPackage::Allocate();
	}
	case RtnAccountPackage::PackageID:
	{
		return RtnAccountPackage::Allocate();
	}
	case RtnAccountDeletePackage::PackageID:
	{
		return RtnAccountDeletePackage::Allocate();
	}
	case RtnPositionPackage::PackageID:
	{
		return RtnPositionPackage::Allocate();
	}
	case RtnAccountRiskPackage::PackageID:
	{
		return RtnAccountRiskPackage::Allocate();
	}
	case RtnAccountRiskDeletePackage::PackageID:
	{
		return RtnAccountRiskDeletePackage::Allocate();
	}
	case RtnAccountRiskNotifyPackage::PackageID:
	{
		return RtnAccountRiskNotifyPackage::Allocate();
	}
	case ReqQryRiskGroupAccountPackage::PackageID:
	{
		return ReqQryRiskGroupAccountPackage::Allocate();
	}
	case RspQryRiskGroupAccountPackage::PackageID:
	{
		return RspQryRiskGroupAccountPackage::Allocate();
	}
	case ReqQryRiskGroupCapitalPackage::PackageID:
	{
		return ReqQryRiskGroupCapitalPackage::Allocate();
	}
	case RspQryRiskGroupCapitalPackage::PackageID:
	{
		return RspQryRiskGroupCapitalPackage::Allocate();
	}
	case ReqQryRiskGroupPositionPackage::PackageID:
	{
		return ReqQryRiskGroupPositionPackage::Allocate();
	}
	case RspQryRiskGroupPositionPackage::PackageID:
	{
		return RspQryRiskGroupPositionPackage::Allocate();
	}
	case ReqQryRiskGroupOrderPackage::PackageID:
	{
		return ReqQryRiskGroupOrderPackage::Allocate();
	}
	case RspQryRiskGroupOrderPackage::PackageID:
	{
		return RspQryRiskGroupOrderPackage::Allocate();
	}
	case ReqQryRiskGroupTradePackage::PackageID:
	{
		return ReqQryRiskGroupTradePackage::Allocate();
	}
	case RspQryRiskGroupTradePackage::PackageID:
	{
		return RspQryRiskGroupTradePackage::Allocate();
	}
	case ReqQryRiskGroupAccountRiskPackage::PackageID:
	{
		return ReqQryRiskGroupAccountRiskPackage::Allocate();
	}
	case RspQryRiskGroupAccountRiskPackage::PackageID:
	{
		return RspQryRiskGroupAccountRiskPackage::Allocate();
	}
	case ReqQryRiskGroupAccountRiskNotifyPackage::PackageID:
	{
		return ReqQryRiskGroupAccountRiskNotifyPackage::Allocate();
	}
	case RspQryRiskGroupAccountRiskNotifyPackage::PackageID:
	{
		return RspQryRiskGroupAccountRiskNotifyPackage::Allocate();
	}
	case ReqRiskInsertOrderPackage::PackageID:
	{
		return ReqRiskInsertOrderPackage::Allocate();
	}
	case RspRiskInsertOrderPackage::PackageID:
	{
		return RspRiskInsertOrderPackage::Allocate();
	}
	case ReqRiskCancelOrderPackage::PackageID:
	{
		return ReqRiskCancelOrderPackage::Allocate();
	}
	case RspRiskCancelOrderPackage::PackageID:
	{
		return RspRiskCancelOrderPackage::Allocate();
	}
	case ReqMdUserLoginPackage::PackageID:
	{
		return ReqMdUserLoginPackage::Allocate();
	}
	case RspMdUserLoginPackage::PackageID:
	{
		return RspMdUserLoginPackage::Allocate();
	}
	case ReqMdUserLogoutPackage::PackageID:
	{
		return ReqMdUserLogoutPackage::Allocate();
	}
	case RspMdUserLogoutPackage::PackageID:
	{
		return RspMdUserLogoutPackage::Allocate();
	}
	case ReqMdInitPackage::PackageID:
	{
		return ReqMdInitPackage::Allocate();
	}
	case RspMdInitPackage::PackageID:
	{
		return RspMdInitPackage::Allocate();
	}
	case ReqSubscribeMdPackage::PackageID:
	{
		return ReqSubscribeMdPackage::Allocate();
	}
	case RspSubscribeMdPackage::PackageID:
	{
		return RspSubscribeMdPackage::Allocate();
	}
	case RtnShortMdPackage::PackageID:
	{
		return RtnShortMdPackage::Allocate();
	}
	case RtnExchangeStatusPackage::PackageID:
	{
		return RtnExchangeStatusPackage::Allocate();
	}
	case RtnMdInitCompletedPackage::PackageID:
	{
		return RtnMdInitCompletedPackage::Allocate();
	}
	case ReqAdminUserLoginPackage::PackageID:
	{
		return ReqAdminUserLoginPackage::Allocate();
	}
	case RspAdminUserLoginPackage::PackageID:
	{
		return RspAdminUserLoginPackage::Allocate();
	}
	case ReqAdminUserLogoutPackage::PackageID:
	{
		return ReqAdminUserLogoutPackage::Allocate();
	}
	case RspAdminUserLogoutPackage::PackageID:
	{
		return RspAdminUserLogoutPackage::Allocate();
	}
	case RtnAdminUserLogoutPackage::PackageID:
	{
		return RtnAdminUserLogoutPackage::Allocate();
	}
	case ReqAddRiskUserPackage::PackageID:
	{
		return ReqAddRiskUserPackage::Allocate();
	}
	case RspAddRiskUserPackage::PackageID:
	{
		return RspAddRiskUserPackage::Allocate();
	}
	case ReqUpdateRiskUserPackage::PackageID:
	{
		return ReqUpdateRiskUserPackage::Allocate();
	}
	case RspUpdateRiskUserPackage::PackageID:
	{
		return RspUpdateRiskUserPackage::Allocate();
	}
	case ReqRemoveRiskUserPackage::PackageID:
	{
		return ReqRemoveRiskUserPackage::Allocate();
	}
	case RspRemoveRiskUserPackage::PackageID:
	{
		return RspRemoveRiskUserPackage::Allocate();
	}
	case ReqAddAdminUserPackage::PackageID:
	{
		return ReqAddAdminUserPackage::Allocate();
	}
	case RspAddAdminUserPackage::PackageID:
	{
		return RspAddAdminUserPackage::Allocate();
	}
	case ReqUpdateAdminUserPackage::PackageID:
	{
		return ReqUpdateAdminUserPackage::Allocate();
	}
	case RspUpdateAdminUserPackage::PackageID:
	{
		return RspUpdateAdminUserPackage::Allocate();
	}
	case ReqRemoveAdminUserPackage::PackageID:
	{
		return ReqRemoveAdminUserPackage::Allocate();
	}
	case RspRemoveAdminUserPackage::PackageID:
	{
		return RspRemoveAdminUserPackage::Allocate();
	}
	case ReqAddPrimaryAccountPackage::PackageID:
	{
		return ReqAddPrimaryAccountPackage::Allocate();
	}
	case RspAddPrimaryAccountPackage::PackageID:
	{
		return RspAddPrimaryAccountPackage::Allocate();
	}
	case ReqUpdatePrimaryAccountPackage::PackageID:
	{
		return ReqUpdatePrimaryAccountPackage::Allocate();
	}
	case RspUpdatePrimaryAccountPackage::PackageID:
	{
		return RspUpdatePrimaryAccountPackage::Allocate();
	}
	case ReqRemovePrimaryAccountPackage::PackageID:
	{
		return ReqRemovePrimaryAccountPackage::Allocate();
	}
	case RspRemovePrimaryAccountPackage::PackageID:
	{
		return RspRemovePrimaryAccountPackage::Allocate();
	}
	case ReqAddAccountPackage::PackageID:
	{
		return ReqAddAccountPackage::Allocate();
	}
	case RspAddAccountPackage::PackageID:
	{
		return RspAddAccountPackage::Allocate();
	}
	case ReqUpdateAccountPackage::PackageID:
	{
		return ReqUpdateAccountPackage::Allocate();
	}
	case RspUpdateAccountPackage::PackageID:
	{
		return RspUpdateAccountPackage::Allocate();
	}
	case ReqRemoveAccountPackage::PackageID:
	{
		return ReqRemoveAccountPackage::Allocate();
	}
	case RspRemoveAccountPackage::PackageID:
	{
		return RspRemoveAccountPackage::Allocate();
	}
	case ReqAddBaseCommissionPackage::PackageID:
	{
		return ReqAddBaseCommissionPackage::Allocate();
	}
	case RspAddBaseCommissionPackage::PackageID:
	{
		return RspAddBaseCommissionPackage::Allocate();
	}
	case ReqUpdateBaseCommissionPackage::PackageID:
	{
		return ReqUpdateBaseCommissionPackage::Allocate();
	}
	case RspUpdateBaseCommissionPackage::PackageID:
	{
		return RspUpdateBaseCommissionPackage::Allocate();
	}
	case ReqRemoveBaseCommissionPackage::PackageID:
	{
		return ReqRemoveBaseCommissionPackage::Allocate();
	}
	case RspRemoveBaseCommissionPackage::PackageID:
	{
		return RspRemoveBaseCommissionPackage::Allocate();
	}
	case ReqAddCommissionGroupPackage::PackageID:
	{
		return ReqAddCommissionGroupPackage::Allocate();
	}
	case RspAddCommissionGroupPackage::PackageID:
	{
		return RspAddCommissionGroupPackage::Allocate();
	}
	case ReqUpdateCommissionGroupPackage::PackageID:
	{
		return ReqUpdateCommissionGroupPackage::Allocate();
	}
	case RspUpdateCommissionGroupPackage::PackageID:
	{
		return RspUpdateCommissionGroupPackage::Allocate();
	}
	case ReqRemoveCommissionGroupPackage::PackageID:
	{
		return ReqRemoveCommissionGroupPackage::Allocate();
	}
	case RspRemoveCommissionGroupPackage::PackageID:
	{
		return RspRemoveCommissionGroupPackage::Allocate();
	}
	case ReqAddOptionMarginParamPackage::PackageID:
	{
		return ReqAddOptionMarginParamPackage::Allocate();
	}
	case RspAddOptionMarginParamPackage::PackageID:
	{
		return RspAddOptionMarginParamPackage::Allocate();
	}
	case ReqUpdateOptionMarginParamPackage::PackageID:
	{
		return ReqUpdateOptionMarginParamPackage::Allocate();
	}
	case RspUpdateOptionMarginParamPackage::PackageID:
	{
		return RspUpdateOptionMarginParamPackage::Allocate();
	}
	case ReqRemoveOptionMarginParamPackage::PackageID:
	{
		return ReqRemoveOptionMarginParamPackage::Allocate();
	}
	case RspRemoveOptionMarginParamPackage::PackageID:
	{
		return RspRemoveOptionMarginParamPackage::Allocate();
	}
	case ReqAddTradeOfferPackage::PackageID:
	{
		return ReqAddTradeOfferPackage::Allocate();
	}
	case RspAddTradeOfferPackage::PackageID:
	{
		return RspAddTradeOfferPackage::Allocate();
	}
	case ReqUpdateTradeOfferPackage::PackageID:
	{
		return ReqUpdateTradeOfferPackage::Allocate();
	}
	case RspUpdateTradeOfferPackage::PackageID:
	{
		return RspUpdateTradeOfferPackage::Allocate();
	}
	case ReqRemoveTradeOfferPackage::PackageID:
	{
		return ReqRemoveTradeOfferPackage::Allocate();
	}
	case RspRemoveTradeOfferPackage::PackageID:
	{
		return RspRemoveTradeOfferPackage::Allocate();
	}
	case ReqAddTradeGroupPackage::PackageID:
	{
		return ReqAddTradeGroupPackage::Allocate();
	}
	case RspAddTradeGroupPackage::PackageID:
	{
		return RspAddTradeGroupPackage::Allocate();
	}
	case ReqUpdateTradeGroupPackage::PackageID:
	{
		return ReqUpdateTradeGroupPackage::Allocate();
	}
	case RspUpdateTradeGroupPackage::PackageID:
	{
		return RspUpdateTradeGroupPackage::Allocate();
	}
	case ReqRemoveTradeGroupPackage::PackageID:
	{
		return ReqRemoveTradeGroupPackage::Allocate();
	}
	case RspRemoveTradeGroupPackage::PackageID:
	{
		return RspRemoveTradeGroupPackage::Allocate();
	}
	case ReqAddTradeGroupItemPackage::PackageID:
	{
		return ReqAddTradeGroupItemPackage::Allocate();
	}
	case RspAddTradeGroupItemPackage::PackageID:
	{
		return RspAddTradeGroupItemPackage::Allocate();
	}
	case ReqUpdateTradeGroupItemPackage::PackageID:
	{
		return ReqUpdateTradeGroupItemPackage::Allocate();
	}
	case RspUpdateTradeGroupItemPackage::PackageID:
	{
		return RspUpdateTradeGroupItemPackage::Allocate();
	}
	case ReqRemoveTradeGroupItemPackage::PackageID:
	{
		return ReqRemoveTradeGroupItemPackage::Allocate();
	}
	case RspRemoveTradeGroupItemPackage::PackageID:
	{
		return RspRemoveTradeGroupItemPackage::Allocate();
	}
	case ReqAddRiskGroupPackage::PackageID:
	{
		return ReqAddRiskGroupPackage::Allocate();
	}
	case RspAddRiskGroupPackage::PackageID:
	{
		return RspAddRiskGroupPackage::Allocate();
	}
	case ReqUpdateRiskGroupPackage::PackageID:
	{
		return ReqUpdateRiskGroupPackage::Allocate();
	}
	case RspUpdateRiskGroupPackage::PackageID:
	{
		return RspUpdateRiskGroupPackage::Allocate();
	}
	case ReqRemoveRiskGroupPackage::PackageID:
	{
		return ReqRemoveRiskGroupPackage::Allocate();
	}
	case RspRemoveRiskGroupPackage::PackageID:
	{
		return RspRemoveRiskGroupPackage::Allocate();
	}
	case ReqAddRiskGroupItemPackage::PackageID:
	{
		return ReqAddRiskGroupItemPackage::Allocate();
	}
	case RspAddRiskGroupItemPackage::PackageID:
	{
		return RspAddRiskGroupItemPackage::Allocate();
	}
	case ReqUpdateRiskGroupItemPackage::PackageID:
	{
		return ReqUpdateRiskGroupItemPackage::Allocate();
	}
	case RspUpdateRiskGroupItemPackage::PackageID:
	{
		return RspUpdateRiskGroupItemPackage::Allocate();
	}
	case ReqRemoveRiskGroupItemPackage::PackageID:
	{
		return ReqRemoveRiskGroupItemPackage::Allocate();
	}
	case RspRemoveRiskGroupItemPackage::PackageID:
	{
		return RspRemoveRiskGroupItemPackage::Allocate();
	}
	case ReqAddOrUpdateRiskPackage::PackageID:
	{
		return ReqAddOrUpdateRiskPackage::Allocate();
	}
	case RspAddOrUpdateRiskPackage::PackageID:
	{
		return RspAddOrUpdateRiskPackage::Allocate();
	}
	case ReqRemoveRiskPackage::PackageID:
	{
		return ReqRemoveRiskPackage::Allocate();
	}
	case RspRemoveRiskPackage::PackageID:
	{
		return RspRemoveRiskPackage::Allocate();
	}
	case ReqAddRiskRulePackage::PackageID:
	{
		return ReqAddRiskRulePackage::Allocate();
	}
	case RspAddRiskRulePackage::PackageID:
	{
		return RspAddRiskRulePackage::Allocate();
	}
	case ReqAddRiskRuleItemPackage::PackageID:
	{
		return ReqAddRiskRuleItemPackage::Allocate();
	}
	case RspAddRiskRuleItemPackage::PackageID:
	{
		return RspAddRiskRuleItemPackage::Allocate();
	}
	case ReqAddRiskTradeScopePackage::PackageID:
	{
		return ReqAddRiskTradeScopePackage::Allocate();
	}
	case RspAddRiskTradeScopePackage::PackageID:
	{
		return RspAddRiskTradeScopePackage::Allocate();
	}
	case ReqAddAccountRiskPackage::PackageID:
	{
		return ReqAddAccountRiskPackage::Allocate();
	}
	case RspAddAccountRiskPackage::PackageID:
	{
		return RspAddAccountRiskPackage::Allocate();
	}
	case ReqRemoveAccountRiskPackage::PackageID:
	{
		return ReqRemoveAccountRiskPackage::Allocate();
	}
	case RspRemoveAccountRiskPackage::PackageID:
	{
		return RspRemoveAccountRiskPackage::Allocate();
	}
	case ReqMoneyTransferPackage::PackageID:
	{
		return ReqMoneyTransferPackage::Allocate();
	}
	case RspMoneyTransferPackage::PackageID:
	{
		return RspMoneyTransferPackage::Allocate();
	}
	case ReqAuditOrderPackage::PackageID:
	{
		return ReqAuditOrderPackage::Allocate();
	}
	case RspAuditOrderPackage::PackageID:
	{
		return RspAuditOrderPackage::Allocate();
	}
	case ReqOfferLoginPackage::PackageID:
	{
		return ReqOfferLoginPackage::Allocate();
	}
	case RspOfferLoginPackage::PackageID:
	{
		return RspOfferLoginPackage::Allocate();
	}
	case ReqPrimaryAccountLoginPackage::PackageID:
	{
		return ReqPrimaryAccountLoginPackage::Allocate();
	}
	case RspPrimaryAccountLoginPackage::PackageID:
	{
		return RspPrimaryAccountLoginPackage::Allocate();
	}
	case ReqPrimaryAccountLogoutPackage::PackageID:
	{
		return ReqPrimaryAccountLogoutPackage::Allocate();
	}
	case RtnPrimaryAccountLogoutPackage::PackageID:
	{
		return RtnPrimaryAccountLogoutPackage::Allocate();
	}
	case ReqPrimaryAccountInitPackage::PackageID:
	{
		return ReqPrimaryAccountInitPackage::Allocate();
	}
	case RspPrimaryAccountInitPackage::PackageID:
	{
		return RspPrimaryAccountInitPackage::Allocate();
	}
	case ReqPrimaryAccountQueryPackage::PackageID:
	{
		return ReqPrimaryAccountQueryPackage::Allocate();
	}
	case RspPrimaryAccountQueryPackage::PackageID:
	{
		return RspPrimaryAccountQueryPackage::Allocate();
	}
	case ReqQryOfferOptionInstrumentPackage::PackageID:
	{
		return ReqQryOfferOptionInstrumentPackage::Allocate();
	}
	case RspQryOfferOptionInstrumentPackage::PackageID:
	{
		return RspQryOfferOptionInstrumentPackage::Allocate();
	}
	case RtnOfferOptionInstrumentPackage::PackageID:
	{
		return RtnOfferOptionInstrumentPackage::Allocate();
	}
	case ReqOfferOrderPackage::PackageID:
	{
		return ReqOfferOrderPackage::Allocate();
	}
	case ReqOfferCancelOrderPackage::PackageID:
	{
		return ReqOfferCancelOrderPackage::Allocate();
	}
	case RtnOfferOrderPackage::PackageID:
	{
		return RtnOfferOrderPackage::Allocate();
	}
	case RtnOfferTradePackage::PackageID:
	{
		return RtnOfferTradePackage::Allocate();
	}
	case RtnOfferErrorCancelOrderPackage::PackageID:
	{
		return RtnOfferErrorCancelOrderPackage::Allocate();
	}
	case RtnOfferCapitalPackage::PackageID:
	{
		return RtnOfferCapitalPackage::Allocate();
	}
	case RtnOfferPositionPackage::PackageID:
	{
		return RtnOfferPositionPackage::Allocate();
	}
	default:
		break;
	}
	return nullptr;
}

