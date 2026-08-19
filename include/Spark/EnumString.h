#pragma once
#include <stdint.h>
#include <Spark/Types.h>


inline const char* GetAccountClassString(AccountClassType value)
{
	switch (value)
	{
		case AccountClassType::Future:
			return "Future";
		case AccountClassType::Stock:
			return "Stock";
		case AccountClassType::Option:
			return "Option";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetAccountSettlementStatusString(AccountSettlementStatusType value)
{
	switch (value)
	{
		case AccountSettlementStatusType::NotStart:
			return "NotStart";
		case AccountSettlementStatusType::SettlementFailed:
			return "SettlementFailed";
		case AccountSettlementStatusType::SettlementCompleted:
			return "SettlementCompleted";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetAccountTypeString(AccountTypeType value)
{
	switch (value)
	{
		case AccountTypeType::Primary:
			return "Primary";
		case AccountTypeType::Sub:
			return "Sub";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetAccountStatusString(AccountStatusType value)
{
	switch (value)
	{
		case AccountStatusType::Normal:
			return "Normal";
		case AccountStatusType::Forbidden:
			return "Forbidden";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetAppPlatformString(AppPlatformType value)
{
	switch (value)
	{
		case AppPlatformType::Windows:
			return "Windows";
		case AppPlatformType::Linux:
			return "Linux";
		case AppPlatformType::IOS:
			return "IOS";
		case AppPlatformType::Android:
			return "Android";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetAuditStatusString(AuditStatusType value)
{
	switch (value)
	{
		case AuditStatusType::Approve:
			return "Approve";
		case AuditStatusType::Reject:
			return "Reject";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetBarPrecesString(BarPrecesType value)
{
	switch (value)
	{
		case BarPrecesType::Second:
			return "Second";
		case BarPrecesType::Minute:
			return "Minute";
		case BarPrecesType::Day:
			return "Day";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetComponentString(ComponentType value)
{
	switch (value)
	{
		case ComponentType::TradeFront:
			return "TradeFront";
		case ComponentType::RiskFront:
			return "RiskFront";
		case ComponentType::AdminFront:
			return "AdminFront";
		case ComponentType::OfferFront:
			return "OfferFront";
		case ComponentType::MarketData:
			return "MarketData";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetContingentConditionString(ContingentConditionType value)
{
	switch (value)
	{
		case ContingentConditionType::Immediately:
			return "Immediately";
		case ContingentConditionType::Touch:
			return "Touch";
		case ContingentConditionType::TouchProfit:
			return "TouchProfit";
		case ContingentConditionType::ParkedOrder:
			return "ParkedOrder";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetConnectStatusString(ConnectStatusType value)
{
	switch (value)
	{
		case ConnectStatusType::UnConnected:
			return "UnConnected";
		case ConnectStatusType::Connecting:
			return "Connecting";
		case ConnectStatusType::Accepted:
			return "Accepted";
		case ConnectStatusType::Rejected:
			return "Rejected";
		case ConnectStatusType::Connected:
			return "Connected";
		case ConnectStatusType::DisConnected:
			return "DisConnected";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetDBOperateString(DBOperateType value)
{
	switch (value)
	{
		case DBOperateType::CreateTables:
			return "CreateTables";
		case DBOperateType::DropTables:
			return "DropTables";
		case DBOperateType::TruncateTables:
			return "TruncateTables";
		case DBOperateType::Insert:
			return "Insert";
		case DBOperateType::Delete:
			return "Delete";
		case DBOperateType::DeleteByIndex:
			return "DeleteByIndex";
		case DBOperateType::Update:
			return "Update";
		case DBOperateType::BatchInsert:
			return "BatchInsert";
		case DBOperateType::Truncate:
			return "Truncate";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetDBTypeString(DBTypeType value)
{
	switch (value)
	{
		case DBTypeType::DuckDB:
			return "DuckDB";
		case DBTypeType::SqliteDB:
			return "SqliteDB";
		case DBTypeType::MysqlDB:
			return "MysqlDB";
		case DBTypeType::MariaDB:
			return "MariaDB";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetDeliveryMethodString(DeliveryMethodType value)
{
	switch (value)
	{
		case DeliveryMethodType::Cash:
			return "Cash";
		case DeliveryMethodType::Commodity:
			return "Commodity";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetDirectionString(DirectionType value)
{
	switch (value)
	{
		case DirectionType::Buy:
			return "Buy";
		case DirectionType::Sell:
			return "Sell";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetExchangeStatusString(ExchangeStatusType value)
{
	switch (value)
	{
		case ExchangeStatusType::BeforeTrading:
			return "BeforeTrading";
		case ExchangeStatusType::NoTrading:
			return "NoTrading";
		case ExchangeStatusType::Continous:
			return "Continous";
		case ExchangeStatusType::AuctionOrdering:
			return "AuctionOrdering";
		case ExchangeStatusType::AuctionBalance:
			return "AuctionBalance";
		case ExchangeStatusType::AuctionMatch:
			return "AuctionMatch";
		case ExchangeStatusType::Closed:
			return "Closed";
		case ExchangeStatusType::Initializing:
			return "Initializing";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetForceCloseReasonString(ForceCloseReasonType value)
{
	switch (value)
	{
		case ForceCloseReasonType::NotForceClose:
			return "NotForceClose";
		case ForceCloseReasonType::LackDeposit:
			return "LackDeposit";
		case ForceCloseReasonType::ClientOverPositionLimit:
			return "ClientOverPositionLimit";
		case ForceCloseReasonType::MemberOverPositionLimit:
			return "MemberOverPositionLimit";
		case ForceCloseReasonType::NotMultiple:
			return "NotMultiple";
		case ForceCloseReasonType::Violation:
			return "Violation";
		case ForceCloseReasonType::Other:
			return "Other";
		case ForceCloseReasonType::PersonDeliv:
			return "PersonDeliv";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetHedgeFlagString(HedgeFlagType value)
{
	switch (value)
	{
		case HedgeFlagType::Speculation:
			return "Speculation";
		case HedgeFlagType::Arbitrage:
			return "Arbitrage";
		case HedgeFlagType::Hedge:
			return "Hedge";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetInitStatusString(InitStatusType value)
{
	switch (value)
	{
		case InitStatusType::UnInitialized:
			return "UnInitialized";
		case InitStatusType::Initializing:
			return "Initializing";
		case InitStatusType::Initialized:
			return "Initialized";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetInstrumentClassString(InstrumentClassType value)
{
	switch (value)
	{
		case InstrumentClassType::Normal:
			return "Normal";
		case InstrumentClassType::Main:
			return "Main";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetIOModelString(IOModelType value)
{
	switch (value)
	{
		case IOModelType::Select:
			return "Select";
		case IOModelType::Epoll:
			return "Epoll";
		case IOModelType::Iocp:
			return "Iocp";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetIOTypeString(IOTypeType value)
{
	switch (value)
	{
		case IOTypeType::Tcp:
			return "Tcp";
		case IOTypeType::Udp:
			return "Udp";
		case IOTypeType::Shm:
			return "Shm";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetLogicFuncString(LogicFuncType value)
{
	switch (value)
	{
		case LogicFuncType::Equal:
			return "Equal";
		case LogicFuncType::NotEqual:
			return "NotEqual";
		case LogicFuncType::LessThan:
			return "LessThan";
		case LogicFuncType::LessEqual:
			return "LessEqual";
		case LogicFuncType::GreatThan:
			return "GreatThan";
		case LogicFuncType::GreatEqual:
			return "GreatEqual";
		case LogicFuncType::In:
			return "In";
		case LogicFuncType::EqualIn:
			return "EqualIn";
		case LogicFuncType::Out:
			return "Out";
		case LogicFuncType::EqualOut:
			return "EqualOut";
		case LogicFuncType::PctIn:
			return "PctIn";
		case LogicFuncType::PctEqualIn:
			return "PctEqualIn";
		case LogicFuncType::PctOut:
			return "PctOut";
		case LogicFuncType::PctEqualOut:
			return "PctEqualOut";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetLoginStatusString(LoginStatusType value)
{
	switch (value)
	{
		case LoginStatusType::NotLog:
			return "NotLog";
		case LoginStatusType::Logging:
			return "Logging";
		case LoginStatusType::Logged:
			return "Logged";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetMarginPriceTypeString(MarginPriceTypeType value)
{
	switch (value)
	{
		case MarginPriceTypeType::PreSettlementPrice:
			return "PreSettlementPrice";
		case MarginPriceTypeType::SettlementPrice:
			return "SettlementPrice";
		case MarginPriceTypeType::AveragePrice:
			return "AveragePrice";
		case MarginPriceTypeType::OpenPrice:
			return "OpenPrice";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetMarginTypeString(MarginTypeType value)
{
	switch (value)
	{
		case MarginTypeType::Rate:
			return "Rate";
		case MarginTypeType::Fixed:
			return "Fixed";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetMarketDataTypeString(MarketDataTypeType value)
{
	switch (value)
	{
		case MarketDataTypeType::Tick:
			return "Tick";
		case MarketDataTypeType::Bar:
			return "Bar";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetMatchModeString(MatchModeType value)
{
	switch (value)
	{
		case MatchModeType::OrderBook:
			return "OrderBook";
		case MatchModeType::LastPrice:
			return "LastPrice";
		case MatchModeType::OppositePrice:
			return "OppositePrice";
		case MatchModeType::Bar:
			return "Bar";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetPosiDirectionString(PosiDirectionType value)
{
	switch (value)
	{
		case PosiDirectionType::Long:
			return "Long";
		case PosiDirectionType::Short:
			return "Short";
		case PosiDirectionType::Net:
			return "Net";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetOfferTypeString(OfferTypeType value)
{
	switch (value)
	{
		case OfferTypeType::SE:
			return "SE";
		case OfferTypeType::Stds:
			return "Stds";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetOffsetFlagString(OffsetFlagType value)
{
	switch (value)
	{
		case OffsetFlagType::Open:
			return "Open";
		case OffsetFlagType::Close:
			return "Close";
		case OffsetFlagType::CloseToday:
			return "CloseToday";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetOptionIndicatorTypeString(OptionIndicatorTypeType value)
{
	switch (value)
	{
		case OptionIndicatorTypeType::Delta:
			return "Delta";
		case OptionIndicatorTypeType::Gamma:
			return "Gamma";
		case OptionIndicatorTypeType::Rho:
			return "Rho";
		case OptionIndicatorTypeType::Theta:
			return "Theta";
		case OptionIndicatorTypeType::Vega:
			return "Vega";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetOptionOrderPriceTypeString(OptionOrderPriceTypeType value)
{
	switch (value)
	{
		case OptionOrderPriceTypeType::LimitPrice:
			return "LimitPrice";
		case OptionOrderPriceTypeType::FAL:
			return "FAL";
		case OptionOrderPriceTypeType::FAK:
			return "FAK";
		case OptionOrderPriceTypeType::FOKXJ:
			return "FOKXJ";
		case OptionOrderPriceTypeType::FOK:
			return "FOK";
		case OptionOrderPriceTypeType::DBestPrice:
			return "DBestPrice";
		case OptionOrderPriceTypeType::WBestPrice:
			return "WBestPrice";
		case OptionOrderPriceTypeType::FAKSZ:
			return "FAKSZ";
		case OptionOrderPriceTypeType::FAK5SZ:
			return "FAK5SZ";
		case OptionOrderPriceTypeType::FOKSZ:
			return "FOKSZ";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetOptionTypeString(OptionTypeType value)
{
	switch (value)
	{
		case OptionTypeType::Call:
			return "Call";
		case OptionTypeType::Put:
			return "Put";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetOrderPriceTypeString(OrderPriceTypeType value)
{
	switch (value)
	{
		case OrderPriceTypeType::LimitPrice:
			return "LimitPrice";
		case OrderPriceTypeType::AnyPrice:
			return "AnyPrice";
		case OrderPriceTypeType::StockFAK5:
			return "StockFAK5";
		case OrderPriceTypeType::StockFAL5:
			return "StockFAL5";
		case OrderPriceTypeType::StockFAK:
			return "StockFAK";
		case OrderPriceTypeType::StockFOK:
			return "StockFOK";
		case OrderPriceTypeType::StockDBestPrice:
			return "StockDBestPrice";
		case OrderPriceTypeType::StockWBestPrice:
			return "StockWBestPrice";
		case OrderPriceTypeType::OptFAL:
			return "OptFAL";
		case OrderPriceTypeType::OptFAK:
			return "OptFAK";
		case OrderPriceTypeType::OptFOKXJ:
			return "OptFOKXJ";
		case OrderPriceTypeType::OptFOK:
			return "OptFOK";
		case OrderPriceTypeType::OptDBestPrice:
			return "OptDBestPrice";
		case OrderPriceTypeType::OptWBestPrice:
			return "OptWBestPrice";
		case OrderPriceTypeType::OptFAKSZ:
			return "OptFAKSZ";
		case OrderPriceTypeType::OptFAK5SZ:
			return "OptFAK5SZ";
		case OrderPriceTypeType::OptFOKSZ:
			return "OptFOKSZ";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetOrderStatusString(OrderStatusType value)
{
	switch (value)
	{
		case OrderStatusType::Inserting:
			return "Inserting";
		case OrderStatusType::Inserted:
			return "Inserted";
		case OrderStatusType::PartTraded:
			return "PartTraded";
		case OrderStatusType::AllTraded:
			return "AllTraded";
		case OrderStatusType::Canceled:
			return "Canceled";
		case OrderStatusType::PartTradedCanceled:
			return "PartTradedCanceled";
		case OrderStatusType::Auditing:
			return "Auditing";
		case OrderStatusType::AuditReject:
			return "AuditReject";
		case OrderStatusType::RiskChecking:
			return "RiskChecking";
		case OrderStatusType::RiskCheckFailed:
			return "RiskCheckFailed";
		case OrderStatusType::Error:
			return "Error";
		case OrderStatusType::NotTouched:
			return "NotTouched";
		case OrderStatusType::Touched:
			return "Touched";
		case OrderStatusType::Unknown:
			return "Unknown";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetOrderTypeString(OrderTypeType value)
{
	switch (value)
	{
		case OrderTypeType::Normal:
			return "Normal";
		case OrderTypeType::Swap:
			return "Swap";
		case OrderTypeType::Combination:
			return "Combination";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetParamTypeString(ParamTypeType value)
{
	switch (value)
	{
		case ParamTypeType::Const:
			return "Const";
		case ParamTypeType::Index:
			return "Index";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetProductClassString(ProductClassType value)
{
	switch (value)
	{
		case ProductClassType::Future:
			return "Future";
		case ProductClassType::FutureOption:
			return "FutureOption";
		case ProductClassType::Combination:
			return "Combination";
		case ProductClassType::Spot:
			return "Spot";
		case ProductClassType::EFP:
			return "EFP";
		case ProductClassType::Index:
			return "Index";
		case ProductClassType::Stock:
			return "Stock";
		case ProductClassType::StockOption:
			return "StockOption";
		case ProductClassType::ETF:
			return "ETF";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetProtocolTypeString(ProtocolTypeType value)
{
	switch (value)
	{
		case ProtocolTypeType::Xtp:
			return "Xtp";
		case ProtocolTypeType::Step:
			return "Step";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetQueryStatusString(QueryStatusType value)
{
	switch (value)
	{
		case QueryStatusType::UnQueried:
			return "UnQueried";
		case QueryStatusType::Quering:
			return "Quering";
		case QueryStatusType::Queried:
			return "Queried";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetRiskActionString(RiskActionType value)
{
	switch (value)
	{
		case RiskActionType::Warn:
			return "Warn";
		case RiskActionType::ForbiddenOpen:
			return "ForbiddenOpen";
		case RiskActionType::ForbiddenBuyOpen:
			return "ForbiddenBuyOpen";
		case RiskActionType::ForbiddenSellOpen:
			return "ForbiddenSellOpen";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetRiskClassIDString(RiskClassIDType value)
{
	switch (value)
	{
		case RiskClassIDType::TradeScope:
			return "TradeScope";
		case RiskClassIDType::VolumePrice:
			return "VolumePrice";
		case RiskClassIDType::Position:
			return "Position";
		case RiskClassIDType::NetValueLine:
			return "NetValueLine";
		case RiskClassIDType::Concentration:
			return "Concentration";
		case RiskClassIDType::Profit:
			return "Profit";
		case RiskClassIDType::Retracement:
			return "Retracement";
		case RiskClassIDType::RiskLevel:
			return "RiskLevel";
		case RiskClassIDType::OptionIndicator:
			return "OptionIndicator";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetRiskIndexTypeString(RiskIndexTypeType value)
{
	switch (value)
	{
		case RiskIndexTypeType::Int:
			return "Int";
		case RiskIndexTypeType::Double:
			return "Double";
		case RiskIndexTypeType::String:
			return "String";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetRiskLevelTypeString(RiskLevelTypeType value)
{
	switch (value)
	{
		case RiskLevelTypeType::Margin:
			return "Margin";
		case RiskLevelTypeType::MarketValue:
			return "MarketValue";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetRiskRuleStyleString(RiskRuleStyleType value)
{
	switch (value)
	{
		case RiskRuleStyleType::Afterward:
			return "Afterward";
		case RiskRuleStyleType::Beforehand:
			return "Beforehand";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetRiskStatusString(RiskStatusType value)
{
	switch (value)
	{
		case RiskStatusType::Normal:
			return "Normal";
		case RiskStatusType::Triggered:
			return "Triggered";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetSecurityDetailTypeString(SecurityDetailTypeType value)
{
	switch (value)
	{
		case SecurityDetailTypeType::Index:
			return "Index";
		case SecurityDetailTypeType::MainBoard:
			return "MainBoard";
		case SecurityDetailTypeType::Star:
			return "Star";
		case SecurityDetailTypeType::SMEBoard:
			return "SMEBoard";
		case SecurityDetailTypeType::SecondBoard:
			return "SecondBoard";
		case SecurityDetailTypeType::BShares:
			return "BShares";
		case SecurityDetailTypeType::ETF:
			return "ETF";
		case SecurityDetailTypeType::AllotmentCode:
			return "AllotmentCode";
		case SecurityDetailTypeType::Bond:
			return "Bond";
		case SecurityDetailTypeType::ConvertibleBond:
			return "ConvertibleBond";
		case SecurityDetailTypeType::ReverseRepo:
			return "ReverseRepo";
		case SecurityDetailTypeType::Option:
			return "Option";
		case SecurityDetailTypeType::Other:
			return "Other";
		case SecurityDetailTypeType::PreferredStock:
			return "PreferredStock";
		case SecurityDetailTypeType::Future:
			return "Future";
		case SecurityDetailTypeType::FutureOption:
			return "FutureOption";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetSectionClassString(SectionClassType value)
{
	switch (value)
	{
		case SectionClassType::Auction:
			return "Auction";
		case SectionClassType::Section:
			return "Section";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetServerTypeString(ServerTypeType value)
{
	switch (value)
	{
		case ServerTypeType::Client:
			return "Client";
		case ServerTypeType::Server:
			return "Server";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetSystemRunningStatusString(SystemRunningStatusType value)
{
	switch (value)
	{
		case SystemRunningStatusType::NotRun:
			return "NotRun";
		case SystemRunningStatusType::Initialized:
			return "Initialized";
		case SystemRunningStatusType::IsRunning:
			return "IsRunning";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetSystemSettlementStatusString(SystemSettlementStatusType value)
{
	switch (value)
	{
		case SystemSettlementStatusType::NotStart:
			return "NotStart";
		case SystemSettlementStatusType::SettleDataPrepared:
			return "SettleDataPrepared";
		case SystemSettlementStatusType::Settling:
			return "Settling";
		case SystemSettlementStatusType::SettlementFailed:
			return "SettlementFailed";
		case SystemSettlementStatusType::SettlementCompleted:
			return "SettlementCompleted";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetStockOrderPriceTypeString(StockOrderPriceTypeType value)
{
	switch (value)
	{
		case StockOrderPriceTypeType::LimitPrice:
			return "LimitPrice";
		case StockOrderPriceTypeType::FAK5:
			return "FAK5";
		case StockOrderPriceTypeType::FAL5:
			return "FAL5";
		case StockOrderPriceTypeType::FAK:
			return "FAK";
		case StockOrderPriceTypeType::FOK:
			return "FOK";
		case StockOrderPriceTypeType::DBestPrice:
			return "DBestPrice";
		case StockOrderPriceTypeType::WBestPrice:
			return "WBestPrice";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetTickTypeString(TickTypeType value)
{
	switch (value)
	{
		case TickTypeType::Add:
			return "Add";
		case TickTypeType::Delete:
			return "Delete";
		case TickTypeType::Status:
			return "Status";
		case TickTypeType::Trade:
			return "Trade";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetTimeConditionString(TimeConditionType value)
{
	switch (value)
	{
		case TimeConditionType::IOC:
			return "IOC";
		case TimeConditionType::GFS:
			return "GFS";
		case TimeConditionType::GFD:
			return "GFD";
		case TimeConditionType::GTD:
			return "GTD";
		case TimeConditionType::GTC:
			return "GTC";
		case TimeConditionType::GFA:
			return "GFA";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetTradeScopeTypeString(TradeScopeTypeType value)
{
	switch (value)
	{
		case TradeScopeTypeType::BlackList:
			return "BlackList";
		case TradeScopeTypeType::WhiteList:
			return "WhiteList";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetTradeStatusString(TradeStatusType value)
{
	switch (value)
	{
		case TradeStatusType::Ignore:
			return "Ignore";
		case TradeStatusType::Normal:
			return "Normal";
		case TradeStatusType::CloseOnly:
			return "CloseOnly";
		case TradeStatusType::BanTrading:
			return "BanTrading";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetTradeTypeString(TradeTypeType value)
{
	switch (value)
	{
		case TradeTypeType::Common:
			return "Common";
		case TradeTypeType::OptionsExecution:
			return "OptionsExecution";
		case TradeTypeType::OTC:
			return "OTC";
		case TradeTypeType::EFPDerived:
			return "EFPDerived";
		case TradeTypeType::CombinationDerived:
			return "CombinationDerived";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetTransferDirectionString(TransferDirectionType value)
{
	switch (value)
	{
		case TransferDirectionType::TransferIn:
			return "TransferIn";
		case TransferDirectionType::TransferOut:
			return "TransferOut";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetTransferFlagString(TransferFlagType value)
{
	switch (value)
	{
		case TransferFlagType::Normal:
			return "Normal";
		case TransferFlagType::Import:
			return "Import";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetTransferMethodString(TransferMethodType value)
{
	switch (value)
	{
		case TransferMethodType::Transfer:
			return "Transfer";
		case TransferMethodType::Cheque:
			return "Cheque";
		case TransferMethodType::Cash:
			return "Cash";
		case TransferMethodType::BFTransfer:
			return "BFTransfer";
		default:
			return "Unknown";
	}
	return "";
}

inline const char* GetVolumeConditionString(VolumeConditionType value)
{
	switch (value)
	{
		case VolumeConditionType::AV:
			return "AV";
		case VolumeConditionType::MV:
			return "MV";
		case VolumeConditionType::CV:
			return "CV";
		default:
			return "Unknown";
	}
	return "";
}


