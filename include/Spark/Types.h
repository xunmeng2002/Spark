#pragma once
#include <stdint.h>

//布尔类型
typedef bool BoolType;


//短整型类型
typedef unsigned short UShortType;


//客户端委托编号类型
typedef int ClientOrderIDType;

//指令类型
typedef int CommandType;

//错误代码类型
typedef int ErrorIDType;

//前置代码类型
typedef int FrontIDType;

//组代码类型
typedef int GroupIDType;

//整型类型
typedef int IntType;

//最大委托编号类型
typedef int MaxOrderIDType;

//整型类型
typedef int NumberType;

//报盘代码类型
typedef int OfferIDType;

//委托编号类型
typedef int OrderIDType;

//请求代码类型
typedef int RequestIDType;

//风控代码类型
typedef int RiskIDType;

//风控指标代码类型
typedef int RiskIndexIDType;

//风控等级
typedef int RiskLevelType;

//风控规则代码类型
typedef int RiskRuleIDType;

//风控规则子项代码类型
typedef int RiskRuleItemIDType;

//风控文本引用序号
typedef int RiskTextRefType;

//角色代码类型
typedef int RoleIDType;

//时间秒数类型
typedef int SecondType;

//序号类型
typedef int SequenceNoType;

//合约乘数类型
typedef int VolumeMultipleType;


	//64位整型类型
typedef long long Int64Type;

	//会话代码类型
typedef long long SessionIDType;

	//数量类型
typedef long long VolumeType;


//双精度浮点类型
typedef double DoubleType;

//金额类型
typedef double MoneyType;

//价格类型
typedef double PriceType;

//比率
typedef double RateType;

//大额数量
typedef double LargeVolumeType;


//账号类型
typedef char AccountIDType[32];

//账号名称类型
typedef char AccountNameType[64];

//App代码类型
typedef char AppIDType[32];

//App版本类型
typedef char AppVersionType[32];

//授权码类型
typedef char AuthCodeType[64];

//银行卡号类型
typedef char BankCardIDType[32];

//主板时间类型
typedef char BiosTimeType[16];

//经纪公司代码
typedef char BrokerIDType[16];

//经纪公司名称
typedef char BrokerNameType[16];

//经纪公司委托编号类型
typedef char BrokerOrderIDType[32];

//币种类型
typedef char CurrencyIDType[8];

//币种组类型
typedef char CurrencyGroupType[8];

//CPU编号类型
typedef char CpuIDType[32];

//日期类型
typedef char DateType[16];

//数据库名称类型
typedef char DBNameType[32];

//磁盘序号类型
typedef char DiskIDType[64];

//磁盘路径类型
typedef char DiskPathType[32];

//交易所代码类型
typedef char ExchangeIDType[8];

//交易所名称类型
typedef char ExchangeNameType[64];

//交易所成交编号类型
typedef char ExchangeTradeIDType[32];

//组名称类型
typedef char GroupNameType[64];

//主机名类型
typedef char HostNameType[32];

//合约代码类型
typedef char InstrumentIDType[32];

//合约名称类型
typedef char InstrumentNameType[64];

//IP地址类型
typedef char IPAddressType[32];

//Mac地址类型
typedef char MacAddressType[32];

//错误信息类型
typedef char MessageType[256];

//名称类型
typedef char NameType[64];

//报盘名称类型
typedef char OfferNameType[32];

//机构代码类型
typedef char OrgIDType[16];

//系统委托编号类型
typedef char OrderSysIDType[64];

//密码类型
typedef char PasswordType[64];

//参数值类型
typedef char ParamValueType[256];

//手机号码类型
typedef char PhoneNumberType[32];

//品种代码类型
typedef char ProductIDType[32];

//用户产品信息类型
typedef char ProductInfoType[32];

//品种名称类型
typedef char ProductNameType[32];

//协议地址类型
typedef char ProtocolAddressType[32];

//预留字段类型
typedef char ReserveType[32];

//风控名称
typedef char RiskNameType[256];

//角色名称类型
typedef char RoleNameType[32];

//席位号
typedef char SeatNoType[24];

//交易节名称
typedef char SessionNameType[32];

//Tick买卖标志
typedef char TickBSFlagType[8];

//时间类型
typedef char TimeType[16];

//成交编号类型
typedef char TradeIDType[64];

//用户代码类型
typedef char UserIDType[32];

//用户名称类型
typedef char UserNameType[64];


//账户类别
enum class AccountClassType : int
{
	//期货
	Future = 0,
	//股票
	Stock = 1,
	//期权
	Option = 2,
};

//账户结算状态
enum class AccountSettlementStatusType : int
{
	//未开始
	NotStart = 0,
	//结算失败
	SettlementFailed = 1,
	//结算完成
	SettlementCompleted = 2,
};

