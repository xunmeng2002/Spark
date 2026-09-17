// 本文件由 ../Templates/Cpp/Protocol/Packages/Fields.h.tpl 生成；请勿手改，改动请改模板后重跑 pumpall.py
#pragma once
#include <Spark/Types.h>

class RspInfoField
{
public:
	static constexpr UInt16Type FieldId = 0x0003;
	ErrorIdType ErrorId;		//错误代码
	MessageType ErrorMsg;		//错误信息
};
class NotifyComponentConnectStatusField
{
public:
	static constexpr UInt16Type FieldId = 0x0004;
	SessionIdType SessionId;		//会话编号
	ComponentType Component;		//组件类型
	BoolType IsConnected;		//是否链接
};
class ReqAccountLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x1001;
	AccountIdType AccountId;		//账户代码
	PasswordType Password;		//密码
};
class RspAccountLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x1002;
	AccountIdType AccountId;		//账户代码
	DateType LoginDate;		//登录日期
	TimeType LoginTime;		//登录时间
	SessionIdType SessionId;		//会话编号
};
class ReqAccountLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x1003;
	AccountIdType AccountId;		//账户代码
};
class RspAccountLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x1004;
	AccountIdType AccountId;		//账户代码
};
class AccountLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x1005;
	AccountIdType AccountId;		//账户代码
	ErrorIdType ErrorId;		//错误代码
	MessageType ErrorMsg;		//错误信息
};
class ReqQryAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x1006;
	AccountIdType AccountId;		//账户代码
};
class AccountField
{
public:
	static constexpr UInt16Type FieldId = 0x1007;
	AccountIdType AccountId;		//账户代码
	AccountTypeType AccountType;		//账户类型
	AccountStatusType AccountStatus;		//账户状态
	GroupIdType TradeGroupId;		//交易组代码
	GroupIdType RiskGroupId;		//交易组代码
	GroupIdType CommissionGroupId;		//交易组代码
};
class ReqQryHolderAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x1008;
	AccountIdType AccountId;		//账户代码
};
class HolderAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x1009;
	ExchangeIdType ExchangeId;		//交易所代码
	AccountIdType HolderAccountId;		//股东账户代码
	BoolType PrimaryFlag;		//主账号标志
};
class ReqQryCapitalField
{
public:
	static constexpr UInt16Type FieldId = 0x100A;
	AccountIdType AccountId;		//账户代码
};
class CapitalField
{
public:
	static constexpr UInt16Type FieldId = 0x100B;
	DateType TradingDay;		//交易日
	AccountIdType AccountId;		//账户代码
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
	static constexpr UInt16Type FieldId = 0x100C;
	AccountIdType AccountId;		//账户代码
};
class PositionField
{
public:
	static constexpr UInt16Type FieldId = 0x100D;
	DateType TradingDay;		//交易日
	AccountIdType AccountId;		//账户代码
	AccountTypeType AccountType;		//账户类型
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
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
	static constexpr UInt16Type FieldId = 0x100E;
	AccountIdType AccountId;		//账户代码
};
class OrderField
{
public:
	static constexpr UInt16Type FieldId = 0x100F;
	DateType TradingDay;		//交易日
	AccountIdType AccountId;		//账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	ProductClassType ProductClass;		//品种类型
	OrderIdType OrderId;		//委托编号
	OrderSysIdType OrderSysId;		//系统委托编号
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
	SessionIdType SessionId;		//会话编号
	ClientOrderIdType ClientOrderId;		//客户端委托编号
	RequestIdType RequestId;		//客户端请求编号
	MoneyType FrozenCash;		//冻结资金
	MoneyType FrozenMargin;		//冻结保证金
	MoneyType FrozenCommission;		//冻结手续费
	MoneyType FrozenStampTax;		//冻结印花税
	MoneyType FrozenTransferFee;		//冻结过户费
};
class ReqQryTradeField
{
public:
	static constexpr UInt16Type FieldId = 0x1010;
	AccountIdType AccountId;		//账户代码
};
class TradeField
{
public:
	static constexpr UInt16Type FieldId = 0x1011;
	DateType TradingDay;		//交易日
	AccountIdType AccountId;		//账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	ProductClassType ProductClass;		//品种类型
	OrderIdType OrderId;		//委托编号
	OrderSysIdType OrderSysId;		//系统委托编号
	TradeIdType TradeId;		//成交编号
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
	static constexpr UInt16Type FieldId = 0x1012;
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
};
class InstrumentField
{
public:
	static constexpr UInt16Type FieldId = 0x1013;
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	InstrumentIdType ExchangeInstId;		//交易所合约代码
	InstrumentNameType InstrumentName;		//合约名称
	VolumeMultipleType VolumeMultiple;		//合约乘数
	ProductClassType ProductClass;		//品种类型
};
class ReqQryOptionInstrumentField
{
public:
	static constexpr UInt16Type FieldId = 0x1014;
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
};
class OptionInstrumentField
{
public:
	static constexpr UInt16Type FieldId = 0x1015;
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	InstrumentIdType ExchangeInstId;		//交易所合约代码
	InstrumentNameType InstrumentName;		//合约名称
	VolumeMultipleType VolumeMultiple;		//合约乘数
	OptionTypeType OptionType;		//期权类型
	InstrumentIdType UnderlyingInstrumentId;		//标的合约代码
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
	static constexpr UInt16Type FieldId = 0x1016;
	AccountIdType AccountId;		//账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class CommissionRateField
{
public:
	static constexpr UInt16Type FieldId = 0x1017;
	AccountIdType AccountId;		//账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
	RateType OpenByMoney;		//开仓费率
	RateType CloseByMoney;		//平仓费率
	RateType OpenByVolume;		//每手开仓费用
	RateType CloseByVolume;		//每手平仓费用
	MoneyType MinCommission;		//最低手续费
	MoneyType MaxCommission;		//最高手续费
};
class ReqQryMoneyTransferField
{
public:
	static constexpr UInt16Type FieldId = 0x1018;
	AccountIdType AccountId;		//账户代码
};
class MoneyTransferField
{
public:
	static constexpr UInt16Type FieldId = 0x1019;
	DateType TradingDay;		//交易日
	AccountIdType AccountId;		//账户代码
	SequenceNoType MoneyTransferId;		//出入金编号
	AccountTypeType AccountType;		//账户类型
	TransferDirectionType TransferDirection;		//转移方向
	MoneyType TransferAmount;		//转移金额
	MessageType InfoMessage;		//备注信息
	UserIdType UserId;		//用户代码
	DateType TransferDate;		//操作日期
	TimeType TransferTime;		//操作时间
};
class ReqInsertOrderField
{
public:
	static constexpr UInt16Type FieldId = 0x101A;
	AccountIdType AccountId;		//账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	DirectionType Direction;		//买卖方向
	OffsetFlagType OffsetFlag;		//开平标志
	OrderPriceTypeType OrderPriceType;		//委托价格类型
	PriceType Price;		//委托价格
	VolumeType Volume;		//委托数量
	ClientOrderIdType ClientOrderId;		//客户端委托编号
};
class ReqCancelOrderField
{
public:
	static constexpr UInt16Type FieldId = 0x101B;
	AccountIdType AccountId;		//账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	ClientOrderIdType ClientCancelOrderId;		//客户端撤单委托编号
	OrderIdType OrderId;		//委托编号
	OrderSysIdType OrderSysId;		//系统委托编号
	ClientOrderIdType ClientOrderId;		//客户端委托编号
};
class CancelOrderField
{
public:
	static constexpr UInt16Type FieldId = 0x101C;
	AccountIdType AccountId;		//账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	ClientOrderIdType ClientCancelOrderId;		//客户端撤单委托编号
	OrderIdType OrderId;		//委托编号
	OrderSysIdType OrderSysId;		//系统委托编号
	ClientOrderIdType ClientOrderId;		//客户端委托编号
};
class ReqRiskUserLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x2001;
	UserIdType UserId;		//用户代码
	PasswordType Password;		//密码
};
class RspRiskUserLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x2002;
	UserIdType UserId;		//用户代码
	GroupIdType RiskGroupId;		//交易组代码
	DateType LoginDate;		//登录日期
	TimeType LoginTime;		//登录时间
	SessionIdType SessionId;		//会话编号
};
class ReqRiskUserLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x2003;
	UserIdType UserId;		//用户代码
};
class RspRiskUserLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x2004;
	UserIdType UserId;		//用户代码
};
class RiskUserLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x2005;
	UserIdType UserId;		//用户代码
	ErrorIdType ErrorId;		//错误代码
	MessageType ErrorMsg;		//错误信息
};
class AccountDeleteField
{
public:
	static constexpr UInt16Type FieldId = 0x2006;
	AccountIdType AccountId;		//账户代码
};
class AccountRiskField
{
public:
	static constexpr UInt16Type FieldId = 0x2007;
	AccountIdType AccountId;		//账户代码
	RiskIdType RiskId;		//风控代码
	RiskStatusType RiskStatus;		//风控状态
};
class AccountRiskDeleteField
{
public:
	static constexpr UInt16Type FieldId = 0x2008;
	AccountIdType AccountId;		//账户代码
	RiskIdType RiskId;		//风控代码
};
class AccountRiskNotifyField
{
public:
	static constexpr UInt16Type FieldId = 0x2009;
	DateType NotifyDate;		//通知日期
	TimeType NotifyTime;		//通知时间
	AccountIdType AccountId;		//账户代码
	RiskIdType RiskId;		//风控代码
	RiskRuleIdType RiskRuleId;		//风控规则代码
	RiskStatusType RiskStatus;		//风控状态
	MessageType RiskMessage;		//风控信息
};
class ReqQryRiskGroupAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x200A;
	UserIdType UserId;		//用户代码
};
class ReqQryRiskGroupCapitalField
{
public:
	static constexpr UInt16Type FieldId = 0x200B;
	UserIdType UserId;		//用户代码
	AccountIdType AccountId;		//账户代码
};
class ReqQryRiskGroupPositionField
{
public:
	static constexpr UInt16Type FieldId = 0x200C;
	UserIdType UserId;		//用户代码
	AccountIdType AccountId;		//账户代码
};
class ReqQryRiskGroupOrderField
{
public:
	static constexpr UInt16Type FieldId = 0x200D;
	UserIdType UserId;		//用户代码
	AccountIdType AccountId;		//账户代码
};
class ReqQryRiskGroupTradeField
{
public:
	static constexpr UInt16Type FieldId = 0x200E;
	UserIdType UserId;		//用户代码
	AccountIdType AccountId;		//账户代码
};
class ReqQryRiskGroupAccountRiskField
{
public:
	static constexpr UInt16Type FieldId = 0x200F;
	UserIdType UserId;		//用户代码
	AccountIdType AccountId;		//账户代码
};
class ReqQryRiskGroupAccountRiskNotifyField
{
public:
	static constexpr UInt16Type FieldId = 0x2010;
	UserIdType UserId;		//用户代码
	AccountIdType AccountId;		//账户代码
};
class ReqRiskInsertOrderField
{
public:
	static constexpr UInt16Type FieldId = 0x2011;
	UserIdType UserId;		//用户代码
	AccountIdType AccountId;		//账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	DirectionType Direction;		//买卖方向
	OffsetFlagType OffsetFlag;		//开平标志
	OrderPriceTypeType OrderPriceType;		//委托价格类型
	PriceType Price;		//委托价格
	VolumeType Volume;		//委托数量
	ClientOrderIdType ClientOrderId;		//客户端委托编号
	BoolType IsForceClose;		//是否强平单
};
class ReqRiskCancelOrderField
{
public:
	static constexpr UInt16Type FieldId = 0x2012;
	UserIdType UserId;		//用户代码
	AccountIdType AccountId;		//账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	ClientOrderIdType ClientCancelOrderId;		//客户端撤单委托编号
	OrderIdType OrderId;		//委托编号
	OrderSysIdType OrderSysId;		//系统委托编号
	ClientOrderIdType ClientOrderId;		//客户端委托编号
};
class ReqMdUserLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x3001;
	UserIdType MdUserId;		//行情用户代码
	PasswordType Password;		//密码
};
class RspMdUserLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x3002;
	UserIdType MdUserId;		//行情用户代码
	DateType LoginDate;		//登录日期
	TimeType LoginTime;		//登录时间
	SessionIdType SessionId;		//会话编号
};
class ReqMdUserLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x3003;
	UserIdType MdUserId;		//行情用户代码
};
class RspMdUserLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x3004;
	UserIdType MdUserId;		//行情用户代码
};
class ReqMdInitField
{
public:
	static constexpr UInt16Type FieldId = 0x3005;
	ExchangeIdType ExchangeId;		//交易所代码
	DateType TradingDay;		//交易日
};
class RspMdInitField
{
public:
	static constexpr UInt16Type FieldId = 0x3006;
	ExchangeIdType ExchangeId;		//交易所代码
	DateType TradingDay;		//交易日
};
class ReqSubscribeMdField
{
public:
	static constexpr UInt16Type FieldId = 0x3007;
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
};
class RspSubscribeMdField
{
public:
	static constexpr UInt16Type FieldId = 0x3008;
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
};
class ShortMdField
{
public:
	static constexpr UInt16Type FieldId = 0x3009;
	DateType TradingDay;		//交易日
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
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
	static constexpr UInt16Type FieldId = 0x300A;
	ExchangeIdType ExchangeId;		//交易所代码
	DateType ExchangeDate;		//交易所日期
	ExchangeStatusType ExchangeStatus;		//交易所状态
};
class MdInitCompletedField
{
public:
	static constexpr UInt16Type FieldId = 0x300B;
	ExchangeIdType ExchangeId;		//交易所代码
	DateType TradingDay;		//交易日
};
class ReqAdminUserLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x4001;
	UserIdType AdminUserId;		//管理用户代码
	PasswordType Password;		//密码
};
class RspAdminUserLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x4002;
	UserIdType AdminUserId;		//管理用户代码
	DateType LoginDate;		//登录日期
	TimeType LoginTime;		//登录时间
	SessionIdType SessionId;		//会话编号
};
class ReqAdminUserLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x4003;
	UserIdType AdminUserId;		//管理用户代码
};
class RspAdminUserLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x4004;
	UserIdType AdminUserId;		//管理用户代码
};
class AdminUserLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x4005;
	UserIdType UserId;		//用户代码
	ErrorIdType ErrorId;		//错误代码
	MessageType ErrorMsg;		//错误信息
};
class ReqAddRiskUserField
{
public:
	static constexpr UInt16Type FieldId = 0x4006;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
	UserNameType UserName;		//用户名称
	PasswordType Password;		//密码
	GroupIdType RiskGroupId;		//交易组代码
};
class RspAddRiskUserField
{
public:
	static constexpr UInt16Type FieldId = 0x4007;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
};
class ReqUpdateRiskUserField
{
public:
	static constexpr UInt16Type FieldId = 0x4008;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
	UserNameType UserName;		//用户名称
	PasswordType Password;		//密码
	GroupIdType RiskGroupId;		//交易组代码
};
class RspUpdateRiskUserField
{
public:
	static constexpr UInt16Type FieldId = 0x4009;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
};
class ReqRemoveRiskUserField
{
public:
	static constexpr UInt16Type FieldId = 0x400A;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
};
class RspRemoveRiskUserField
{
public:
	static constexpr UInt16Type FieldId = 0x400B;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
};
class ReqAddAdminUserField
{
public:
	static constexpr UInt16Type FieldId = 0x400C;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
	UserNameType UserName;		//用户名称
	PasswordType Password;		//密码
};
class RspAddAdminUserField
{
public:
	static constexpr UInt16Type FieldId = 0x400D;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
};
class ReqUpdateAdminUserField
{
public:
	static constexpr UInt16Type FieldId = 0x400E;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
	UserNameType UserName;		//用户名称
	PasswordType Password;		//密码
};
class RspUpdateAdminUserField
{
public:
	static constexpr UInt16Type FieldId = 0x400F;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
};
class ReqRemoveAdminUserField
{
public:
	static constexpr UInt16Type FieldId = 0x4010;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
};
class RspRemoveAdminUserField
{
public:
	static constexpr UInt16Type FieldId = 0x4011;
	UserIdType AdminUserId;		//管理用户代码
	UserIdType UserId;		//用户代码
};
class ReqAddPrimaryAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x4012;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
	AccountNameType PrimaryAccountName;		//主账户名称
	AccountClassType AccountClass;		//账户类别
	PasswordType BrokerPassword;		//经纪公司密码
	OfferIdType OfferId;		//报盘代码
	BoolType IsAllowLogin;		//是否允许登陆
	BoolType IsSimulateAccount;		//是否模拟账号
	AccountStatusType AccountStatus;		//账户状态
	PasswordType Password;		//密码
	GroupIdType RiskGroupId;		//交易组代码
	GroupIdType CommissionGroupId;		//交易组代码
	BoolType IsAutoAudit;		//是否自动审核
};
class RspAddPrimaryAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x4013;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType PrimaryAccountId;		//主账户代码
};
class ReqUpdatePrimaryAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x4014;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
	AccountNameType PrimaryAccountName;		//主账户名称
	AccountClassType AccountClass;		//账户类别
	PasswordType BrokerPassword;		//经纪公司密码
	OfferIdType OfferId;		//报盘代码
	BoolType IsAllowLogin;		//是否允许登陆
	BoolType IsSimulateAccount;		//是否模拟账号
	AccountStatusType AccountStatus;		//账户状态
	PasswordType Password;		//密码
	GroupIdType RiskGroupId;		//交易组代码
	GroupIdType CommissionGroupId;		//交易组代码
	BoolType IsAutoAudit;		//是否自动审核
};
class RspUpdatePrimaryAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x4015;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType PrimaryAccountId;		//主账户代码
};
class ReqRemovePrimaryAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x4016;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType PrimaryAccountId;		//主账户代码
};
class RspRemovePrimaryAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x4017;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType PrimaryAccountId;		//主账户代码
};
class ReqAddAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x4018;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIdType AccountId;		//账户代码
	AccountNameType AccountName;		//账户名称
	AccountStatusType AccountStatus;		//账户状态
	PasswordType Password;		//密码
	GroupIdType TradeGroupId;		//交易组代码
	GroupIdType RiskGroupId;		//交易组代码
	GroupIdType CommissionGroupId;		//交易组代码
	BoolType IsAutoAudit;		//是否自动审核
};
class RspAddAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x4019;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType AccountId;		//账户代码
};
class ReqUpdateAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x401A;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIdType AccountId;		//账户代码
	AccountNameType AccountName;		//账户名称
	AccountStatusType AccountStatus;		//账户状态
	PasswordType Password;		//密码
	GroupIdType TradeGroupId;		//交易组代码
	GroupIdType RiskGroupId;		//交易组代码
	GroupIdType CommissionGroupId;		//交易组代码
	BoolType IsAutoAudit;		//是否自动审核
};
class RspUpdateAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x401B;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType AccountId;		//账户代码
};
class ReqRemoveAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x401C;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType AccountId;		//账户代码
};
class RspRemoveAccountField
{
public:
	static constexpr UInt16Type FieldId = 0x401D;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType AccountId;		//账户代码
};
class ReqAddBaseCommissionField
{
public:
	static constexpr UInt16Type FieldId = 0x401E;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
	RateType OpenStampTaxByMoney;		//开仓印花税率
	RateType CloseStampTaxByMoney;		//平仓印花税率
	RateType OpenTransferFeeByMoney;		//开仓过户费率
	RateType CloseTransferFeeByMoney;		//平仓过户费率
};
class RspAddBaseCommissionField
{
public:
	static constexpr UInt16Type FieldId = 0x401F;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqUpdateBaseCommissionField
{
public:
	static constexpr UInt16Type FieldId = 0x4020;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
	RateType OpenStampTaxByMoney;		//开仓印花税率
	RateType CloseStampTaxByMoney;		//平仓印花税率
	RateType OpenTransferFeeByMoney;		//开仓过户费率
	RateType CloseTransferFeeByMoney;		//平仓过户费率
};
class RspUpdateBaseCommissionField
{
public:
	static constexpr UInt16Type FieldId = 0x4021;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqRemoveBaseCommissionField
{
public:
	static constexpr UInt16Type FieldId = 0x4022;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class RspRemoveBaseCommissionField
{
public:
	static constexpr UInt16Type FieldId = 0x4023;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqAddCommissionGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4024;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIdType CommissionGroupId;		//交易组代码
	GroupNameType CommissionGroupName;		//手续费组名称
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
	RateType OpenByMoney;		//开仓费率
	RateType CloseByMoney;		//平仓费率
	RateType OpenByVolume;		//每手开仓费用
	RateType CloseByVolume;		//每手平仓费用
	MoneyType MinCommission;		//最低手续费
	MoneyType MaxCommission;		//最高手续费
};
class RspAddCommissionGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4025;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIdType CommissionGroupId;		//交易组代码
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqUpdateCommissionGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4026;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIdType CommissionGroupId;		//交易组代码
	GroupNameType CommissionGroupName;		//手续费组名称
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
	RateType OpenByMoney;		//开仓费率
	RateType CloseByMoney;		//平仓费率
	RateType OpenByVolume;		//每手开仓费用
	RateType CloseByVolume;		//每手平仓费用
	MoneyType MinCommission;		//最低手续费
	MoneyType MaxCommission;		//最高手续费
};
class RspUpdateCommissionGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4027;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIdType CommissionGroupId;		//交易组代码
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqRemoveCommissionGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4028;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIdType CommissionGroupId;		//交易组代码
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class RspRemoveCommissionGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4029;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	GroupIdType CommissionGroupId;		//交易组代码
	ExchangeIdType ExchangeId;		//交易所代码
	ProductClassType ProductClass;		//品种类型
};
class ReqAddOptionMarginParamField
{
public:
	static constexpr UInt16Type FieldId = 0x402A;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIdType ExchangeId;		//交易所代码
	RateType Param1;		//参数1
	RateType Param2;		//参数2
};
class RspAddOptionMarginParamField
{
public:
	static constexpr UInt16Type FieldId = 0x402B;
	UserIdType AdminUserId;		//管理用户代码
	ExchangeIdType ExchangeId;		//交易所代码
};
class ReqUpdateOptionMarginParamField
{
public:
	static constexpr UInt16Type FieldId = 0x402C;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	ExchangeIdType ExchangeId;		//交易所代码
	RateType Param1;		//参数1
	RateType Param2;		//参数2
};
class RspUpdateOptionMarginParamField
{
public:
	static constexpr UInt16Type FieldId = 0x402D;
	UserIdType AdminUserId;		//管理用户代码
	ExchangeIdType ExchangeId;		//交易所代码
};
class ReqRemoveOptionMarginParamField
{
public:
	static constexpr UInt16Type FieldId = 0x402E;
	UserIdType AdminUserId;		//管理用户代码
	ExchangeIdType ExchangeId;		//交易所代码
};
class RspRemoveOptionMarginParamField
{
public:
	static constexpr UInt16Type FieldId = 0x402F;
	UserIdType AdminUserId;		//管理用户代码
	ExchangeIdType ExchangeId;		//交易所代码
};
class ReqAddTradeOfferField
{
public:
	static constexpr UInt16Type FieldId = 0x4030;
	UserIdType AdminUserId;		//管理用户代码
	OfferIdType OfferId;		//报盘代码
	OfferNameType OfferName;		//报盘名称
	OfferTypeType OfferType;		//报盘类型
	PasswordType OfferPassword;		//报盘密码
};
class RspAddTradeOfferField
{
public:
	static constexpr UInt16Type FieldId = 0x4031;
	UserIdType AdminUserId;		//管理用户代码
	OfferIdType OfferId;		//报盘代码
};
class ReqUpdateTradeOfferField
{
public:
	static constexpr UInt16Type FieldId = 0x4032;
	UserIdType AdminUserId;		//管理用户代码
	OfferIdType OfferId;		//报盘代码
	OfferNameType OfferName;		//报盘名称
	OfferTypeType OfferType;		//报盘类型
	PasswordType OfferPassword;		//报盘密码
};
class RspUpdateTradeOfferField
{
public:
	static constexpr UInt16Type FieldId = 0x4033;
	UserIdType AdminUserId;		//管理用户代码
	OfferIdType OfferId;		//报盘代码
};
class ReqRemoveTradeOfferField
{
public:
	static constexpr UInt16Type FieldId = 0x4034;
	UserIdType AdminUserId;		//管理用户代码
	OfferIdType OfferId;		//报盘代码
};
class RspRemoveTradeOfferField
{
public:
	static constexpr UInt16Type FieldId = 0x4035;
	UserIdType AdminUserId;		//管理用户代码
	OfferIdType OfferId;		//报盘代码
};
class ReqAddTradeGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4036;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
	GroupNameType TradeGroupName;		//交易组名称
};
class RspAddTradeGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4037;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
};
class ReqUpdateTradeGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4038;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
	GroupNameType TradeGroupName;		//交易组名称
};
class RspUpdateTradeGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4039;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
};
class ReqRemoveTradeGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x403A;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
};
class RspRemoveTradeGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x403B;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
};
class ReqAddTradeGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x403C;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
	AccountClassType AccountClass;		//账户类别
	AccountIdType PrimaryAccountId;		//主账户代码
};
class RspAddTradeGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x403D;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
	AccountClassType AccountClass;		//账户类别
};
class ReqUpdateTradeGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x403E;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
	AccountClassType AccountClass;		//账户类别
	AccountIdType PrimaryAccountId;		//主账户代码
};
class RspUpdateTradeGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x403F;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
	AccountClassType AccountClass;		//账户类别
};
class ReqRemoveTradeGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x4040;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
	AccountClassType AccountClass;		//账户类别
};
class RspRemoveTradeGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x4041;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType TradeGroupId;		//交易组代码
	AccountClassType AccountClass;		//账户类别
};
class ReqAddRiskGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4042;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
	GroupNameType RiskGroupName;		//风控组名称
};
class RspAddRiskGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4043;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
};
class ReqUpdateRiskGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4044;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
	GroupNameType RiskGroupName;		//风控组名称
};
class RspUpdateRiskGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4045;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
};
class ReqRemoveRiskGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4046;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
};
class RspRemoveRiskGroupField
{
public:
	static constexpr UInt16Type FieldId = 0x4047;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
};
class ReqAddRiskGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x4048;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
	RiskIdType RiskId;		//风控代码
};
class RspAddRiskGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x4049;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
	RiskIdType RiskId;		//风控代码
};
class ReqUpdateRiskGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x404A;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
	RiskIdType RiskId;		//风控代码
};
class RspUpdateRiskGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x404B;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
	RiskIdType RiskId;		//风控代码
};
class ReqRemoveRiskGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x404C;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
	RiskIdType RiskId;		//风控代码
};
class RspRemoveRiskGroupItemField
{
public:
	static constexpr UInt16Type FieldId = 0x404D;
	UserIdType AdminUserId;		//管理用户代码
	GroupIdType RiskGroupId;		//交易组代码
	RiskIdType RiskId;		//风控代码
};
class ReqAddOrUpdateRiskField
{
public:
	static constexpr UInt16Type FieldId = 0x404E;
	UserIdType AdminUserId;		//管理用户代码
	RiskIdType RiskId;		//风控代码
	RiskNameType RiskName;		//风控名称
	RiskClassIdType RiskClassId;		//风控类别代码
};
class RspAddOrUpdateRiskField
{
public:
	static constexpr UInt16Type FieldId = 0x404F;
	UserIdType AdminUserId;		//管理用户代码
	RiskIdType RiskId;		//风控代码
};
class ReqRemoveRiskField
{
public:
	static constexpr UInt16Type FieldId = 0x4050;
	UserIdType AdminUserId;		//管理用户代码
	RiskIdType RiskId;		//风控代码
};
class RspRemoveRiskField
{
public:
	static constexpr UInt16Type FieldId = 0x4051;
	UserIdType AdminUserId;		//管理用户代码
	RiskIdType RiskId;		//风控代码
};
class ReqAddRiskRuleField
{
public:
	static constexpr UInt16Type FieldId = 0x4052;
	UserIdType AdminUserId;		//管理用户代码
	RiskIdType RiskId;		//风控代码
	RiskRuleIdType RiskRuleId;		//风控规则代码
	RiskRuleStyleType RiskRuleStyle;		//风控类型
	MessageType FormatRiskMessage;		//带格式的风控信息
};
class RspAddRiskRuleField
{
public:
	static constexpr UInt16Type FieldId = 0x4053;
	UserIdType AdminUserId;		//管理用户代码
	RiskIdType RiskId;		//风控代码
	RiskRuleIdType RiskRuleId;		//风控规则代码
};
class ReqAddRiskRuleItemField
{
public:
	static constexpr UInt16Type FieldId = 0x4054;
	UserIdType AdminUserId;		//管理用户代码
	RiskIdType RiskId;		//风控代码
	RiskRuleIdType RiskRuleId;		//风控规则代码
	RiskRuleItemIdType RiskRuleItemId;		//风控规则分项编号
	RiskIndexTypeType RiskIndexType;		//风控指标类型
	RiskIndexIdType RiskIndexId;		//风控指标代码
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
	static constexpr UInt16Type FieldId = 0x4055;
	UserIdType AdminUserId;		//管理用户代码
	RiskIdType RiskId;		//风控代码
	RiskRuleIdType RiskRuleId;		//风控规则代码
	RiskRuleItemIdType RiskRuleItemId;		//风控规则分项编号
};
class ReqAddRiskTradeScopeField
{
public:
	static constexpr UInt16Type FieldId = 0x4056;
	UserIdType AdminUserId;		//管理用户代码
	RiskIdType RiskId;		//风控代码
	TradeScopeTypeType TradeScopeType;		//交易范围类别
	GroupIdType InstrumentGroupId;		//合约组代码
	MessageType FormatRiskMessage;		//带格式的风控信息
};
class RspAddRiskTradeScopeField
{
public:
	static constexpr UInt16Type FieldId = 0x4057;
	UserIdType AdminUserId;		//管理用户代码
	RiskIdType RiskId;		//风控代码
};
class ReqAddAccountRiskField
{
public:
	static constexpr UInt16Type FieldId = 0x4058;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType AccountId;		//账户代码
	RiskIdType RiskId;		//风控代码
};
class RspAddAccountRiskField
{
public:
	static constexpr UInt16Type FieldId = 0x4059;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType AccountId;		//账户代码
	RiskIdType RiskId;		//风控代码
};
class ReqRemoveAccountRiskField
{
public:
	static constexpr UInt16Type FieldId = 0x405A;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType AccountId;		//账户代码
	RiskIdType RiskId;		//风控代码
};
class RspRemoveAccountRiskField
{
public:
	static constexpr UInt16Type FieldId = 0x405B;
	UserIdType AdminUserId;		//管理用户代码
	AccountIdType AccountId;		//账户代码
	RiskIdType RiskId;		//风控代码
};
class ReqMoneyTransferField
{
public:
	static constexpr UInt16Type FieldId = 0x405C;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIdType AccountId;		//账户代码
	SequenceNoType MoneyTransferId;		//出入金编号
	AccountTypeType AccountType;		//账户类型
	TransferDirectionType TransferDirection;		//转移方向
	MoneyType TransferAmount;		//转移金额
	MessageType InfoMessage;		//备注信息
	UserIdType UserId;		//用户代码
	DateType TransferDate;		//操作日期
	TimeType TransferTime;		//操作时间
};
class RspMoneyTransferField
{
public:
	static constexpr UInt16Type FieldId = 0x405D;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIdType AccountId;		//账户代码
	SequenceNoType MoneyTransferId;		//出入金编号
};
class ReqAuditOrderField
{
public:
	static constexpr UInt16Type FieldId = 0x405E;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIdType AccountId;		//账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	OrderIdType OrderId;		//委托编号
	AuditStatusType AuditStatus;		//审核状态
};
class RspAuditOrderField
{
public:
	static constexpr UInt16Type FieldId = 0x405F;
	UserIdType AdminUserId;		//管理用户代码
	DateType TradingDay;		//交易日
	AccountIdType AccountId;		//账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	OrderIdType OrderId;		//委托编号
	AuditStatusType AuditStatus;		//审核状态
};
class ReqOfferLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x5001;
	OfferIdType OfferId;		//报盘代码
	PasswordType OfferPassword;		//报盘密码
};
class RspOfferLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x5002;
	ErrorIdType ErrorId;		//错误代码
	MessageType ErrorMsg;		//错误信息
	OfferIdType OfferId;		//报盘代码
	DateType TradingDay;		//交易日
};
class ReqPrimaryAccountLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x5003;
	AccountIdType PrimaryAccountId;		//主账户代码
	PasswordType Password;		//密码
};
class RspPrimaryAccountLoginField
{
public:
	static constexpr UInt16Type FieldId = 0x5004;
	ErrorIdType ErrorId;		//错误代码
	MessageType ErrorMsg;		//错误信息
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
};
class ReqPrimaryAccountLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x5005;
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
};
class RtnPrimaryAccountLogoutField
{
public:
	static constexpr UInt16Type FieldId = 0x5006;
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
};
class ReqPrimaryAccountInitField
{
public:
	static constexpr UInt16Type FieldId = 0x5007;
	AccountIdType PrimaryAccountId;		//主账户代码
};
class RspPrimaryAccountInitField
{
public:
	static constexpr UInt16Type FieldId = 0x5008;
	ErrorIdType ErrorId;		//错误代码
	MessageType ErrorMsg;		//错误信息
	AccountIdType PrimaryAccountId;		//主账户代码
};
class ReqPrimaryAccountQueryField
{
public:
	static constexpr UInt16Type FieldId = 0x5009;
	AccountIdType PrimaryAccountId;		//主账户代码
};
class RspPrimaryAccountQueryField
{
public:
	static constexpr UInt16Type FieldId = 0x500A;
	ErrorIdType ErrorId;		//错误代码
	MessageType ErrorMsg;		//错误信息
	AccountIdType PrimaryAccountId;		//主账户代码
};
class ReqQryOfferOptionInstrumentField
{
public:
	static constexpr UInt16Type FieldId = 0x500B;
	AccountIdType PrimaryAccountId;		//主账户代码
};
class RspQryOfferOptionInstrumentField
{
public:
	static constexpr UInt16Type FieldId = 0x500C;
	ErrorIdType ErrorId;		//错误代码
	MessageType ErrorMsg;		//错误信息
	AccountIdType PrimaryAccountId;		//主账户代码
};
class OfferOptionInstrumentField
{
public:
	static constexpr UInt16Type FieldId = 0x500D;
	DateType TradingDay;		//交易日
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	InstrumentIdType ExchangeInstId;		//交易所合约代码
	InstrumentNameType InstrumentName;		//合约名称
	VolumeMultipleType VolumeMultiple;		//合约乘数
	OptionTypeType OptionType;		//期权类型
	InstrumentIdType UnderlyingInstrumentId;		//标的合约代码
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
	static constexpr UInt16Type FieldId = 0x500E;
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	ProductClassType ProductClass;		//品种类型
	OrderIdType OrderId;		//委托编号
	DirectionType Direction;		//买卖方向
	OffsetFlagType OffsetFlag;		//开平标志
	OrderPriceTypeType OrderPriceType;		//委托价格类型
	PriceType Price;		//委托价格
	VolumeType Volume;		//委托数量
};
class ReqOfferCancelOrderField
{
public:
	static constexpr UInt16Type FieldId = 0x500F;
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	ProductClassType ProductClass;		//品种类型
	DirectionType Direction;		//买卖方向
	OrderIdType CancelOrderId;		//本地撤单编号
	OrderIdType OrderId;		//委托编号
	OrderSysIdType OrderSysId;		//系统委托编号
};
class OfferOrderField
{
public:
	static constexpr UInt16Type FieldId = 0x5010;
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	OrderIdType OrderId;		//委托编号
	OrderSysIdType OrderSysId;		//系统委托编号
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
	static constexpr UInt16Type FieldId = 0x5011;
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	OrderIdType OrderId;		//委托编号
	OrderSysIdType OrderSysId;		//系统委托编号
	TradeIdType TradeId;		//成交编号
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
	static constexpr UInt16Type FieldId = 0x5012;
	ErrorIdType ErrorId;		//错误代码
	MessageType ErrorMsg;		//错误信息
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	DirectionType Direction;		//买卖方向
	OrderIdType CancelOrderId;		//本地撤单编号
	OrderIdType OrderId;		//委托编号
	OrderSysIdType OrderSysId;		//系统委托编号
};
class OfferCapitalField
{
public:
	static constexpr UInt16Type FieldId = 0x5013;
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
	MoneyType PreCashAsset;		//上日现金资产
};
class OfferPositionField
{
public:
	static constexpr UInt16Type FieldId = 0x5014;
	DateType TradingDay;		//交易日
	AccountIdType PrimaryAccountId;		//主账户代码
	ExchangeIdType ExchangeId;		//交易所代码
	InstrumentIdType InstrumentId;		//合约代码
	PosiDirectionType PosiDirection;		//持仓方向
	VolumeType TotalPosition;		//持仓数量
	VolumeType PositionFrozen;		//冻结持仓
	PriceType TotalCostPrice;		//总成本价
	MoneyType Margin;		//保证金
	MoneyType MarketValue;		//市值
};
