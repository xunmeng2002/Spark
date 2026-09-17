// 本文件由 ../Templates/Cpp/Protocol/Packages/Packages.h.tpl 生成；请勿手改，改动请改模板后重跑 pumpall.py
#pragma once
#include <Spark/Fields.h>
#include <Spark/Network/Protocol/Package.h>

using Spark::Network::Package;

namespace Spark::Packages
{
class NotifyComponentConnectStatusPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x0001;
	NotifyComponentConnectStatusPackage();
	~NotifyComponentConnectStatusPackage();
	static NotifyComponentConnectStatusPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	NotifyComponentConnectStatusField* NotifyComponentConnectStatus = nullptr;
};
class ReqAccountLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1001;
	ReqAccountLoginPackage();
	~ReqAccountLoginPackage();
	static ReqAccountLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAccountLoginField* ReqAccountLogin = nullptr;
};
class RspAccountLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1002;
	RspAccountLoginPackage();
	~RspAccountLoginPackage();
	static RspAccountLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAccountLoginField* RspAccountLogin = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAccountLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1003;
	ReqAccountLogoutPackage();
	~ReqAccountLogoutPackage();
	static ReqAccountLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAccountLogoutField* ReqAccountLogout = nullptr;
};
class RspAccountLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1004;
	RspAccountLogoutPackage();
	~RspAccountLogoutPackage();
	static RspAccountLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAccountLogoutField* RspAccountLogout = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1005;
	ReqQryAccountPackage();
	~ReqQryAccountPackage();
	static ReqQryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryAccountField* ReqQryAccount = nullptr;
};
class RspQryAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1006;
	RspQryAccountPackage();
	~RspQryAccountPackage();
	static RspQryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	AccountField* Account = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryHolderAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1007;
	ReqQryHolderAccountPackage();
	~ReqQryHolderAccountPackage();
	static ReqQryHolderAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryHolderAccountField* ReqQryHolderAccount = nullptr;
};
class RspQryHolderAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1008;
	RspQryHolderAccountPackage();
	~RspQryHolderAccountPackage();
	static RspQryHolderAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	HolderAccountField* HolderAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryCapitalPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1009;
	ReqQryCapitalPackage();
	~ReqQryCapitalPackage();
	static ReqQryCapitalPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryCapitalField* ReqQryCapital = nullptr;
};
class RspQryCapitalPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x100A;
	RspQryCapitalPackage();
	~RspQryCapitalPackage();
	static RspQryCapitalPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	CapitalField* Capital = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryPositionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x100B;
	ReqQryPositionPackage();
	~ReqQryPositionPackage();
	static ReqQryPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryPositionField* ReqQryPosition = nullptr;
};
class RspQryPositionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x100C;
	RspQryPositionPackage();
	~RspQryPositionPackage();
	static RspQryPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	PositionField* Position = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x100D;
	ReqQryOrderPackage();
	~ReqQryOrderPackage();
	static ReqQryOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryOrderField* ReqQryOrder = nullptr;
};
class RspQryOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x100E;
	RspQryOrderPackage();
	~RspQryOrderPackage();
	static RspQryOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OrderField* Order = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryTradePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x100F;
	ReqQryTradePackage();
	~ReqQryTradePackage();
	static ReqQryTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryTradeField* ReqQryTrade = nullptr;
};
class RspQryTradePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1010;
	RspQryTradePackage();
	~RspQryTradePackage();
	static RspQryTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	TradeField* Trade = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryInstrumentPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1011;
	ReqQryInstrumentPackage();
	~ReqQryInstrumentPackage();
	static ReqQryInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryInstrumentField* ReqQryInstrument = nullptr;
};
class RspQryInstrumentPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1012;
	RspQryInstrumentPackage();
	~RspQryInstrumentPackage();
	static RspQryInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	InstrumentField* Instrument = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryOptionInstrumentPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1013;
	ReqQryOptionInstrumentPackage();
	~ReqQryOptionInstrumentPackage();
	static ReqQryOptionInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryOptionInstrumentField* ReqQryOptionInstrument = nullptr;
};
class RspQryOptionInstrumentPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1014;
	RspQryOptionInstrumentPackage();
	~RspQryOptionInstrumentPackage();
	static RspQryOptionInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OptionInstrumentField* OptionInstrument = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryCommissionRatePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1015;
	ReqQryCommissionRatePackage();
	~ReqQryCommissionRatePackage();
	static ReqQryCommissionRatePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryCommissionRateField* ReqQryCommissionRate = nullptr;
};
class RspQryCommissionRatePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1016;
	RspQryCommissionRatePackage();
	~RspQryCommissionRatePackage();
	static RspQryCommissionRatePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	CommissionRateField* CommissionRate = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryMoneyTransferPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1017;
	ReqQryMoneyTransferPackage();
	~ReqQryMoneyTransferPackage();
	static ReqQryMoneyTransferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryMoneyTransferField* ReqQryMoneyTransfer = nullptr;
};
class RspQryMoneyTransferPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1018;
	RspQryMoneyTransferPackage();
	~RspQryMoneyTransferPackage();
	static RspQryMoneyTransferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	MoneyTransferField* MoneyTransfer = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqInsertOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1019;
	ReqInsertOrderPackage();
	~ReqInsertOrderPackage();
	static ReqInsertOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqInsertOrderField* ReqInsertOrder = nullptr;
};
class RspInsertOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x101A;
	RspInsertOrderPackage();
	~RspInsertOrderPackage();
	static RspInsertOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OrderField* Order = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqCancelOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x101B;
	ReqCancelOrderPackage();
	~ReqCancelOrderPackage();
	static ReqCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqCancelOrderField* ReqCancelOrder = nullptr;
};
class RspCancelOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x101C;
	RspCancelOrderPackage();
	~RspCancelOrderPackage();
	static RspCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	CancelOrderField* CancelOrder = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class RtnOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x101D;
	RtnOrderPackage();
	~RtnOrderPackage();
	static RtnOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OrderField* Order = nullptr;
};
class RtnTradePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x101E;
	RtnTradePackage();
	~RtnTradePackage();
	static RtnTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	TradeField* Trade = nullptr;
};
class RtnMoneyTransferPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x101F;
	RtnMoneyTransferPackage();
	~RtnMoneyTransferPackage();
	static RtnMoneyTransferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	MoneyTransferField* MoneyTransfer = nullptr;
};
class RtnAccountLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x1020;
	RtnAccountLogoutPackage();
	~RtnAccountLogoutPackage();
	static RtnAccountLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	AccountLogoutField* AccountLogout = nullptr;
};
class ReqRiskUserLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2001;
	ReqRiskUserLoginPackage();
	~ReqRiskUserLoginPackage();
	static ReqRiskUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRiskUserLoginField* ReqRiskUserLogin = nullptr;
};
class RspRiskUserLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2002;
	RspRiskUserLoginPackage();
	~RspRiskUserLoginPackage();
	static RspRiskUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRiskUserLoginField* RspRiskUserLogin = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRiskUserLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2003;
	ReqRiskUserLogoutPackage();
	~ReqRiskUserLogoutPackage();
	static ReqRiskUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRiskUserLogoutField* ReqRiskUserLogout = nullptr;
};
class RspRiskUserLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2004;
	RspRiskUserLogoutPackage();
	~RspRiskUserLogoutPackage();
	static RspRiskUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRiskUserLogoutField* RspRiskUserLogout = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class RtnRiskUserLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2005;
	RtnRiskUserLogoutPackage();
	~RtnRiskUserLogoutPackage();
	static RtnRiskUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RiskUserLogoutField* RiskUserLogout = nullptr;
};
class RtnAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2006;
	RtnAccountPackage();
	~RtnAccountPackage();
	static RtnAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	AccountField* Account = nullptr;
};
class RtnAccountDeletePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2007;
	RtnAccountDeletePackage();
	~RtnAccountDeletePackage();
	static RtnAccountDeletePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	AccountDeleteField* AccountDelete = nullptr;
};
class RtnPositionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2008;
	RtnPositionPackage();
	~RtnPositionPackage();
	static RtnPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	PositionField* Position = nullptr;
};
class RtnAccountRiskPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2009;
	RtnAccountRiskPackage();
	~RtnAccountRiskPackage();
	static RtnAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	AccountRiskField* AccountRisk = nullptr;
};
class RtnAccountRiskDeletePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x200A;
	RtnAccountRiskDeletePackage();
	~RtnAccountRiskDeletePackage();
	static RtnAccountRiskDeletePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	AccountRiskDeleteField* AccountRiskDelete = nullptr;
};
class RtnAccountRiskNotifyPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x200B;
	RtnAccountRiskNotifyPackage();
	~RtnAccountRiskNotifyPackage();
	static RtnAccountRiskNotifyPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	AccountRiskNotifyField* AccountRiskNotify = nullptr;
};
class ReqQryRiskGroupAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x200C;
	ReqQryRiskGroupAccountPackage();
	~ReqQryRiskGroupAccountPackage();
	static ReqQryRiskGroupAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryRiskGroupAccountField* ReqQryRiskGroupAccount = nullptr;
};
class RspQryRiskGroupAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x200D;
	RspQryRiskGroupAccountPackage();
	~RspQryRiskGroupAccountPackage();
	static RspQryRiskGroupAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	AccountField* Account = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupCapitalPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x200E;
	ReqQryRiskGroupCapitalPackage();
	~ReqQryRiskGroupCapitalPackage();
	static ReqQryRiskGroupCapitalPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryRiskGroupCapitalField* ReqQryRiskGroupCapital = nullptr;
};
class RspQryRiskGroupCapitalPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x200F;
	RspQryRiskGroupCapitalPackage();
	~RspQryRiskGroupCapitalPackage();
	static RspQryRiskGroupCapitalPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	CapitalField* Capital = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupPositionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2010;
	ReqQryRiskGroupPositionPackage();
	~ReqQryRiskGroupPositionPackage();
	static ReqQryRiskGroupPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryRiskGroupPositionField* ReqQryRiskGroupPosition = nullptr;
};
class RspQryRiskGroupPositionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2011;
	RspQryRiskGroupPositionPackage();
	~RspQryRiskGroupPositionPackage();
	static RspQryRiskGroupPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	PositionField* Position = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2012;
	ReqQryRiskGroupOrderPackage();
	~ReqQryRiskGroupOrderPackage();
	static ReqQryRiskGroupOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryRiskGroupOrderField* ReqQryRiskGroupOrder = nullptr;
};
class RspQryRiskGroupOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2013;
	RspQryRiskGroupOrderPackage();
	~RspQryRiskGroupOrderPackage();
	static RspQryRiskGroupOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OrderField* Order = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupTradePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2014;
	ReqQryRiskGroupTradePackage();
	~ReqQryRiskGroupTradePackage();
	static ReqQryRiskGroupTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryRiskGroupTradeField* ReqQryRiskGroupTrade = nullptr;
};
class RspQryRiskGroupTradePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2015;
	RspQryRiskGroupTradePackage();
	~RspQryRiskGroupTradePackage();
	static RspQryRiskGroupTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	TradeField* Trade = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupAccountRiskPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2016;
	ReqQryRiskGroupAccountRiskPackage();
	~ReqQryRiskGroupAccountRiskPackage();
	static ReqQryRiskGroupAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryRiskGroupAccountRiskField* ReqQryRiskGroupAccountRisk = nullptr;
};
class RspQryRiskGroupAccountRiskPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2017;
	RspQryRiskGroupAccountRiskPackage();
	~RspQryRiskGroupAccountRiskPackage();
	static RspQryRiskGroupAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	AccountRiskField* AccountRisk = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupAccountRiskNotifyPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2018;
	ReqQryRiskGroupAccountRiskNotifyPackage();
	~ReqQryRiskGroupAccountRiskNotifyPackage();
	static ReqQryRiskGroupAccountRiskNotifyPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryRiskGroupAccountRiskNotifyField* ReqQryRiskGroupAccountRiskNotify = nullptr;
};
class RspQryRiskGroupAccountRiskNotifyPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x2019;
	RspQryRiskGroupAccountRiskNotifyPackage();
	~RspQryRiskGroupAccountRiskNotifyPackage();
	static RspQryRiskGroupAccountRiskNotifyPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	AccountRiskNotifyField* AccountRiskNotify = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRiskInsertOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x201A;
	ReqRiskInsertOrderPackage();
	~ReqRiskInsertOrderPackage();
	static ReqRiskInsertOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRiskInsertOrderField* ReqRiskInsertOrder = nullptr;
};
class RspRiskInsertOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x201B;
	RspRiskInsertOrderPackage();
	~RspRiskInsertOrderPackage();
	static RspRiskInsertOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OrderField* Order = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRiskCancelOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x201C;
	ReqRiskCancelOrderPackage();
	~ReqRiskCancelOrderPackage();
	static ReqRiskCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRiskCancelOrderField* ReqRiskCancelOrder = nullptr;
};
class RspRiskCancelOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x201D;
	RspRiskCancelOrderPackage();
	~RspRiskCancelOrderPackage();
	static RspRiskCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	CancelOrderField* CancelOrder = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqMdUserLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x3001;
	ReqMdUserLoginPackage();
	~ReqMdUserLoginPackage();
	static ReqMdUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqMdUserLoginField* ReqMdUserLogin = nullptr;
};
class RspMdUserLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x3002;
	RspMdUserLoginPackage();
	~RspMdUserLoginPackage();
	static RspMdUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspMdUserLoginField* RspMdUserLogin = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqMdUserLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x3003;
	ReqMdUserLogoutPackage();
	~ReqMdUserLogoutPackage();
	static ReqMdUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqMdUserLogoutField* ReqMdUserLogout = nullptr;
};
class RspMdUserLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x3004;
	RspMdUserLogoutPackage();
	~RspMdUserLogoutPackage();
	static RspMdUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspMdUserLogoutField* RspMdUserLogout = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqMdInitPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x3005;
	ReqMdInitPackage();
	~ReqMdInitPackage();
	static ReqMdInitPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqMdInitField* ReqMdInit = nullptr;
};
class RspMdInitPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x3006;
	RspMdInitPackage();
	~RspMdInitPackage();
	static RspMdInitPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspMdInitField* RspMdInit = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqSubscribeMdPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x3007;
	ReqSubscribeMdPackage();
	~ReqSubscribeMdPackage();
	static ReqSubscribeMdPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqSubscribeMdField* ReqSubscribeMd = nullptr;
};
class RspSubscribeMdPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x3008;
	RspSubscribeMdPackage();
	~RspSubscribeMdPackage();
	static RspSubscribeMdPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspSubscribeMdField* RspSubscribeMd = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class RtnShortMdPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x3009;
	RtnShortMdPackage();
	~RtnShortMdPackage();
	static RtnShortMdPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ShortMdField* ShortMd = nullptr;
};
class RtnExchangeStatusPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x300A;
	RtnExchangeStatusPackage();
	~RtnExchangeStatusPackage();
	static RtnExchangeStatusPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RtnExchangeStatusField* RtnExchangeStatus = nullptr;
};
class RtnMdInitCompletedPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x300B;
	RtnMdInitCompletedPackage();
	~RtnMdInitCompletedPackage();
	static RtnMdInitCompletedPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	MdInitCompletedField* MdInitCompleted = nullptr;
};
class ReqAdminUserLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4001;
	ReqAdminUserLoginPackage();
	~ReqAdminUserLoginPackage();
	static ReqAdminUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAdminUserLoginField* ReqAdminUserLogin = nullptr;
};
class RspAdminUserLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4002;
	RspAdminUserLoginPackage();
	~RspAdminUserLoginPackage();
	static RspAdminUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAdminUserLoginField* RspAdminUserLogin = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAdminUserLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4003;
	ReqAdminUserLogoutPackage();
	~ReqAdminUserLogoutPackage();
	static ReqAdminUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAdminUserLogoutField* ReqAdminUserLogout = nullptr;
};
class RspAdminUserLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4004;
	RspAdminUserLogoutPackage();
	~RspAdminUserLogoutPackage();
	static RspAdminUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAdminUserLogoutField* RspAdminUserLogout = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class RtnAdminUserLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4005;
	RtnAdminUserLogoutPackage();
	~RtnAdminUserLogoutPackage();
	static RtnAdminUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	AdminUserLogoutField* AdminUserLogout = nullptr;
};
class ReqAddRiskUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4006;
	ReqAddRiskUserPackage();
	~ReqAddRiskUserPackage();
	static ReqAddRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddRiskUserField* ReqAddRiskUser = nullptr;
};
class RspAddRiskUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4007;
	RspAddRiskUserPackage();
	~RspAddRiskUserPackage();
	static RspAddRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddRiskUserField* RspAddRiskUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateRiskUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4008;
	ReqUpdateRiskUserPackage();
	~ReqUpdateRiskUserPackage();
	static ReqUpdateRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdateRiskUserField* ReqUpdateRiskUser = nullptr;
};
class RspUpdateRiskUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4009;
	RspUpdateRiskUserPackage();
	~RspUpdateRiskUserPackage();
	static RspUpdateRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdateRiskUserField* RspUpdateRiskUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveRiskUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x400A;
	ReqRemoveRiskUserPackage();
	~ReqRemoveRiskUserPackage();
	static ReqRemoveRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveRiskUserField* ReqRemoveRiskUser = nullptr;
};
class RspRemoveRiskUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x400B;
	RspRemoveRiskUserPackage();
	~RspRemoveRiskUserPackage();
	static RspRemoveRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveRiskUserField* RspRemoveRiskUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddAdminUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x400C;
	ReqAddAdminUserPackage();
	~ReqAddAdminUserPackage();
	static ReqAddAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddAdminUserField* ReqAddAdminUser = nullptr;
};
class RspAddAdminUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x400D;
	RspAddAdminUserPackage();
	~RspAddAdminUserPackage();
	static RspAddAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddAdminUserField* RspAddAdminUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateAdminUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x400E;
	ReqUpdateAdminUserPackage();
	~ReqUpdateAdminUserPackage();
	static ReqUpdateAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdateAdminUserField* ReqUpdateAdminUser = nullptr;
};
class RspUpdateAdminUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x400F;
	RspUpdateAdminUserPackage();
	~RspUpdateAdminUserPackage();
	static RspUpdateAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdateAdminUserField* RspUpdateAdminUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveAdminUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4010;
	ReqRemoveAdminUserPackage();
	~ReqRemoveAdminUserPackage();
	static ReqRemoveAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveAdminUserField* ReqRemoveAdminUser = nullptr;
};
class RspRemoveAdminUserPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4011;
	RspRemoveAdminUserPackage();
	~RspRemoveAdminUserPackage();
	static RspRemoveAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveAdminUserField* RspRemoveAdminUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddPrimaryAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4012;
	ReqAddPrimaryAccountPackage();
	~ReqAddPrimaryAccountPackage();
	static ReqAddPrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddPrimaryAccountField* ReqAddPrimaryAccount = nullptr;
};
class RspAddPrimaryAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4013;
	RspAddPrimaryAccountPackage();
	~RspAddPrimaryAccountPackage();
	static RspAddPrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddPrimaryAccountField* RspAddPrimaryAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdatePrimaryAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4014;
	ReqUpdatePrimaryAccountPackage();
	~ReqUpdatePrimaryAccountPackage();
	static ReqUpdatePrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdatePrimaryAccountField* ReqUpdatePrimaryAccount = nullptr;
};
class RspUpdatePrimaryAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4015;
	RspUpdatePrimaryAccountPackage();
	~RspUpdatePrimaryAccountPackage();
	static RspUpdatePrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdatePrimaryAccountField* RspUpdatePrimaryAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemovePrimaryAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4016;
	ReqRemovePrimaryAccountPackage();
	~ReqRemovePrimaryAccountPackage();
	static ReqRemovePrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemovePrimaryAccountField* ReqRemovePrimaryAccount = nullptr;
};
class RspRemovePrimaryAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4017;
	RspRemovePrimaryAccountPackage();
	~RspRemovePrimaryAccountPackage();
	static RspRemovePrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemovePrimaryAccountField* RspRemovePrimaryAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4018;
	ReqAddAccountPackage();
	~ReqAddAccountPackage();
	static ReqAddAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddAccountField* ReqAddAccount = nullptr;
};
class RspAddAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4019;
	RspAddAccountPackage();
	~RspAddAccountPackage();
	static RspAddAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddAccountField* RspAddAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x401A;
	ReqUpdateAccountPackage();
	~ReqUpdateAccountPackage();
	static ReqUpdateAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdateAccountField* ReqUpdateAccount = nullptr;
};
class RspUpdateAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x401B;
	RspUpdateAccountPackage();
	~RspUpdateAccountPackage();
	static RspUpdateAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdateAccountField* RspUpdateAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x401C;
	ReqRemoveAccountPackage();
	~ReqRemoveAccountPackage();
	static ReqRemoveAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveAccountField* ReqRemoveAccount = nullptr;
};
class RspRemoveAccountPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x401D;
	RspRemoveAccountPackage();
	~RspRemoveAccountPackage();
	static RspRemoveAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveAccountField* RspRemoveAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddBaseCommissionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x401E;
	ReqAddBaseCommissionPackage();
	~ReqAddBaseCommissionPackage();
	static ReqAddBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddBaseCommissionField* ReqAddBaseCommission = nullptr;
};
class RspAddBaseCommissionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x401F;
	RspAddBaseCommissionPackage();
	~RspAddBaseCommissionPackage();
	static RspAddBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddBaseCommissionField* RspAddBaseCommission = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateBaseCommissionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4020;
	ReqUpdateBaseCommissionPackage();
	~ReqUpdateBaseCommissionPackage();
	static ReqUpdateBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdateBaseCommissionField* ReqUpdateBaseCommission = nullptr;
};
class RspUpdateBaseCommissionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4021;
	RspUpdateBaseCommissionPackage();
	~RspUpdateBaseCommissionPackage();
	static RspUpdateBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdateBaseCommissionField* RspUpdateBaseCommission = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveBaseCommissionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4022;
	ReqRemoveBaseCommissionPackage();
	~ReqRemoveBaseCommissionPackage();
	static ReqRemoveBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveBaseCommissionField* ReqRemoveBaseCommission = nullptr;
};
class RspRemoveBaseCommissionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4023;
	RspRemoveBaseCommissionPackage();
	~RspRemoveBaseCommissionPackage();
	static RspRemoveBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspInfoField* RspInfo = nullptr;
	RspRemoveBaseCommissionField* RspRemoveBaseCommission = nullptr;
};
class ReqAddCommissionGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4024;
	ReqAddCommissionGroupPackage();
	~ReqAddCommissionGroupPackage();
	static ReqAddCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddCommissionGroupField* ReqAddCommissionGroup = nullptr;
};
class RspAddCommissionGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4025;
	RspAddCommissionGroupPackage();
	~RspAddCommissionGroupPackage();
	static RspAddCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddCommissionGroupField* RspAddCommissionGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateCommissionGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4026;
	ReqUpdateCommissionGroupPackage();
	~ReqUpdateCommissionGroupPackage();
	static ReqUpdateCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdateCommissionGroupField* ReqUpdateCommissionGroup = nullptr;
};
class RspUpdateCommissionGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4027;
	RspUpdateCommissionGroupPackage();
	~RspUpdateCommissionGroupPackage();
	static RspUpdateCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdateCommissionGroupField* RspUpdateCommissionGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveCommissionGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4028;
	ReqRemoveCommissionGroupPackage();
	~ReqRemoveCommissionGroupPackage();
	static ReqRemoveCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveCommissionGroupField* ReqRemoveCommissionGroup = nullptr;
};
class RspRemoveCommissionGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4029;
	RspRemoveCommissionGroupPackage();
	~RspRemoveCommissionGroupPackage();
	static RspRemoveCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveCommissionGroupField* RspRemoveCommissionGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddOptionMarginParamPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x402A;
	ReqAddOptionMarginParamPackage();
	~ReqAddOptionMarginParamPackage();
	static ReqAddOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddOptionMarginParamField* ReqAddOptionMarginParam = nullptr;
};
class RspAddOptionMarginParamPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x402B;
	RspAddOptionMarginParamPackage();
	~RspAddOptionMarginParamPackage();
	static RspAddOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddOptionMarginParamField* RspAddOptionMarginParam = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateOptionMarginParamPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x402C;
	ReqUpdateOptionMarginParamPackage();
	~ReqUpdateOptionMarginParamPackage();
	static ReqUpdateOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdateOptionMarginParamField* ReqUpdateOptionMarginParam = nullptr;
};
class RspUpdateOptionMarginParamPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x402D;
	RspUpdateOptionMarginParamPackage();
	~RspUpdateOptionMarginParamPackage();
	static RspUpdateOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdateOptionMarginParamField* RspUpdateOptionMarginParam = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveOptionMarginParamPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x402E;
	ReqRemoveOptionMarginParamPackage();
	~ReqRemoveOptionMarginParamPackage();
	static ReqRemoveOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveOptionMarginParamField* ReqRemoveOptionMarginParam = nullptr;
};
class RspRemoveOptionMarginParamPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x402F;
	RspRemoveOptionMarginParamPackage();
	~RspRemoveOptionMarginParamPackage();
	static RspRemoveOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveOptionMarginParamField* RspRemoveOptionMarginParam = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddTradeOfferPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4030;
	ReqAddTradeOfferPackage();
	~ReqAddTradeOfferPackage();
	static ReqAddTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddTradeOfferField* ReqAddTradeOffer = nullptr;
};
class RspAddTradeOfferPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4031;
	RspAddTradeOfferPackage();
	~RspAddTradeOfferPackage();
	static RspAddTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddTradeOfferField* RspAddTradeOffer = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateTradeOfferPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4032;
	ReqUpdateTradeOfferPackage();
	~ReqUpdateTradeOfferPackage();
	static ReqUpdateTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdateTradeOfferField* ReqUpdateTradeOffer = nullptr;
};
class RspUpdateTradeOfferPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4033;
	RspUpdateTradeOfferPackage();
	~RspUpdateTradeOfferPackage();
	static RspUpdateTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdateTradeOfferField* RspUpdateTradeOffer = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveTradeOfferPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4034;
	ReqRemoveTradeOfferPackage();
	~ReqRemoveTradeOfferPackage();
	static ReqRemoveTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveTradeOfferField* ReqRemoveTradeOffer = nullptr;
};
class RspRemoveTradeOfferPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4035;
	RspRemoveTradeOfferPackage();
	~RspRemoveTradeOfferPackage();
	static RspRemoveTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveTradeOfferField* RspRemoveTradeOffer = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddTradeGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4036;
	ReqAddTradeGroupPackage();
	~ReqAddTradeGroupPackage();
	static ReqAddTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddTradeGroupField* ReqAddTradeGroup = nullptr;
};
class RspAddTradeGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4037;
	RspAddTradeGroupPackage();
	~RspAddTradeGroupPackage();
	static RspAddTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddTradeGroupField* RspAddTradeGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateTradeGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4038;
	ReqUpdateTradeGroupPackage();
	~ReqUpdateTradeGroupPackage();
	static ReqUpdateTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdateTradeGroupField* ReqUpdateTradeGroup = nullptr;
};
class RspUpdateTradeGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4039;
	RspUpdateTradeGroupPackage();
	~RspUpdateTradeGroupPackage();
	static RspUpdateTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdateTradeGroupField* RspUpdateTradeGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveTradeGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x403A;
	ReqRemoveTradeGroupPackage();
	~ReqRemoveTradeGroupPackage();
	static ReqRemoveTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveTradeGroupField* ReqRemoveTradeGroup = nullptr;
};
class RspRemoveTradeGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x403B;
	RspRemoveTradeGroupPackage();
	~RspRemoveTradeGroupPackage();
	static RspRemoveTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveTradeGroupField* RspRemoveTradeGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddTradeGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x403C;
	ReqAddTradeGroupItemPackage();
	~ReqAddTradeGroupItemPackage();
	static ReqAddTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddTradeGroupItemField* ReqAddTradeGroupItem = nullptr;
};
class RspAddTradeGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x403D;
	RspAddTradeGroupItemPackage();
	~RspAddTradeGroupItemPackage();
	static RspAddTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddTradeGroupItemField* RspAddTradeGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateTradeGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x403E;
	ReqUpdateTradeGroupItemPackage();
	~ReqUpdateTradeGroupItemPackage();
	static ReqUpdateTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdateTradeGroupItemField* ReqUpdateTradeGroupItem = nullptr;
};
class RspUpdateTradeGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x403F;
	RspUpdateTradeGroupItemPackage();
	~RspUpdateTradeGroupItemPackage();
	static RspUpdateTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdateTradeGroupItemField* RspUpdateTradeGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveTradeGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4040;
	ReqRemoveTradeGroupItemPackage();
	~ReqRemoveTradeGroupItemPackage();
	static ReqRemoveTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveTradeGroupItemField* ReqRemoveTradeGroupItem = nullptr;
};
class RspRemoveTradeGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4041;
	RspRemoveTradeGroupItemPackage();
	~RspRemoveTradeGroupItemPackage();
	static RspRemoveTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveTradeGroupItemField* RspRemoveTradeGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddRiskGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4042;
	ReqAddRiskGroupPackage();
	~ReqAddRiskGroupPackage();
	static ReqAddRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddRiskGroupField* ReqAddRiskGroup = nullptr;
};
class RspAddRiskGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4043;
	RspAddRiskGroupPackage();
	~RspAddRiskGroupPackage();
	static RspAddRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddRiskGroupField* RspAddRiskGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateRiskGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4044;
	ReqUpdateRiskGroupPackage();
	~ReqUpdateRiskGroupPackage();
	static ReqUpdateRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdateRiskGroupField* ReqUpdateRiskGroup = nullptr;
};
class RspUpdateRiskGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4045;
	RspUpdateRiskGroupPackage();
	~RspUpdateRiskGroupPackage();
	static RspUpdateRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdateRiskGroupField* RspUpdateRiskGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveRiskGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4046;
	ReqRemoveRiskGroupPackage();
	~ReqRemoveRiskGroupPackage();
	static ReqRemoveRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveRiskGroupField* ReqRemoveRiskGroup = nullptr;
};
class RspRemoveRiskGroupPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4047;
	RspRemoveRiskGroupPackage();
	~RspRemoveRiskGroupPackage();
	static RspRemoveRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveRiskGroupField* RspRemoveRiskGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddRiskGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4048;
	ReqAddRiskGroupItemPackage();
	~ReqAddRiskGroupItemPackage();
	static ReqAddRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddRiskGroupItemField* ReqAddRiskGroupItem = nullptr;
};
class RspAddRiskGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4049;
	RspAddRiskGroupItemPackage();
	~RspAddRiskGroupItemPackage();
	static RspAddRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddRiskGroupItemField* RspAddRiskGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateRiskGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x404A;
	ReqUpdateRiskGroupItemPackage();
	~ReqUpdateRiskGroupItemPackage();
	static ReqUpdateRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqUpdateRiskGroupItemField* ReqUpdateRiskGroupItem = nullptr;
};
class RspUpdateRiskGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x404B;
	RspUpdateRiskGroupItemPackage();
	~RspUpdateRiskGroupItemPackage();
	static RspUpdateRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspUpdateRiskGroupItemField* RspUpdateRiskGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveRiskGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x404C;
	ReqRemoveRiskGroupItemPackage();
	~ReqRemoveRiskGroupItemPackage();
	static ReqRemoveRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveRiskGroupItemField* ReqRemoveRiskGroupItem = nullptr;
};
class RspRemoveRiskGroupItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x404D;
	RspRemoveRiskGroupItemPackage();
	~RspRemoveRiskGroupItemPackage();
	static RspRemoveRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveRiskGroupItemField* RspRemoveRiskGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddOrUpdateRiskPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x404E;
	ReqAddOrUpdateRiskPackage();
	~ReqAddOrUpdateRiskPackage();
	static ReqAddOrUpdateRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddOrUpdateRiskField* ReqAddOrUpdateRisk = nullptr;
};
class RspAddOrUpdateRiskPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x404F;
	RspAddOrUpdateRiskPackage();
	~RspAddOrUpdateRiskPackage();
	static RspAddOrUpdateRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddOrUpdateRiskField* RspAddOrUpdateRisk = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveRiskPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4050;
	ReqRemoveRiskPackage();
	~ReqRemoveRiskPackage();
	static ReqRemoveRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveRiskField* ReqRemoveRisk = nullptr;
};
class RspRemoveRiskPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4051;
	RspRemoveRiskPackage();
	~RspRemoveRiskPackage();
	static RspRemoveRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveRiskField* RspRemoveRisk = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddRiskRulePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4052;
	ReqAddRiskRulePackage();
	~ReqAddRiskRulePackage();
	static ReqAddRiskRulePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddRiskRuleField* ReqAddRiskRule = nullptr;
};
class RspAddRiskRulePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4053;
	RspAddRiskRulePackage();
	~RspAddRiskRulePackage();
	static RspAddRiskRulePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddRiskRuleField* RspAddRiskRule = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddRiskRuleItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4054;
	ReqAddRiskRuleItemPackage();
	~ReqAddRiskRuleItemPackage();
	static ReqAddRiskRuleItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddRiskRuleItemField* ReqAddRiskRuleItem = nullptr;
};
class RspAddRiskRuleItemPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4055;
	RspAddRiskRuleItemPackage();
	~RspAddRiskRuleItemPackage();
	static RspAddRiskRuleItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddRiskRuleItemField* RspAddRiskRuleItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddRiskTradeScopePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4056;
	ReqAddRiskTradeScopePackage();
	~ReqAddRiskTradeScopePackage();
	static ReqAddRiskTradeScopePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddRiskTradeScopeField* ReqAddRiskTradeScope = nullptr;
};
class RspAddRiskTradeScopePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4057;
	RspAddRiskTradeScopePackage();
	~RspAddRiskTradeScopePackage();
	static RspAddRiskTradeScopePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddRiskTradeScopeField* RspAddRiskTradeScope = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddAccountRiskPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4058;
	ReqAddAccountRiskPackage();
	~ReqAddAccountRiskPackage();
	static ReqAddAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAddAccountRiskField* ReqAddAccountRisk = nullptr;
};
class RspAddAccountRiskPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x4059;
	RspAddAccountRiskPackage();
	~RspAddAccountRiskPackage();
	static RspAddAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAddAccountRiskField* RspAddAccountRisk = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveAccountRiskPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x405A;
	ReqRemoveAccountRiskPackage();
	~ReqRemoveAccountRiskPackage();
	static ReqRemoveAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqRemoveAccountRiskField* ReqRemoveAccountRisk = nullptr;
};
class RspRemoveAccountRiskPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x405B;
	RspRemoveAccountRiskPackage();
	~RspRemoveAccountRiskPackage();
	static RspRemoveAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspRemoveAccountRiskField* RspRemoveAccountRisk = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqMoneyTransferPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x405C;
	ReqMoneyTransferPackage();
	~ReqMoneyTransferPackage();
	static ReqMoneyTransferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqMoneyTransferField* ReqMoneyTransfer = nullptr;
};
class RspMoneyTransferPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x405D;
	RspMoneyTransferPackage();
	~RspMoneyTransferPackage();
	static RspMoneyTransferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspMoneyTransferField* RspMoneyTransfer = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAuditOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x405E;
	ReqAuditOrderPackage();
	~ReqAuditOrderPackage();
	static ReqAuditOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqAuditOrderField* ReqAuditOrder = nullptr;
};
class RspAuditOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x405F;
	RspAuditOrderPackage();
	~RspAuditOrderPackage();
	static RspAuditOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspAuditOrderField* RspAuditOrder = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqOfferLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5001;
	ReqOfferLoginPackage();
	~ReqOfferLoginPackage();
	static ReqOfferLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqOfferLoginField* ReqOfferLogin = nullptr;
};
class RspOfferLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5002;
	RspOfferLoginPackage();
	~RspOfferLoginPackage();
	static RspOfferLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspOfferLoginField* RspOfferLogin = nullptr;
};
class ReqPrimaryAccountLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5003;
	ReqPrimaryAccountLoginPackage();
	~ReqPrimaryAccountLoginPackage();
	static ReqPrimaryAccountLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqPrimaryAccountLoginField* ReqPrimaryAccountLogin = nullptr;
};
class RspPrimaryAccountLoginPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5004;
	RspPrimaryAccountLoginPackage();
	~RspPrimaryAccountLoginPackage();
	static RspPrimaryAccountLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspPrimaryAccountLoginField* RspPrimaryAccountLogin = nullptr;
};
class ReqPrimaryAccountLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5005;
	ReqPrimaryAccountLogoutPackage();
	~ReqPrimaryAccountLogoutPackage();
	static ReqPrimaryAccountLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqPrimaryAccountLogoutField* ReqPrimaryAccountLogout = nullptr;
};
class RtnPrimaryAccountLogoutPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5006;
	RtnPrimaryAccountLogoutPackage();
	~RtnPrimaryAccountLogoutPackage();
	static RtnPrimaryAccountLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RtnPrimaryAccountLogoutField* RtnPrimaryAccountLogout = nullptr;
};
class ReqPrimaryAccountInitPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5007;
	ReqPrimaryAccountInitPackage();
	~ReqPrimaryAccountInitPackage();
	static ReqPrimaryAccountInitPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqPrimaryAccountInitField* ReqPrimaryAccountInit = nullptr;
};
class RspPrimaryAccountInitPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5008;
	RspPrimaryAccountInitPackage();
	~RspPrimaryAccountInitPackage();
	static RspPrimaryAccountInitPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspPrimaryAccountInitField* RspPrimaryAccountInit = nullptr;
};
class ReqPrimaryAccountQueryPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5009;
	ReqPrimaryAccountQueryPackage();
	~ReqPrimaryAccountQueryPackage();
	static ReqPrimaryAccountQueryPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqPrimaryAccountQueryField* ReqPrimaryAccountQuery = nullptr;
};
class RspPrimaryAccountQueryPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x500A;
	RspPrimaryAccountQueryPackage();
	~RspPrimaryAccountQueryPackage();
	static RspPrimaryAccountQueryPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspPrimaryAccountQueryField* RspPrimaryAccountQuery = nullptr;
};
class ReqQryOfferOptionInstrumentPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x500B;
	ReqQryOfferOptionInstrumentPackage();
	~ReqQryOfferOptionInstrumentPackage();
	static ReqQryOfferOptionInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqQryOfferOptionInstrumentField* ReqQryOfferOptionInstrument = nullptr;
};
class RspQryOfferOptionInstrumentPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x500C;
	RspQryOfferOptionInstrumentPackage();
	~RspQryOfferOptionInstrumentPackage();
	static RspQryOfferOptionInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	RspQryOfferOptionInstrumentField* RspQryOfferOptionInstrument = nullptr;
};
class RtnOfferOptionInstrumentPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x500D;
	RtnOfferOptionInstrumentPackage();
	~RtnOfferOptionInstrumentPackage();
	static RtnOfferOptionInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OfferOptionInstrumentField* OfferOptionInstrument = nullptr;
};
class ReqOfferOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x500E;
	ReqOfferOrderPackage();
	~ReqOfferOrderPackage();
	static ReqOfferOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqOfferOrderField* ReqOfferOrder = nullptr;
};
class ReqOfferCancelOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x500F;
	ReqOfferCancelOrderPackage();
	~ReqOfferCancelOrderPackage();
	static ReqOfferCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	ReqOfferCancelOrderField* ReqOfferCancelOrder = nullptr;
};
class RtnOfferOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5010;
	RtnOfferOrderPackage();
	~RtnOfferOrderPackage();
	static RtnOfferOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OfferOrderField* OfferOrder = nullptr;
};
class RtnOfferTradePackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5011;
	RtnOfferTradePackage();
	~RtnOfferTradePackage();
	static RtnOfferTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OfferTradeField* OfferTrade = nullptr;
};
class RtnOfferErrorCancelOrderPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5012;
	RtnOfferErrorCancelOrderPackage();
	~RtnOfferErrorCancelOrderPackage();
	static RtnOfferErrorCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OfferErrorCancelOrderField* OfferErrorCancelOrder = nullptr;
};
class RtnOfferCapitalPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5013;
	RtnOfferCapitalPackage();
	~RtnOfferCapitalPackage();
	static RtnOfferCapitalPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OfferCapitalField* OfferCapital = nullptr;
};
class RtnOfferPositionPackage : public Package
{
public:
	static constexpr UInt16Type PackageId = 0x5014;
	RtnOfferPositionPackage();
	~RtnOfferPositionPackage();
	static RtnOfferPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIdType sessionId, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;

	OfferPositionField* OfferPosition = nullptr;
};
}