//账户类型
enum class AccountTypeType : int
{
	//主账户
	Primary = 0,
	//子账户
	Sub = 1,
};

//账户状态
enum class AccountStatusType : int
{
	//启用
	Normal = 0,
	//禁用
	Forbidden = 1,
};

//应用平台类型
enum class AppPlatformType : int
{
	Windows = 0,
	Linux = 1,
	IOS = 2,
	Android = 3,
};

//审核状态
enum class AuditStatusType : int
{
	//通过
	Approve = 0,
	//驳回
	Reject = 1,
};

//Bar精度
enum class BarPrecesType : int
{
	//秒
	Second = 0,
	//分
	Minute = 1,
	//日
	Day = 2,
};

//组件类型
enum class ComponentType : int
{
	//交易前置
	TradeFront = 0,
	//风控前置
	RiskFront = 1,
	//管理前置
	AdminFront = 2,
	//报盘前置
	OfferFront = 3,
	//行情
	MarketData = 4,
};

//触发条件类型
enum class ContingentConditionType : int
{
	//立即
	Immediately = 0,
	//止损
	Touch = 1,
	//止赢
	TouchProfit = 2,
	//预埋单
	ParkedOrder = 3,
};

//连接状态
enum class ConnectStatusType : int
{
	//未连接
	UnConnected = 0,
	//正在链接
	Connecting = 1,
	//已接受
	Accepted = 2,
	//已拒绝
	Rejected = 3,
	//已连接
	Connected = 4,
	//已断开连接
	DisConnected = 5,
};

//数据库操作
enum class DBOperateType : int
{
	//创建所有表
	CreateTables = 0,
	//删除所有表
	DropTables = 1,
	//清空所有表
	TruncateTables = 2,
	//插入
	Insert = 3,
	//删除
	Delete = 4,
	//按索引删除
	DeleteByIndex = 5,
	//更新
	Update = 6,
	//批量插入
	BatchInsert = 7,
	//清空
	Truncate = 8,
};

//数据库类型
enum class DBTypeType : int
{
	//duckdb
	DuckDB = 0,
	//sqlite
	SqliteDB = 1,
	//mysql
	MysqlDB = 2,
	//maria
	MariaDB = 3,
};

//交割方式
enum class DeliveryMethodType : int
{
	//现金交割
	Cash = 0,
	//实物交割
	Commodity = 1,
};

//买卖方向
enum class DirectionType : int
{
	//买
	Buy = 0,
	//卖
	Sell = 1,
};

//交易所状态
enum class ExchangeStatusType : int
{
	//开盘前
	BeforeTrading = 0,
	//非交易
	NoTrading = 1,
	//连续交易
	Continous = 2,
	//集合竞价报单
	AuctionOrdering = 3,
	//集合竞价平衡
	AuctionBalance = 4,
	//集合竞价撮合
	AuctionMatch = 5,
	//收盘
	Closed = 6,
	//初始化
	Initializing = 7,
};

//强平原因
enum class ForceCloseReasonType : int
{
	//非强平
	NotForceClose = 0,
	//资金不足
	LackDeposit = 1,
	//客户超仓
	ClientOverPositionLimit = 2,
	//会员超仓
	MemberOverPositionLimit = 3,
	//持仓非整数倍
	NotMultiple = 4,
	//违规
	Violation = 5,
	//其它
	Other = 6,
	//自然人临近交割
	PersonDeliv = 7,
};

//投保标志
enum class HedgeFlagType : int
{
	//投机
	Speculation = 0,
	//套利
	Arbitrage = 1,
	//保值
	Hedge = 2,
};

//行情初始化状态
enum class InitStatusType : int
{
	//未初始化
	UnInitialized = 0,
	//正在初始化
	Initializing = 1,
	//已初始化
	Initialized = 2,
};

//合约类别
enum class InstrumentClassType : int
{
	//普通合约
	Normal = 0,
	//主力合约
	Main = 1,
};

//IO模型
enum class IOModelType : int
{
	//Select
	Select = 0,
	//Epoll
	Epoll = 1,
	//Iocp
	Iocp = 2,
};

//IO类型
enum class IOTypeType : int
{
	//Tcp
	Tcp = 0,
	//Udp
	Udp = 1,
	//共享内存
	Shm = 2,
};

//逻辑运算函数
enum class LogicFuncType : int
{
	//等于
	Equal = 0,
	//不等于
	NotEqual = 1,
	//小于
	LessThan = 2,
	//小于等于
	LessEqual = 3,
	//大于
	GreatThan = 4,
	//大于等于
	GreatEqual = 5,
	//在区间范围内
	In = 6,
	//在区间范围内包含等于
	EqualIn = 7,
	//在区间范围外
	Out = 8,
	//在区间范围外包含等于
	EqualOut = 9,
	//在百分比范围内
	PctIn = 10,
	//在百分比范围内包含等于
	PctEqualIn = 11,
	//在百分比范围外
	PctOut = 12,
	//在百分比范围外包含等于
	PctEqualOut = 13,
};

