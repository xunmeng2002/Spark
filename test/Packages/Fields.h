#pragma once
#include <Spark/Types.h>

class RspInfoField
{
public:
	static constexpr UShortType FieldID = 0x0003;
	ErrorIDType ErrorID;		//错误代码
	MessageType ErrorMsg;		//错误信息
};
class NotifyComponentConnectStatusField
{
public:
	static constexpr UShortType FieldID = 0x0004;
	SessionIDType SessionID;		//会话编号
	ComponentType Component;		//组件类型
	BoolType IsConnected;		//是否链接
};
class ReqAccountLoginField
{
public:
	static constexpr UShortType FieldID = 0x1001;
	AccountIDType AccountID;		//账户代码
	PasswordType Password;		//密码
};
class RspAccountLoginField
{
public:
	static constexpr UShortType FieldID = 0x1002;
	AccountIDType AccountID;		//账户代码
	DateType LoginDate;		//登录日期
	TimeType LoginTime;		//登录时间
	SessionIDType SessionID;		//会话编号
};
class ReqAccountLogoutField
{
public:
	static constexpr UShortType FieldID = 0x1003;
	AccountIDType AccountID;		//账户代码
};
class RspAccountLogoutField
{
public:
	static constexpr UShortType FieldID = 0x1004;
	AccountIDType AccountID;		//账户代码
};
class AccountLogoutField
{
public:
	static constexpr UShortType FieldID = 0x1005;
	AccountIDType AccountID;		//账户代码
	ErrorIDType ErrorID;		//错误代码
	MessageType ErrorMsg;		//错误信息
};
class ReqQryAccountField
{
public:
	static constexpr UShortType FieldID = 0x1006;
	AccountIDType AccountID;		//账户代码
};
class AccountField
{
public:
	static constexpr UShortType FieldID = 0x1007;
	AccountIDType AccountID;		//账户代码
	AccountTypeType AccountType;		//账户类型
	AccountStatusType AccountStatus;		//账户状态
	GroupIDType TradeGroupID;		//交易组代码
	GroupIDType RiskGroupID;		//交易组代码
	GroupIDType CommissionGroupID;		//交易组代码
};
class ReqQryHolderAccountField
{
public:
	static constexpr UShortType FieldID = 0x1008;
	AccountIDType AccountID;		//账户代码
};
class HolderAccountField
{
public:
	static constexpr UShortType FieldID = 0x1009;
	ExchangeIDType ExchangeID;		//交易所代码
	AccountIDType HolderAccountID;		//股东账户代码
	BoolType PrimaryFlag;		//主账号标志
};
class ReqQryCapitalField
{
public:
	static constexpr UShortType FieldID = 0x100A;
	AccountIDType AccountID;		//账户代码
};
class CapitalField
{
public:
	static constexpr UShortType FieldID = 0x100B;
	DateType TradingDay;		//交易日
	AccountIDType AccountID;		//账户代码
	AccountTypeType AccountType;		//账户类型
	MoneyType Asset;		//总资产
	MoneyType PreAsset;		//上日总资产
	MoneyType CashAsset;		//现金资产
	MoneyType PreCashAsset;		//上日现金资产
	MoneyType Available;		//可用资金
	MoneyType CashIn;		//现金收入
	MoneyType CashOut;		//现金支出
	MoneyType Margin;		//保证金
	MoneyType Commission;		//手续费
	MoneyType StampTax;		//印花税
	MoneyType TransferFee;		//过户费
	MoneyType FrozenCash;		//冻结资金
	MoneyType FrozenMargin;		//冻结保证金
	MoneyType FrozenCommission;		//冻结手续费
	MoneyType FrozenStampTax;		//冻结印花税
	MoneyType FrozenTransferFee;		//冻结过户费
	MoneyType MarketValue;		//市值
	MoneyType TotalProfit;		//总盈亏
	MoneyType TodayProfit;		//当日盈亏
	MoneyType Deposit;		//入金
	MoneyType Withdraw;		//出金
};
class ReqQryPositionField
{
public:
	static constexpr UShortType FieldID = 0x100C;
	AccountIDType AccountID;		//账户代码
};
class PositionField
{
public:
	static constexpr UShortType FieldID = 0x100D;
	DateType TradingDay;		//交易日
	AccountIDType AccountID;		//账户代码
	AccountTypeType AccountType;		//账户类型
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	ProductClassType ProductClass;		//品种类型
	PosiDirectionType PosiDirection;		//持仓方向
	VolumeType TotalPosition;		//持仓数量
	VolumeType PositionFrozen;		//冻结持仓
	VolumeType TodayPosition;		//今日持仓
	PriceType TotalCostPrice;		//总成本价
	PriceType TodayCostPrice;		//当日成本价
	MoneyType CashIn;		//现金收入
	MoneyType CashOut;		//现金支出
	MoneyType Margin;		//保证金
	MoneyType Commission;		//手续费
	MoneyType StampTax;		//印花税
	MoneyType TransferFee;		//过户费
	MoneyType MarketValue;		//市值
	VolumeMultipleType VolumeMultiple;		//合约乘数
	MoneyType TotalCost;		//总成本
	MoneyType TodayCost;		//当日成本
	MoneyType TotalProfit;		//总盈亏
	MoneyType TodayProfit;		//当日盈亏
	PriceType LastPrice;		//最新价
	PriceType PrePrice;		//昨收盘价或昨结算价
};
class ReqQryOrderField
{
public:
	static constexpr UShortType FieldID = 0x100E;
	AccountIDType AccountID;		//账户代码
};
class OrderField
{
public:
	static constexpr UShortType FieldID = 0x100F;
	DateType TradingDay;		//交易日
	AccountIDType AccountID;		//账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	ProductClassType ProductClass;		//品种类型
	OrderIDType OrderID;		//委托编号
	OrderSysIDType OrderSysID;		//系统委托编号
	DirectionType Direction;		//买卖方向
	OffsetFlagType OffsetFlag;		//开平标志
	OrderPriceTypeType OrderPriceType;		//委托价格类型
	PriceType Price;		//委托价格
	VolumeType Volume;		//委托数量
	VolumeType VolumeTotal;		//剩余数量
	VolumeType VolumeTraded;		//成交数量
	VolumeMultipleType VolumeMultiple;		//合约乘数
	OrderStatusType OrderStatus;		//委托状态
	MessageType StatusMsg;		//状态信息
	DateType OrderDate;		//委托日期
	TimeType OrderTime;		//委托时间
	DateType CancelDate;		//撤单日期
	TimeType CancelTime;		//撤单时间
	SessionIDType SessionID;		//会话编号
	ClientOrderIDType ClientOrderID;		//客户端委托编号
	RequestIDType RequestID;		//客户端请求编号
	MoneyType FrozenCash;		//冻结资金
	MoneyType FrozenMargin;		//冻结保证金
	MoneyType FrozenCommission;		//冻结手续费
	MoneyType FrozenStampTax;		//冻结印花税
	MoneyType FrozenTransferFee;		//冻结过户费
};
class ReqQryTradeField
{
public:
	static constexpr UShortType FieldID = 0x1010;
	AccountIDType AccountID;		//账户代码
};
class TradeField
{
public:
	static constexpr UShortType FieldID = 0x1011;
	DateType TradingDay;		//交易日
	AccountIDType AccountID;		//账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	ProductClassType ProductClass;		//品种类型
	OrderIDType OrderID;		//委托编号
	OrderSysIDType OrderSysID;		//系统委托编号
	TradeIDType TradeID;		//成交编号
	DirectionType Direction;		//买卖方向
	OffsetFlagType OffsetFlag;		//开平标志
	PriceType Price;		//委托价格
	VolumeType Volume;		//委托数量
	VolumeMultipleType VolumeMultiple;		//合约乘数
	MoneyType TradeAmount;		//成交金额
	MoneyType Commission;		//手续费
	MoneyType StampTax;		//印花税
	MoneyType TransferFee;		//过户费
	DateType TradeDate;		//成交日期
	TimeType TradeTime;		//成交时间
};
class ReqQryInstrumentField
{
public:
	static constexpr UShortType FieldID = 0x1012;
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
};
class InstrumentField
{
public:
	static constexpr UShortType FieldID = 0x1013;
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	InstrumentIDType ExchangeInstID;		//交易所合约代码
	InstrumentNameType InstrumentName;		//合约名称
	VolumeMultipleType VolumeMultiple;		//合约乘数
	ProductClassType ProductClass;		//品种类型
};
class ReqQryOptionInstrumentField
{
public:
	static constexpr UShortType FieldID = 0x1014;
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
};
class OptionInstrumentField
{
public:
	static constexpr UShortType FieldID = 0x1015;
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	InstrumentIDType ExchangeInstID;		//交易所合约代码
	InstrumentNameType InstrumentName;		//合约名称
	VolumeMultipleType VolumeMultiple;		//合约乘数
	OptionTypeType OptionType;		//期权类型
	InstrumentIDType UnderlyingInstrumentID;		//标的合约代码
	PriceType ExecutePrice;		//行权价
	MoneyType UnitMargin;		//单位保证金
	PriceType PriceTick;		//最小变动价位
	VolumeType MaxLimitOrderVolume;		//限价最大下单量
	VolumeType MaxMarketOrderVolume;		//市价最大下单量
	DateType ExpiringDate;		//到期日
};
class ReqQryCommissionRateField
{
public:
	static constexpr UShortType FieldID = 0x1016;
	AccountIDType AccountID;		//账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class CommissionRateField
{
public:
	static constexpr UShortType FieldID = 0x1017;
	AccountIDType AccountID;		//账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
	RateType OpenBuyByMoney;		//买开仓费率
	RateType OpenSellByMoney;		//卖开仓费率
	RateType CloseBuyByMoney;		//买平仓费率
	RateType CloseSellByMoney;		//卖平仓费率
	RateType OpenBuyByVolume;		//每手买开仓费用
	RateType OpenSellByVolume;		//每手卖开仓费用
	RateType CloseBuyByVolume;		//每手买平仓费用
	RateType CloseSellByVolume;		//每手卖平仓费用
	MoneyType MinCommission;		//最低手续费
	MoneyType MaxCommission;		//最高手续费
};
class ReqQryMoneyTransferField
{
public:
	static constexpr UShortType FieldID = 0x1018;
	AccountIDType AccountID;		//账户代码
};
class MoneyTransferField
{
public:
	static constexpr UShortType FieldID = 0x1019;
	DateType TradingDay;		//交易日
	AccountIDType AccountID;		//账户代码
	SequenceNoType MoneyTransferID;		//出入金编号
	AccountTypeType AccountType;		//账户类型
	TransferDirectionType TransferDirection;		//转移方向
	MoneyType TransferAmount;		//转移金额
	MessageType InfoMessage;		//备注信息
	UserIDType UserID;		//用户代码
	DateType TransferDate;		//操作日期
	TimeType TransferTime;		//操作时间
};
class ReqInsertOrderField
{
public:
	static constexpr UShortType FieldID = 0x101A;
	AccountIDType AccountID;		//账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	DirectionType Direction;		//买卖方向
	OffsetFlagType OffsetFlag;		//开平标志
	OrderPriceTypeType OrderPriceType;		//委托价格类型
	PriceType Price;		//委托价格
	VolumeType Volume;		//委托数量
	ClientOrderIDType ClientOrderID;		//客户端委托编号
};
class ReqCancelOrderField
{
public:
	static constexpr UShortType FieldID = 0x101B;
	AccountIDType AccountID;		//账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	ClientOrderIDType ClientCancelOrderID;		//客户端撤单委托编号
	OrderIDType OrderID;		//委托编号
	OrderSysIDType OrderSysID;		//系统委托编号
	ClientOrderIDType ClientOrderID;		//客户端委托编号
};
class CancelOrderField
{
public:
	static constexpr UShortType FieldID = 0x101C;
	AccountIDType AccountID;		//账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	ClientOrderIDType ClientCancelOrderID;		//客户端撤单委托编号
	OrderIDType OrderID;		//委托编号
	OrderSysIDType OrderSysID;		//系统委托编号
	ClientOrderIDType ClientOrderID;		//客户端委托编号
};
class ReqRiskUserLoginField
{
public:
	static constexpr UShortType FieldID = 0x2001;
	UserIDType UserID;		//用户代码
	PasswordType Password;		//密码
};
class RspRiskUserLoginField
{
public:
	static constexpr UShortType FieldID = 0x2002;
	UserIDType UserID;		//用户代码
	GroupIDType RiskGroupID;		//交易组代码
	DateType LoginDate;		//登录日期
	TimeType LoginTime;		//登录时间
	SessionIDType SessionID;		//会话编号
};
class ReqRiskUserLogoutField
{
public:
	static constexpr UShortType FieldID = 0x2003;
	UserIDType UserID;		//用户代码
};
class RspRiskUserLogoutField
{
public:
	static constexpr UShortType FieldID = 0x2004;
	UserIDType UserID;		//用户代码
};
class RiskUserLogoutField
{
public:
	static constexpr UShortType FieldID = 0x2005;
	UserIDType UserID;		//用户代码
	ErrorIDType ErrorID;		//错误代码
	MessageType ErrorMsg;		//错误信息
};
class AccountDeleteField
{
public:
	static constexpr UShortType FieldID = 0x2006;
	AccountIDType AccountID;		//账户代码
};
class AccountRiskField
{
public:
	static constexpr UShortType FieldID = 0x2007;
	AccountIDType AccountID;		//账户代码
	RiskIDType RiskID;		//风控代码
	RiskStatusType RiskStatus;		//风控状态
};
class AccountRiskDeleteField
{
public:
	static constexpr UShortType FieldID = 0x2008;
	AccountIDType AccountID;		//账户代码
	RiskIDType RiskID;		//风控代码
};
class AccountRiskNotifyField
{
public:
	static constexpr UShortType FieldID = 0x2009;
	DateType NotifyDate;		//通知日期
	TimeType NotifyTime;		//通知时间
	AccountIDType AccountID;		//账户代码
	RiskIDType RiskID;		//风控代码
	RiskRuleIDType RiskRuleID;		//风控规则代码
	RiskStatusType RiskStatus;		//风控状态
	MessageType RiskMessage;		//风控信息
};
class ReqQryRiskGroupAccountField
{
public:
	static constexpr UShortType FieldID = 0x200A;
	UserIDType UserID;		//用户代码
};
class ReqQryRiskGroupCapitalField
{
public:
	static constexpr UShortType FieldID = 0x200B;
	UserIDType UserID;		//用户代码
	AccountIDType AccountID;		//账户代码
};
class ReqQryRiskGroupPositionField
{
public:
	static constexpr UShortType FieldID = 0x200C;
	UserIDType UserID;		//用户代码
	AccountIDType AccountID;		//账户代码
};
class ReqQryRiskGroupOrderField
{
public:
	static constexpr UShortType FieldID = 0x200D;
	UserIDType UserID;		//用户代码
	AccountIDType AccountID;		//账户代码
};
class ReqQryRiskGroupTradeField
{
public:
	static constexpr UShortType FieldID = 0x200E;
	UserIDType UserID;		//用户代码
	AccountIDType AccountID;		//账户代码
};
class ReqQryRiskGroupAccountRiskField
{
public:
	static constexpr UShortType FieldID = 0x200F;
	UserIDType UserID;		//用户代码
	AccountIDType AccountID;		//账户代码
};
class ReqQryRiskGroupAccountRiskNotifyField
{
public:
	static constexpr UShortType FieldID = 0x2010;
	UserIDType UserID;		//用户代码
	AccountIDType AccountID;		//账户代码
};
class ReqRiskInsertOrderField
{
public:
	static constexpr UShortType FieldID = 0x2011;
	UserIDType UserID;		//用户代码
	AccountIDType AccountID;		//账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	DirectionType Direction;		//买卖方向
	OffsetFlagType OffsetFlag;		//开平标志
	OrderPriceTypeType OrderPriceType;		//委托价格类型
	PriceType Price;		//委托价格
	VolumeType Volume;		//委托数量
	ClientOrderIDType ClientOrderID;		//客户端委托编号
	BoolType IsForceClose;		//是否强平单
};
class ReqRiskCancelOrderField
{
public:
	static constexpr UShortType FieldID = 0x2012;
	UserIDType UserID;		//用户代码
	AccountIDType AccountID;		//账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	ClientOrderIDType ClientCancelOrderID;		//客户端撤单委托编号
	OrderIDType OrderID;		//委托编号
	OrderSysIDType OrderSysID;		//系统委托编号
	ClientOrderIDType ClientOrderID;		//客户端委托编号
};
class ReqMdUserLoginField
{
public:
	static constexpr UShortType FieldID = 0x3001;
	UserIDType MdUserID;		//行情用户代码
	PasswordType Password;		//密码
};
class RspMdUserLoginField
{
public:
	static constexpr UShortType FieldID = 0x3002;
	UserIDType MdUserID;		//行情用户代码
	DateType LoginDate;		//登录日期
	TimeType LoginTime;		//登录时间
	SessionIDType SessionID;		//会话编号
};
class ReqMdUserLogoutField
{
public:
	static constexpr UShortType FieldID = 0x3003;
	UserIDType MdUserID;		//行情用户代码
};
class RspMdUserLogoutField
{
public:
	static constexpr UShortType FieldID = 0x3004;
	UserIDType MdUserID;		//行情用户代码
};
class ReqMdInitField
{
public:
	static constexpr UShortType FieldID = 0x3005;
	ExchangeIDType ExchangeID;		//交易所代码
	DateType TradingDay;		//交易日
};
class RspMdInitField
{
public:
	static constexpr UShortType FieldID = 0x3006;
	ExchangeIDType ExchangeID;		//交易所代码
	DateType TradingDay;		//交易日
};
class ReqSubscribeMdField
{
public:
	static constexpr UShortType FieldID = 0x3007;
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
};
class RspSubscribeMdField
{
public:
	static constexpr UShortType FieldID = 0x3008;
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
};
class ShortMdField
{
public:
	static constexpr UShortType FieldID = 0x3009;
	DateType TradingDay;		//交易日
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	PriceType LastPrice;		//最新价
	PriceType ClosePrice;		//收盘价
	PriceType PreClosePrice;		//昨收盘价
	PriceType SettlementPrice;		//结算价
	PriceType PreSettlementPrice;		//昨结算价
	PriceType UpperLimitPrice;		//涨停板价
	PriceType LowerLimitPrice;		//跌停板价
};
class RtnExchangeStatusField
{
public:
	static constexpr UShortType FieldID = 0x300A;
	ExchangeIDType ExchangeID;		//交易所代码
	DateType ExchangeDate;		//交易所日期
	ExchangeStatusType ExchangeStatus;		//交易所状态
};
class MdInitCompletedField
{
public:
	static constexpr UShortType FieldID = 0x300B;
	ExchangeIDType ExchangeID;		//交易所代码
	DateType TradingDay;		//交易日
};
class ReqAdminUserLoginField
{
public:
	static constexpr UShortType FieldID = 0x4001;
	UserIDType AdminUserID;		//管理用户代码
	PasswordType Password;		//密码
};
class RspAdminUserLoginField
{
public:
	static constexpr UShortType FieldID = 0x4002;
	UserIDType AdminUserID;		//管理用户代码
	DateType LoginDate;		//登录日期
	TimeType LoginTime;		//登录时间
	SessionIDType SessionID;		//会话编号
};
class ReqAdminUserLogoutField
{
public:
	static constexpr UShortType FieldID = 0x4003;
	UserIDType AdminUserID;		//管理用户代码
};
class RspAdminUserLogoutField
{
public:
	static constexpr UShortType FieldID = 0x4004;
	UserIDType AdminUserID;		//管理用户代码
};
class AdminUserLogoutField
{
public:
	static constexpr UShortType FieldID = 0x4005;
	UserIDType UserID;		//用户代码
	ErrorIDType ErrorID;		//错误代码
	MessageType ErrorMsg;		//错误信息
};
class ReqAddRiskUserField
{
public:
	static constexpr UShortType FieldID = 0x4006;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
	UserNameType UserName;		//用户名称
	PasswordType Password;		//密码
	GroupIDType RiskGroupID;		//交易组代码
};
class RspAddRiskUserField
{
public:
	static constexpr UShortType FieldID = 0x4007;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
};
class ReqUpdateRiskUserField
{
public:
	static constexpr UShortType FieldID = 0x4008;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
	UserNameType UserName;		//用户名称
	PasswordType Password;		//密码
	GroupIDType RiskGroupID;		//交易组代码
};
class RspUpdateRiskUserField
{
public:
	static constexpr UShortType FieldID = 0x4009;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
};
class ReqRemoveRiskUserField
{
public:
	static constexpr UShortType FieldID = 0x400A;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
};
class RspRemoveRiskUserField
{
public:
	static constexpr UShortType FieldID = 0x400B;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
};
class ReqAddAdminUserField
{
public:
	static constexpr UShortType FieldID = 0x400C;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
	UserNameType UserName;		//用户名称
	PasswordType Password;		//密码
};
class RspAddAdminUserField
{
public:
	static constexpr UShortType FieldID = 0x400D;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
};
class ReqUpdateAdminUserField
{
public:
	static constexpr UShortType FieldID = 0x400E;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
	UserNameType UserName;		//用户名称
	PasswordType Password;		//密码
};
class RspUpdateAdminUserField
{
public:
	static constexpr UShortType FieldID = 0x400F;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
};
class ReqRemoveAdminUserField
{
public:
	static constexpr UShortType FieldID = 0x4010;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
};
class RspRemoveAdminUserField
{
public:
	static constexpr UShortType FieldID = 0x4011;
	UserIDType AdminUserID;		//管理用户代码
	UserIDType UserID;		//用户代码
};
class ReqAddPrimaryAccountField
{
public:
	static constexpr UShortType FieldID = 0x4012;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
	AccountNameType PrimaryAccountName;		//主账户名称
	AccountClassType AccountClass;		//账户类别
	PasswordType BrokerPassword;		//经纪公司密码
	OfferIDType OfferID;		//报盘代码
	BoolType IsAllowLogin;		//是否允许登陆
	BoolType IsSimulateAccount;		//是否模拟账号
	AccountStatusType AccountStatus;		//账户状态
	PasswordType Password;		//密码
	GroupIDType RiskGroupID;		//交易组代码
	GroupIDType CommissionGroupID;		//交易组代码
	BoolType IsAutoAudit;		//是否自动审核
};
class RspAddPrimaryAccountField
{
public:
	static constexpr UShortType FieldID = 0x4013;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType PrimaryAccountID;		//主账户代码
};
class ReqUpdatePrimaryAccountField
{
public:
	static constexpr UShortType FieldID = 0x4014;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
	AccountNameType PrimaryAccountName;		//主账户名称
	AccountClassType AccountClass;		//账户类别
	PasswordType BrokerPassword;		//经纪公司密码
	OfferIDType OfferID;		//报盘代码
	BoolType IsAllowLogin;		//是否允许登陆
	BoolType IsSimulateAccount;		//是否模拟账号
	AccountStatusType AccountStatus;		//账户状态
	PasswordType Password;		//密码
	GroupIDType RiskGroupID;		//交易组代码
	GroupIDType CommissionGroupID;		//交易组代码
	BoolType IsAutoAudit;		//是否自动审核
};
class RspUpdatePrimaryAccountField
{
public:
	static constexpr UShortType FieldID = 0x4015;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType PrimaryAccountID;		//主账户代码
};
class ReqRemovePrimaryAccountField
{
public:
	static constexpr UShortType FieldID = 0x4016;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType PrimaryAccountID;		//主账户代码
};
class RspRemovePrimaryAccountField
{
public:
	static constexpr UShortType FieldID = 0x4017;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType PrimaryAccountID;		//主账户代码
};
class ReqAddAccountField
{
public:
	static constexpr UShortType FieldID = 0x4018;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIDType AccountID;		//账户代码
	AccountNameType AccountName;		//账户名称
	AccountStatusType AccountStatus;		//账户状态
	PasswordType Password;		//密码
	GroupIDType TradeGroupID;		//交易组代码
	GroupIDType RiskGroupID;		//交易组代码
	GroupIDType CommissionGroupID;		//交易组代码
	BoolType IsAutoAudit;		//是否自动审核
};
class RspAddAccountField
{
public:
	static constexpr UShortType FieldID = 0x4019;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType AccountID;		//账户代码
};
class ReqUpdateAccountField
{
public:
	static constexpr UShortType FieldID = 0x401A;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIDType AccountID;		//账户代码
	AccountNameType AccountName;		//账户名称
	AccountStatusType AccountStatus;		//账户状态
	PasswordType Password;		//密码
	GroupIDType TradeGroupID;		//交易组代码
	GroupIDType RiskGroupID;		//交易组代码
	GroupIDType CommissionGroupID;		//交易组代码
	BoolType IsAutoAudit;		//是否自动审核
};
class RspUpdateAccountField
{
public:
	static constexpr UShortType FieldID = 0x401B;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType AccountID;		//账户代码
};
class ReqRemoveAccountField
{
public:
	static constexpr UShortType FieldID = 0x401C;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType AccountID;		//账户代码
};
class RspRemoveAccountField
{
public:
	static constexpr UShortType FieldID = 0x401D;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType AccountID;		//账户代码
};
class ReqAddBaseCommissionField
{
public:
	static constexpr UShortType FieldID = 0x401E;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
	RateType OpenStampTaxByMoney;		//开仓印花税率
	RateType CloseStampTaxByMoney;		//平仓印花税率
	RateType OpenTransferFeeByMoney;		//开仓过户费率
	RateType CloseTransferFeeByMoney;		//平仓过户费率
};
class RspAddBaseCommissionField
{
public:
	static constexpr UShortType FieldID = 0x401F;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqUpdateBaseCommissionField
{
public:
	static constexpr UShortType FieldID = 0x4020;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
	RateType OpenStampTaxByMoney;		//开仓印花税率
	RateType CloseStampTaxByMoney;		//平仓印花税率
	RateType OpenTransferFeeByMoney;		//开仓过户费率
	RateType CloseTransferFeeByMoney;		//平仓过户费率
};
class RspUpdateBaseCommissionField
{
public:
	static constexpr UShortType FieldID = 0x4021;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqRemoveBaseCommissionField
{
public:
	static constexpr UShortType FieldID = 0x4022;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class RspRemoveBaseCommissionField
{
public:
	static constexpr UShortType FieldID = 0x4023;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqAddCommissionGroupField
{
public:
	static constexpr UShortType FieldID = 0x4024;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIDType CommissionGroupID;		//交易组代码
	GroupNameType CommissionGroupName;		//手续费组名称
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
	RateType OpenBuyByMoney;		//买开仓费率
	RateType OpenSellByMoney;		//卖开仓费率
	RateType CloseBuyByMoney;		//买平仓费率
	RateType CloseSellByMoney;		//卖平仓费率
	RateType OpenBuyByVolume;		//每手买开仓费用
	RateType OpenSellByVolume;		//每手卖开仓费用
	RateType CloseBuyByVolume;		//每手买平仓费用
	RateType CloseSellByVolume;		//每手卖平仓费用
	MoneyType MinCommission;		//最低手续费
	MoneyType MaxCommission;		//最高手续费
};
class RspAddCommissionGroupField
{
public:
	static constexpr UShortType FieldID = 0x4025;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIDType CommissionGroupID;		//交易组代码
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqUpdateCommissionGroupField
{
public:
	static constexpr UShortType FieldID = 0x4026;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIDType CommissionGroupID;		//交易组代码
	GroupNameType CommissionGroupName;		//手续费组名称
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
	RateType OpenBuyByMoney;		//买开仓费率
	RateType OpenSellByMoney;		//卖开仓费率
	RateType CloseBuyByMoney;		//买平仓费率
	RateType CloseSellByMoney;		//卖平仓费率
	RateType OpenBuyByVolume;		//每手买开仓费用
	RateType OpenSellByVolume;		//每手卖开仓费用
	RateType CloseBuyByVolume;		//每手买平仓费用
	RateType CloseSellByVolume;		//每手卖平仓费用
	MoneyType MinCommission;		//最低手续费
	MoneyType MaxCommission;		//最高手续费
};
class RspUpdateCommissionGroupField
{
public:
	static constexpr UShortType FieldID = 0x4027;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIDType CommissionGroupID;		//交易组代码
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqRemoveCommissionGroupField
{
public:
	static constexpr UShortType FieldID = 0x4028;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIDType CommissionGroupID;		//交易组代码
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class RspRemoveCommissionGroupField
{
public:
	static constexpr UShortType FieldID = 0x4029;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIDType CommissionGroupID;		//交易组代码
	ExchangeIDType ExchangeID;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqAddOptionMarginParamField
{
public:
	static constexpr UShortType FieldID = 0x402A;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIDType ExchangeID;		//交易所代码
	RateType Param1;		//参数1
	RateType Param2;		//参数2
};
class RspAddOptionMarginParamField
{
public:
	static constexpr UShortType FieldID = 0x402B;
	UserIDType AdminUserID;		//管理用户代码
	ExchangeIDType ExchangeID;		//交易所代码
};
class ReqUpdateOptionMarginParamField
{
public:
	static constexpr UShortType FieldID = 0x402C;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIDType ExchangeID;		//交易所代码
	RateType Param1;		//参数1
	RateType Param2;		//参数2
};
class RspUpdateOptionMarginParamField
{
public:
	static constexpr UShortType FieldID = 0x402D;
	UserIDType AdminUserID;		//管理用户代码
	ExchangeIDType ExchangeID;		//交易所代码
};
class ReqRemoveOptionMarginParamField
{
public:
	static constexpr UShortType FieldID = 0x402E;
	UserIDType AdminUserID;		//管理用户代码
	ExchangeIDType ExchangeID;		//交易所代码
};
class RspRemoveOptionMarginParamField
{
public:
	static constexpr UShortType FieldID = 0x402F;
	UserIDType AdminUserID;		//管理用户代码
	ExchangeIDType ExchangeID;		//交易所代码
};
class ReqAddTradeOfferField
{
public:
	static constexpr UShortType FieldID = 0x4030;
	UserIDType AdminUserID;		//管理用户代码
	OfferIDType OfferID;		//报盘代码
	OfferNameType OfferName;		//报盘名称
	OfferTypeType OfferType;		//报盘类型
	PasswordType OfferPassword;		//报盘密码
};
class RspAddTradeOfferField
{
public:
	static constexpr UShortType FieldID = 0x4031;
	UserIDType AdminUserID;		//管理用户代码
	OfferIDType OfferID;		//报盘代码
};
class ReqUpdateTradeOfferField
{
public:
	static constexpr UShortType FieldID = 0x4032;
	UserIDType AdminUserID;		//管理用户代码
	OfferIDType OfferID;		//报盘代码
	OfferNameType OfferName;		//报盘名称
	OfferTypeType OfferType;		//报盘类型
	PasswordType OfferPassword;		//报盘密码
};
class RspUpdateTradeOfferField
{
public:
	static constexpr UShortType FieldID = 0x4033;
	UserIDType AdminUserID;		//管理用户代码
	OfferIDType OfferID;		//报盘代码
};
class ReqRemoveTradeOfferField
{
public:
	static constexpr UShortType FieldID = 0x4034;
	UserIDType AdminUserID;		//管理用户代码
	OfferIDType OfferID;		//报盘代码
};
class RspRemoveTradeOfferField
{
public:
	static constexpr UShortType FieldID = 0x4035;
	UserIDType AdminUserID;		//管理用户代码
	OfferIDType OfferID;		//报盘代码
};
class ReqAddTradeGroupField
{
public:
	static constexpr UShortType FieldID = 0x4036;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
	GroupNameType TradeGroupName;		//交易组名称
};
class RspAddTradeGroupField
{
public:
	static constexpr UShortType FieldID = 0x4037;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
};
class ReqUpdateTradeGroupField
{
public:
	static constexpr UShortType FieldID = 0x4038;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
	GroupNameType TradeGroupName;		//交易组名称
};
class RspUpdateTradeGroupField
{
public:
	static constexpr UShortType FieldID = 0x4039;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
};
class ReqRemoveTradeGroupField
{
public:
	static constexpr UShortType FieldID = 0x403A;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
};
class RspRemoveTradeGroupField
{
public:
	static constexpr UShortType FieldID = 0x403B;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
};
class ReqAddTradeGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x403C;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
	AccountClassType AccountClass;		//账户类别
	AccountIDType PrimaryAccountID;		//主账户代码
};
class RspAddTradeGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x403D;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
	AccountClassType AccountClass;		//账户类别
};
class ReqUpdateTradeGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x403E;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
	AccountClassType AccountClass;		//账户类别
	AccountIDType PrimaryAccountID;		//主账户代码
};
class RspUpdateTradeGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x403F;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
	AccountClassType AccountClass;		//账户类别
};
class ReqRemoveTradeGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x4040;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
	AccountClassType AccountClass;		//账户类别
};
class RspRemoveTradeGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x4041;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType TradeGroupID;		//交易组代码
	AccountClassType AccountClass;		//账户类别
};
class ReqAddRiskGroupField
{
public:
	static constexpr UShortType FieldID = 0x4042;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
	GroupNameType RiskGroupName;		//风控组名称
};
class RspAddRiskGroupField
{
public:
	static constexpr UShortType FieldID = 0x4043;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
};
class ReqUpdateRiskGroupField
{
public:
	static constexpr UShortType FieldID = 0x4044;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
	GroupNameType RiskGroupName;		//风控组名称
};
class RspUpdateRiskGroupField
{
public:
	static constexpr UShortType FieldID = 0x4045;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
};
class ReqRemoveRiskGroupField
{
public:
	static constexpr UShortType FieldID = 0x4046;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
};
class RspRemoveRiskGroupField
{
public:
	static constexpr UShortType FieldID = 0x4047;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
};
class ReqAddRiskGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x4048;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
	RiskIDType RiskID;		//风控代码
};
class RspAddRiskGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x4049;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
	RiskIDType RiskID;		//风控代码
};
class ReqUpdateRiskGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x404A;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
	RiskIDType RiskID;		//风控代码
};
class RspUpdateRiskGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x404B;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
	RiskIDType RiskID;		//风控代码
};
class ReqRemoveRiskGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x404C;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
	RiskIDType RiskID;		//风控代码
};
class RspRemoveRiskGroupItemField
{
public:
	static constexpr UShortType FieldID = 0x404D;
	UserIDType AdminUserID;		//管理用户代码
	GroupIDType RiskGroupID;		//交易组代码
	RiskIDType RiskID;		//风控代码
};
class ReqAddOrUpdateRiskField
{
public:
	static constexpr UShortType FieldID = 0x404E;
	UserIDType AdminUserID;		//管理用户代码
	RiskIDType RiskID;		//风控代码
	RiskNameType RiskName;		//风控名称
	RiskClassIDType RiskClassID;		//风控类别代码
};
class RspAddOrUpdateRiskField
{
public:
	static constexpr UShortType FieldID = 0x404F;
	UserIDType AdminUserID;		//管理用户代码
	RiskIDType RiskID;		//风控代码
};
class ReqRemoveRiskField
{
public:
	static constexpr UShortType FieldID = 0x4050;
	UserIDType AdminUserID;		//管理用户代码
	RiskIDType RiskID;		//风控代码
};
class RspRemoveRiskField
{
public:
	static constexpr UShortType FieldID = 0x4051;
	UserIDType AdminUserID;		//管理用户代码
	RiskIDType RiskID;		//风控代码
};
class ReqAddRiskRuleField
{
public:
	static constexpr UShortType FieldID = 0x4052;
	UserIDType AdminUserID;		//管理用户代码
	RiskIDType RiskID;		//风控代码
	RiskRuleIDType RiskRuleID;		//风控规则代码
	RiskRuleStyleType RiskRuleStyle;		//风控类型
	MessageType FormatRiskMessage;		//带格式的风控信息
};
class RspAddRiskRuleField
{
public:
	static constexpr UShortType FieldID = 0x4053;
	UserIDType AdminUserID;		//管理用户代码
	RiskIDType RiskID;		//风控代码
	RiskRuleIDType RiskRuleID;		//风控规则代码
};
class ReqAddRiskRuleItemField
{
public:
	static constexpr UShortType FieldID = 0x4054;
	UserIDType AdminUserID;		//管理用户代码
	RiskIDType RiskID;		//风控代码
	RiskRuleIDType RiskRuleID;		//风控规则代码
	RiskRuleItemIDType RiskRuleItemID;		//风控规则分项编号
	RiskIndexTypeType RiskIndexType;		//风控指标类型
	RiskIndexIDType RiskIndexID;		//风控指标代码
	RiskTextRefType RiskIndexTextRef;		//风控指标文本引用序号
	ParamValueType IndexParam1;		//指标参数1
	ParamValueType IndexParam2;		//指标参数2
	ParamValueType IndexParam3;		//指标参数3
	ParamValueType IndexParam4;		//指标参数4
	ParamValueType IndexParam5;		//指标参数5
	ParamValueType IndexParam6;		//指标参数6
	LogicFuncType LogicFunc;		//逻辑函数
	ParamTypeType LogicParamType1;		//逻辑函数参数类型1
	ParamValueType LogicParam1;		//逻辑函数参数1
	RiskTextRefType LogicParam1TextRef;		//逻辑参数1文本引用序号
	ParamTypeType LogicParamType2;		//逻辑函数参数类型2
	ParamValueType LogicParam2;		//逻辑函数参数2
	RiskTextRefType LogicParam2TextRef;		//逻辑参数2文本引用序号
};
class RspAddRiskRuleItemField
{
public:
	static constexpr UShortType FieldID = 0x4055;
	UserIDType AdminUserID;		//管理用户代码
	RiskIDType RiskID;		//风控代码
	RiskRuleIDType RiskRuleID;		//风控规则代码
	RiskRuleItemIDType RiskRuleItemID;		//风控规则分项编号
};
class ReqAddRiskTradeScopeField
{
public:
	static constexpr UShortType FieldID = 0x4056;
	UserIDType AdminUserID;		//管理用户代码
	RiskIDType RiskID;		//风控代码
	TradeScopeTypeType TradeScopeType;		//交易范围类别
	GroupIDType InstrumentGroupID;		//合约组代码
	MessageType FormatRiskMessage;		//带格式的风控信息
};
class RspAddRiskTradeScopeField
{
public:
	static constexpr UShortType FieldID = 0x4057;
	UserIDType AdminUserID;		//管理用户代码
	RiskIDType RiskID;		//风控代码
};
class ReqAddAccountRiskField
{
public:
	static constexpr UShortType FieldID = 0x4058;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType AccountID;		//账户代码
	RiskIDType RiskID;		//风控代码
};
class RspAddAccountRiskField
{
public:
	static constexpr UShortType FieldID = 0x4059;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType AccountID;		//账户代码
	RiskIDType RiskID;		//风控代码
};
class ReqRemoveAccountRiskField
{
public:
	static constexpr UShortType FieldID = 0x405A;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType AccountID;		//账户代码
	RiskIDType RiskID;		//风控代码
};
class RspRemoveAccountRiskField
{
public:
	static constexpr UShortType FieldID = 0x405B;
	UserIDType AdminUserID;		//管理用户代码
	AccountIDType AccountID;		//账户代码
	RiskIDType RiskID;		//风控代码
};
class ReqMoneyTransferField
{
public:
	static constexpr UShortType FieldID = 0x405C;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIDType AccountID;		//账户代码
	SequenceNoType MoneyTransferID;		//出入金编号
	AccountTypeType AccountType;		//账户类型
	TransferDirectionType TransferDirection;		//转移方向
	MoneyType TransferAmount;		//转移金额
	MessageType InfoMessage;		//备注信息
	UserIDType UserID;		//用户代码
	DateType TransferDate;		//操作日期
	TimeType TransferTime;		//操作时间
};
class RspMoneyTransferField
{
public:
	static constexpr UShortType FieldID = 0x405D;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIDType AccountID;		//账户代码
	SequenceNoType MoneyTransferID;		//出入金编号
};
class ReqAuditOrderField
{
public:
	static constexpr UShortType FieldID = 0x405E;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIDType AccountID;		//账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	OrderIDType OrderID;		//委托编号
	AuditStatusType AuditStatus;		//审核状态
};
class RspAuditOrderField
{
public:
	static constexpr UShortType FieldID = 0x405F;
	UserIDType AdminUserID;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIDType AccountID;		//账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	OrderIDType OrderID;		//委托编号
	AuditStatusType AuditStatus;		//审核状态
};
class ReqOfferLoginField
{
public:
	static constexpr UShortType FieldID = 0x5001;
	OfferIDType OfferID;		//报盘代码
	PasswordType OfferPassword;		//报盘密码
};
class RspOfferLoginField
{
public:
	static constexpr UShortType FieldID = 0x5002;
	ErrorIDType ErrorID;		//错误代码
	MessageType ErrorMsg;		//错误信息
	OfferIDType OfferID;		//报盘代码
	DateType TradingDay;		//交易日
};
class ReqPrimaryAccountLoginField
{
public:
	static constexpr UShortType FieldID = 0x5003;
	AccountIDType PrimaryAccountID;		//主账户代码
	PasswordType Password;		//密码
};
class RspPrimaryAccountLoginField
{
public:
	static constexpr UShortType FieldID = 0x5004;
	ErrorIDType ErrorID;		//错误代码
	MessageType ErrorMsg;		//错误信息
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
};
class ReqPrimaryAccountLogoutField
{
public:
	static constexpr UShortType FieldID = 0x5005;
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
};
class RtnPrimaryAccountLogoutField
{
public:
	static constexpr UShortType FieldID = 0x5006;
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
};
class ReqPrimaryAccountInitField
{
public:
	static constexpr UShortType FieldID = 0x5007;
	AccountIDType PrimaryAccountID;		//主账户代码
};
class RspPrimaryAccountInitField
{
public:
	static constexpr UShortType FieldID = 0x5008;
	ErrorIDType ErrorID;		//错误代码
	MessageType ErrorMsg;		//错误信息
	AccountIDType PrimaryAccountID;		//主账户代码
};
class ReqPrimaryAccountQueryField
{
public:
	static constexpr UShortType FieldID = 0x5009;
	AccountIDType PrimaryAccountID;		//主账户代码
};
class RspPrimaryAccountQueryField
{
public:
	static constexpr UShortType FieldID = 0x500A;
	ErrorIDType ErrorID;		//错误代码
	MessageType ErrorMsg;		//错误信息
	AccountIDType PrimaryAccountID;		//主账户代码
};
class ReqQryOfferOptionInstrumentField
{
public:
	static constexpr UShortType FieldID = 0x500B;
	AccountIDType PrimaryAccountID;		//主账户代码
};
class RspQryOfferOptionInstrumentField
{
public:
	static constexpr UShortType FieldID = 0x500C;
	ErrorIDType ErrorID;		//错误代码
	MessageType ErrorMsg;		//错误信息
	AccountIDType PrimaryAccountID;		//主账户代码
};
class OfferOptionInstrumentField
{
public:
	static constexpr UShortType FieldID = 0x500D;
	DateType TradingDay;		//交易日
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	InstrumentIDType ExchangeInstID;		//交易所合约代码
	InstrumentNameType InstrumentName;		//合约名称
	VolumeMultipleType VolumeMultiple;		//合约乘数
	OptionTypeType OptionType;		//期权类型
	InstrumentIDType UnderlyingInstrumentID;		//标的合约代码
	PriceType ExecutePrice;		//行权价
	MoneyType UnitMargin;		//单位保证金
	PriceType PriceTick;		//最小变动价位
	VolumeType MaxLimitOrderVolume;		//限价最大下单量
	VolumeType MaxMarketOrderVolume;		//市价最大下单量
	DateType ExpiringDate;		//到期日
};
class ReqOfferOrderField
{
public:
	static constexpr UShortType FieldID = 0x500E;
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	ProductClassType ProductClass;		//品种类型
	OrderIDType OrderID;		//委托编号
	DirectionType Direction;		//买卖方向
	OffsetFlagType OffsetFlag;		//开平标志
	OrderPriceTypeType OrderPriceType;		//委托价格类型
	PriceType Price;		//委托价格
	VolumeType Volume;		//委托数量
};
class ReqOfferCancelOrderField
{
public:
	static constexpr UShortType FieldID = 0x500F;
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	ProductClassType ProductClass;		//品种类型
	DirectionType Direction;		//买卖方向
	OrderIDType CancelOrderID;		//本地撤单编号
	OrderIDType OrderID;		//委托编号
	OrderSysIDType OrderSysID;		//系统委托编号
};
class OfferOrderField
{
public:
	static constexpr UShortType FieldID = 0x5010;
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	OrderIDType OrderID;		//委托编号
	OrderSysIDType OrderSysID;		//系统委托编号
	DirectionType Direction;		//买卖方向
	OffsetFlagType OffsetFlag;		//开平标志
	OrderPriceTypeType OrderPriceType;		//委托价格类型
	PriceType Price;		//委托价格
	VolumeType Volume;		//委托数量
	VolumeType VolumeTotal;		//剩余数量
	VolumeType VolumeTraded;		//成交数量
	OrderStatusType OrderStatus;		//委托状态
	MessageType StatusMsg;		//状态信息
	DateType OrderDate;		//委托日期
	TimeType OrderTime;		//委托时间
	DateType CancelDate;		//撤单日期
	TimeType CancelTime;		//撤单时间
	BoolType IsNewOrder;		//是否新委托
};
class OfferTradeField
{
public:
	static constexpr UShortType FieldID = 0x5011;
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	OrderIDType OrderID;		//委托编号
	OrderSysIDType OrderSysID;		//系统委托编号
	TradeIDType TradeID;		//成交编号
	DirectionType Direction;		//买卖方向
	OffsetFlagType OffsetFlag;		//开平标志
	PriceType Price;		//委托价格
	VolumeType Volume;		//委托数量
	DateType TradeDate;		//成交日期
	TimeType TradeTime;		//成交时间
};
class OfferErrorCancelOrderField
{
public:
	static constexpr UShortType FieldID = 0x5012;
	ErrorIDType ErrorID;		//错误代码
	MessageType ErrorMsg;		//错误信息
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	DirectionType Direction;		//买卖方向
	OrderIDType CancelOrderID;		//本地撤单编号
	OrderIDType OrderID;		//委托编号
	OrderSysIDType OrderSysID;		//系统委托编号
};
class OfferCapitalField
{
public:
	static constexpr UShortType FieldID = 0x5013;
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
	MoneyType PreCashAsset;		//上日现金资产
};
class OfferPositionField
{
public:
	static constexpr UShortType FieldID = 0x5014;
	DateType TradingDay;		//交易日
	AccountIDType PrimaryAccountID;		//主账户代码
	ExchangeIDType ExchangeID;		//交易所代码
	InstrumentIDType InstrumentID;		//合约代码
	PosiDirectionType PosiDirection;		//持仓方向
	VolumeType TotalPosition;		//持仓数量
	VolumeType PositionFrozen;		//冻结持仓
	PriceType TotalCostPrice;		//总成本价
	MoneyType Margin;		//保证金
	MoneyType MarketValue;		//市值
};