//登录状态
enum class LoginStatusType : int
{
	//未登录
	NotLog = 0,
	//登录中
	Logging = 1,
	//已登录
	Logged = 2,
};

//保证金价格类型
enum class MarginPriceTypeType : int
{
	//昨结算价
	PreSettlementPrice = 0,
	//最新价
	SettlementPrice = 1,
	//成交均价
	AveragePrice = 2,
	//开仓价
	OpenPrice = 3,
};

//保证金类型
enum class MarginTypeType : int
{
	//比例
	Rate = 0,
	//固定金额
	Fixed = 1,
};

//行情数据类型
enum class MarketDataTypeType : int
{
	//快照行情
	Tick = 0,
	//Bar行情
	Bar = 1,
};

//撮合模式
enum class MatchModeType : int
{
	//订单簿
	OrderBook = 0,
	//最新价
	LastPrice = 1,
	//对手价
	OppositePrice = 2,
	//Bar撮合
	Bar = 3,
};

//持仓方向
enum class PosiDirectionType : int
{
	//多头
	Long = 0,
	//空头
	Short = 1,
	//净持仓
	Net = 2,
};

//报盘类型
enum class OfferTypeType : int
{
	//模拟
	SE = 0,
	//汇点标准中间件
	Stds = 1,
};

//开平标志
enum class OffsetFlagType : int
{
	//开仓
	Open = 0,
	//平仓
	Close = 1,
	//平今
	CloseToday = 2,
};

//期权指标类型
enum class OptionIndicatorTypeType : int
{
	//Delta
	Delta = 0,
	//Gamma
	Gamma = 1,
	//Rho
	Rho = 2,
	//Theta
	Theta = 3,
	//Vega
	Vega = 4,
};

//期权类型
enum class OptionTypeType : int
{
	//认购期权
	Call = 0,
	//认沽期权
	Put = 1,
};

//委托价格类型
enum class OrderPriceTypeType : int
{
	//限价
	LimitPrice = 0,
	//市价立即成交剩余撤销
	AnyPriceFAK = 1,
	//市价立即成全部交否则撤销
	AnyPriceFOK = 2,
	//限价立即成交剩余撤销
	LimitPriceFAK = 3,
	//限价立即成全部交否则撤销
	LimitPriceFOK = 4,
	//本方最优价
	BestOwnPrice = 5,
	//对方最优价
	BestOppoPrice = 6,
};

//委托状态
enum class OrderStatusType : int
{
	//正在申报
	Inserting = 0,
	//已报
	Inserted = 1,
	//部成
	PartTraded = 2,
	//已成
	AllTraded = 3,
	//已撤
	Canceled = 4,
	//部成部撤
	PartTradedCanceled = 5,
	//审核中
	Auditing = 6,
	//审核驳回
	AuditReject = 7,
	//风控检查中
	RiskChecking = 8,
	//风控检查失败
	RiskCheckFailed = 9,
	//废单
	Error = 10,
	//未触发
	NotTouched = 11,
	//已触发
	Touched = 12,
	//未知
	Unknown = 13,
};

//委托类型
enum class OrderTypeType : int
{
	//普通单
	Normal = 0,
	//互换单
	Swap = 1,
	//组合委托
	Combination = 2,
};

//参数类型
enum class ParamTypeType : int
{
	//常量
	Const = 0,
	//指标
	Index = 1,
};

//产品类型
enum class ProductClassType : int
{
	//期货
	Future = 0,
	//期货期权
	FutureOption = 1,
	//组合
	Combination = 2,
	//即期
	Spot = 3,
	//期转现
	EFP = 4,
	//指数
	Index = 5,
	//股票
	Stock = 6,
	//股票期权
	StockOption = 7,
	//ETF
	ETF = 8,
};

//协议类型
enum class ProtocolTypeType : int
{
	//Xtp协议
	Xtp = 0,
	//Step协议
	Step = 1,
};

//查询状态
enum class QueryStatusType : int
{
	//未查询
	UnQueried = 0,
	//正在查询
	Quering = 1,
	//已查询
	Queried = 2,
};

//风控动作
enum class RiskActionType : int
{
	//警告
	Warn = 0,
	//禁止开仓
	ForbiddenOpen = 1,
	//禁止买入开仓
	ForbiddenBuyOpen = 2,
	//禁止卖出开仓
	ForbiddenSellOpen = 3,
};

//风控类别代码
enum class RiskClassIDType : int
{
	//交易范围控制
	TradeScope = 0,
	//量价控制
	VolumePrice = 1,
	//仓位控制
	Position = 2,
	//净值线
	NetValueLine = 3,
	//集中度控制
	Concentration = 4,
	//盈亏控制
	Profit = 5,
	//回撤控制
	Retracement = 6,
	//风险度控制
	RiskLevel = 7,
	//期权指标控制
	OptionIndicator = 8,
};

//风控指标类型
enum class RiskIndexTypeType : int
{
	//整型
	Int = 0,
	//浮点型
	Double = 1,
	//字符串
	String = 2,
};

//风险度类型
enum class RiskLevelTypeType : int
{
	//保证金
	Margin = 0,
	//市值
	MarketValue = 1,
};

//风控类型
enum class RiskRuleStyleType : int
{
	//事后风控
	Afterward = 0,
	//事前风控
	Beforehand = 1,
};

//风控状态
enum class RiskStatusType : int
{
	//正常
	Normal = 0,
	//已触发
	Triggered = 1,
};

//证券细分类别
enum class SecurityDetailTypeType : int
{
	//指数
	Index = 0,
	//主板
	MainBoard = 1,
	//科创板
	Star = 2,
	//中小板
	SMEBoard = 3,
	//创业板
	SecondBoard = 4,
	//B股
	BShares = 5,
	//ETF
	ETF = 6,
	//配股
	AllotmentCode = 7,
	//债券
	Bond = 8,
	//可转债
	ConvertibleBond = 9,
	//逆回购
	ReverseRepo = 10,
	//期权
	Option = 11,
	//其他
	Other = 12,
	//优先股
	PreferredStock = 13,
	//期货
	Future = 14,
	//期货期权
	FutureOption = 15,
};

//交易节类别
enum class SectionClassType : int
{
	//集合竞价
	Auction = 0,
	//连续竞价
	Section = 1,
};

//服务类别
enum class ServerTypeType : int
{
	//客户端
	Client = 0,
	//服务端
	Server = 1,
};

//系统运行状态
enum class SystemRunningStatusType : int
{
	//未运行
	NotRun = 0,
	//已初始化
	Initialized = 1,
	//运行中
	IsRunning = 2,
};

//系统结算状态
enum class SystemSettlementStatusType : int
{
	//未开始
	NotStart = 0,
	//结算数据已准备
	SettleDataPrepared = 1,
	//结算中
	Settling = 2,
	//结算失败
	SettlementFailed = 3,
	//结算完成
	SettlementCompleted = 4,
};

//证券委托价格类型
enum class StockOrderPriceTypeType : int
{
	//限价
	LimitPrice = 0,
	//最优五档即时成交剩余撤销
	FAK5 = 1,
	//最优五档即时成交剩余转限价
	FAL5 = 2,
	//即时成交剩余撤销
	FAK = 3,
	//全额成交或撤销
	FOK = 4,
	//对方最优价
	DBestPrice = 5,
	//本方最优价
	WBestPrice = 6,
};

//Tick类型
enum class TickTypeType : int
{
	//新增委托
	Add = 0,
	//删除委托
	Delete = 1,
	//产品状态
	Status = 2,
	//成交
	Trade = 3,
};

//有效期类型
enum class TimeConditionType : int
{
	//立即完成，否则撤销
	IOC = 0,
	//本节有效
	GFS = 1,
	//当日有效
	GFD = 2,
	//指定日期前有效
	GTD = 3,
	//撤销前有效
	GTC = 4,
	//集合竞价有效
	GFA = 5,
};

//交易范围类别
enum class TradeScopeTypeType : int
{
	//黑名单
	BlackList = 0,
	//白名单
	WhiteList = 1,
};

//交易状态
enum class TradeStatusType : int
{
	//忽略
	Ignore = 0,
	//正常
	Normal = 1,
	//只可平仓
	CloseOnly = 2,
	//禁止交易
	BanTrading = 3,
};

//成交类型
enum class TradeTypeType : int
{
	//普通成交
	Common = 0,
	//期权执行
	OptionsExecution = 1,
	//OTC成交
	OTC = 2,
	//期转现衍生成交
	EFPDerived = 3,
	//组合衍生成交
	CombinationDerived = 4,
};

//出入金方向
enum class TransferDirectionType : int
{
	//入金
	TransferIn = 0,
	//出金
	TransferOut = 1,
};

//出入金标记
enum class TransferFlagType : int
{
	//正常
	Normal = 0,
	//手动录入
	Import = 1,
};

//出入金方式
enum class TransferMethodType : int
{
	//转账
	Transfer = 0,
	//支票
	Cheque = 1,
	//现金
	Cash = 2,
	//银期转账
	BFTransfer = 3,
};

//成交量类型
enum class VolumeConditionType : int
{
	//任何数量
	AV = 0,
	//最小数量
	MV = 1,
	//全部数量
	CV = 2,
};


