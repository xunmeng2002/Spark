#pragma once
#include <Spark/Fields.h>
#include <Spark/Network/Protocol/Package.h>

using spark::network::Package;

namespace spark::packages
{
class NotifyComponentConnectStatusPackage : public Package
{
public:
	NotifyComponentConnectStatusPackage();
	~NotifyComponentConnectStatusPackage();
	static NotifyComponentConnectStatusPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x0001;
	NotifyComponentConnectStatusField* NotifyComponentConnectStatus = nullptr;
};
class ReqAccountLoginPackage : public Package
{
public:
	ReqAccountLoginPackage();
	~ReqAccountLoginPackage();
	static ReqAccountLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1001;
	ReqAccountLoginField* ReqAccountLogin = nullptr;
};
class RspAccountLoginPackage : public Package
{
public:
	RspAccountLoginPackage();
	~RspAccountLoginPackage();
	static RspAccountLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1002;
	RspAccountLoginField* RspAccountLogin = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAccountLogoutPackage : public Package
{
public:
	ReqAccountLogoutPackage();
	~ReqAccountLogoutPackage();
	static ReqAccountLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1003;
	ReqAccountLogoutField* ReqAccountLogout = nullptr;
};
class RspAccountLogoutPackage : public Package
{
public:
	RspAccountLogoutPackage();
	~RspAccountLogoutPackage();
	static RspAccountLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1004;
	RspAccountLogoutField* RspAccountLogout = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryAccountPackage : public Package
{
public:
	ReqQryAccountPackage();
	~ReqQryAccountPackage();
	static ReqQryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1005;
	ReqQryAccountField* ReqQryAccount = nullptr;
};
class RspQryAccountPackage : public Package
{
public:
	RspQryAccountPackage();
	~RspQryAccountPackage();
	static RspQryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1006;
	AccountField* Account = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryHolderAccountPackage : public Package
{
public:
	ReqQryHolderAccountPackage();
	~ReqQryHolderAccountPackage();
	static ReqQryHolderAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1007;
	ReqQryHolderAccountField* ReqQryHolderAccount = nullptr;
};
class RspQryHolderAccountPackage : public Package
{
public:
	RspQryHolderAccountPackage();
	~RspQryHolderAccountPackage();
	static RspQryHolderAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1008;
	HolderAccountField* HolderAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryCapitalPackage : public Package
{
public:
	ReqQryCapitalPackage();
	~ReqQryCapitalPackage();
	static ReqQryCapitalPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1009;
	ReqQryCapitalField* ReqQryCapital = nullptr;
};
class RspQryCapitalPackage : public Package
{
public:
	RspQryCapitalPackage();
	~RspQryCapitalPackage();
	static RspQryCapitalPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x100A;
	CapitalField* Capital = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryPositionPackage : public Package
{
public:
	ReqQryPositionPackage();
	~ReqQryPositionPackage();
	static ReqQryPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x100B;
	ReqQryPositionField* ReqQryPosition = nullptr;
};
class RspQryPositionPackage : public Package
{
public:
	RspQryPositionPackage();
	~RspQryPositionPackage();
	static RspQryPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x100C;
	PositionField* Position = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryOrderPackage : public Package
{
public:
	ReqQryOrderPackage();
	~ReqQryOrderPackage();
	static ReqQryOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x100D;
	ReqQryOrderField* ReqQryOrder = nullptr;
};
class RspQryOrderPackage : public Package
{
public:
	RspQryOrderPackage();
	~RspQryOrderPackage();
	static RspQryOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x100E;
	OrderField* Order = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryTradePackage : public Package
{
public:
	ReqQryTradePackage();
	~ReqQryTradePackage();
	static ReqQryTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x100F;
	ReqQryTradeField* ReqQryTrade = nullptr;
};
class RspQryTradePackage : public Package
{
public:
	RspQryTradePackage();
	~RspQryTradePackage();
	static RspQryTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1010;
	TradeField* Trade = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryInstrumentPackage : public Package
{
public:
	ReqQryInstrumentPackage();
	~ReqQryInstrumentPackage();
	static ReqQryInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1011;
	ReqQryInstrumentField* ReqQryInstrument = nullptr;
};
class RspQryInstrumentPackage : public Package
{
public:
	RspQryInstrumentPackage();
	~RspQryInstrumentPackage();
	static RspQryInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1012;
	InstrumentField* Instrument = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryOptionInstrumentPackage : public Package
{
public:
	ReqQryOptionInstrumentPackage();
	~ReqQryOptionInstrumentPackage();
	static ReqQryOptionInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1013;
	ReqQryOptionInstrumentField* ReqQryOptionInstrument = nullptr;
};
class RspQryOptionInstrumentPackage : public Package
{
public:
	RspQryOptionInstrumentPackage();
	~RspQryOptionInstrumentPackage();
	static RspQryOptionInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1014;
	OptionInstrumentField* OptionInstrument = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryCommissionRatePackage : public Package
{
public:
	ReqQryCommissionRatePackage();
	~ReqQryCommissionRatePackage();
	static ReqQryCommissionRatePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1015;
	ReqQryCommissionRateField* ReqQryCommissionRate = nullptr;
};
class RspQryCommissionRatePackage : public Package
{
public:
	RspQryCommissionRatePackage();
	~RspQryCommissionRatePackage();
	static RspQryCommissionRatePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1016;
	CommissionRateField* CommissionRate = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryMoneyTransferPackage : public Package
{
public:
	ReqQryMoneyTransferPackage();
	~ReqQryMoneyTransferPackage();
	static ReqQryMoneyTransferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1017;
	ReqQryMoneyTransferField* ReqQryMoneyTransfer = nullptr;
};
class RspQryMoneyTransferPackage : public Package
{
public:
	RspQryMoneyTransferPackage();
	~RspQryMoneyTransferPackage();
	static RspQryMoneyTransferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1018;
	MoneyTransferField* MoneyTransfer = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqInsertOrderPackage : public Package
{
public:
	ReqInsertOrderPackage();
	~ReqInsertOrderPackage();
	static ReqInsertOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1019;
	ReqInsertOrderField* ReqInsertOrder = nullptr;
};
class RspInsertOrderPackage : public Package
{
public:
	RspInsertOrderPackage();
	~RspInsertOrderPackage();
	static RspInsertOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x101A;
	OrderField* Order = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqCancelOrderPackage : public Package
{
public:
	ReqCancelOrderPackage();
	~ReqCancelOrderPackage();
	static ReqCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x101B;
	ReqCancelOrderField* ReqCancelOrder = nullptr;
};
class RspCancelOrderPackage : public Package
{
public:
	RspCancelOrderPackage();
	~RspCancelOrderPackage();
	static RspCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x101C;
	CancelOrderField* CancelOrder = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class RtnOrderPackage : public Package
{
public:
	RtnOrderPackage();
	~RtnOrderPackage();
	static RtnOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x101D;
	OrderField* Order = nullptr;
};
class RtnTradePackage : public Package
{
public:
	RtnTradePackage();
	~RtnTradePackage();
	static RtnTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x101E;
	TradeField* Trade = nullptr;
};
class RtnMoneyTransferPackage : public Package
{
public:
	RtnMoneyTransferPackage();
	~RtnMoneyTransferPackage();
	static RtnMoneyTransferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x101F;
	MoneyTransferField* MoneyTransfer = nullptr;
};
class RtnAccountLogoutPackage : public Package
{
public:
	RtnAccountLogoutPackage();
	~RtnAccountLogoutPackage();
	static RtnAccountLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x1020;
	AccountLogoutField* AccountLogout = nullptr;
};
class ReqRiskUserLoginPackage : public Package
{
public:
	ReqRiskUserLoginPackage();
	~ReqRiskUserLoginPackage();
	static ReqRiskUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2001;
	ReqRiskUserLoginField* ReqRiskUserLogin = nullptr;
};
class RspRiskUserLoginPackage : public Package
{
public:
	RspRiskUserLoginPackage();
	~RspRiskUserLoginPackage();
	static RspRiskUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2002;
	RspRiskUserLoginField* RspRiskUserLogin = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRiskUserLogoutPackage : public Package
{
public:
	ReqRiskUserLogoutPackage();
	~ReqRiskUserLogoutPackage();
	static ReqRiskUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2003;
	ReqRiskUserLogoutField* ReqRiskUserLogout = nullptr;
};
class RspRiskUserLogoutPackage : public Package
{
public:
	RspRiskUserLogoutPackage();
	~RspRiskUserLogoutPackage();
	static RspRiskUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2004;
	RspRiskUserLogoutField* RspRiskUserLogout = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class RtnRiskUserLogoutPackage : public Package
{
public:
	RtnRiskUserLogoutPackage();
	~RtnRiskUserLogoutPackage();
	static RtnRiskUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2005;
	RiskUserLogoutField* RiskUserLogout = nullptr;
};
class RtnAccountPackage : public Package
{
public:
	RtnAccountPackage();
	~RtnAccountPackage();
	static RtnAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2006;
	AccountField* Account = nullptr;
};
class RtnAccountDeletePackage : public Package
{
public:
	RtnAccountDeletePackage();
	~RtnAccountDeletePackage();
	static RtnAccountDeletePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2007;
	AccountDeleteField* AccountDelete = nullptr;
};
class RtnPositionPackage : public Package
{
public:
	RtnPositionPackage();
	~RtnPositionPackage();
	static RtnPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2008;
	PositionField* Position = nullptr;
};
class RtnAccountRiskPackage : public Package
{
public:
	RtnAccountRiskPackage();
	~RtnAccountRiskPackage();
	static RtnAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2009;
	AccountRiskField* AccountRisk = nullptr;
};
class RtnAccountRiskDeletePackage : public Package
{
public:
	RtnAccountRiskDeletePackage();
	~RtnAccountRiskDeletePackage();
	static RtnAccountRiskDeletePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x200A;
	AccountRiskDeleteField* AccountRiskDelete = nullptr;
};
class RtnAccountRiskNotifyPackage : public Package
{
public:
	RtnAccountRiskNotifyPackage();
	~RtnAccountRiskNotifyPackage();
	static RtnAccountRiskNotifyPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x200B;
	AccountRiskNotifyField* AccountRiskNotify = nullptr;
};
class ReqQryRiskGroupAccountPackage : public Package
{
public:
	ReqQryRiskGroupAccountPackage();
	~ReqQryRiskGroupAccountPackage();
	static ReqQryRiskGroupAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x200C;
	ReqQryRiskGroupAccountField* ReqQryRiskGroupAccount = nullptr;
};
class RspQryRiskGroupAccountPackage : public Package
{
public:
	RspQryRiskGroupAccountPackage();
	~RspQryRiskGroupAccountPackage();
	static RspQryRiskGroupAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x200D;
	AccountField* Account = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupCapitalPackage : public Package
{
public:
	ReqQryRiskGroupCapitalPackage();
	~ReqQryRiskGroupCapitalPackage();
	static ReqQryRiskGroupCapitalPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x200E;
	ReqQryRiskGroupCapitalField* ReqQryRiskGroupCapital = nullptr;
};
class RspQryRiskGroupCapitalPackage : public Package
{
public:
	RspQryRiskGroupCapitalPackage();
	~RspQryRiskGroupCapitalPackage();
	static RspQryRiskGroupCapitalPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x200F;
	CapitalField* Capital = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupPositionPackage : public Package
{
public:
	ReqQryRiskGroupPositionPackage();
	~ReqQryRiskGroupPositionPackage();
	static ReqQryRiskGroupPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2010;
	ReqQryRiskGroupPositionField* ReqQryRiskGroupPosition = nullptr;
};
class RspQryRiskGroupPositionPackage : public Package
{
public:
	RspQryRiskGroupPositionPackage();
	~RspQryRiskGroupPositionPackage();
	static RspQryRiskGroupPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2011;
	PositionField* Position = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupOrderPackage : public Package
{
public:
	ReqQryRiskGroupOrderPackage();
	~ReqQryRiskGroupOrderPackage();
	static ReqQryRiskGroupOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2012;
	ReqQryRiskGroupOrderField* ReqQryRiskGroupOrder = nullptr;
};
class RspQryRiskGroupOrderPackage : public Package
{
public:
	RspQryRiskGroupOrderPackage();
	~RspQryRiskGroupOrderPackage();
	static RspQryRiskGroupOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2013;
	OrderField* Order = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupTradePackage : public Package
{
public:
	ReqQryRiskGroupTradePackage();
	~ReqQryRiskGroupTradePackage();
	static ReqQryRiskGroupTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2014;
	ReqQryRiskGroupTradeField* ReqQryRiskGroupTrade = nullptr;
};
class RspQryRiskGroupTradePackage : public Package
{
public:
	RspQryRiskGroupTradePackage();
	~RspQryRiskGroupTradePackage();
	static RspQryRiskGroupTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2015;
	TradeField* Trade = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupAccountRiskPackage : public Package
{
public:
	ReqQryRiskGroupAccountRiskPackage();
	~ReqQryRiskGroupAccountRiskPackage();
	static ReqQryRiskGroupAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2016;
	ReqQryRiskGroupAccountRiskField* ReqQryRiskGroupAccountRisk = nullptr;
};
class RspQryRiskGroupAccountRiskPackage : public Package
{
public:
	RspQryRiskGroupAccountRiskPackage();
	~RspQryRiskGroupAccountRiskPackage();
	static RspQryRiskGroupAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2017;
	AccountRiskField* AccountRisk = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqQryRiskGroupAccountRiskNotifyPackage : public Package
{
public:
	ReqQryRiskGroupAccountRiskNotifyPackage();
	~ReqQryRiskGroupAccountRiskNotifyPackage();
	static ReqQryRiskGroupAccountRiskNotifyPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2018;
	ReqQryRiskGroupAccountRiskNotifyField* ReqQryRiskGroupAccountRiskNotify = nullptr;
};
class RspQryRiskGroupAccountRiskNotifyPackage : public Package
{
public:
	RspQryRiskGroupAccountRiskNotifyPackage();
	~RspQryRiskGroupAccountRiskNotifyPackage();
	static RspQryRiskGroupAccountRiskNotifyPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x2019;
	AccountRiskNotifyField* AccountRiskNotify = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRiskInsertOrderPackage : public Package
{
public:
	ReqRiskInsertOrderPackage();
	~ReqRiskInsertOrderPackage();
	static ReqRiskInsertOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x201A;
	ReqRiskInsertOrderField* ReqRiskInsertOrder = nullptr;
};
class RspRiskInsertOrderPackage : public Package
{
public:
	RspRiskInsertOrderPackage();
	~RspRiskInsertOrderPackage();
	static RspRiskInsertOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x201B;
	OrderField* Order = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRiskCancelOrderPackage : public Package
{
public:
	ReqRiskCancelOrderPackage();
	~ReqRiskCancelOrderPackage();
	static ReqRiskCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x201C;
	ReqRiskCancelOrderField* ReqRiskCancelOrder = nullptr;
};
class RspRiskCancelOrderPackage : public Package
{
public:
	RspRiskCancelOrderPackage();
	~RspRiskCancelOrderPackage();
	static RspRiskCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x201D;
	CancelOrderField* CancelOrder = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqMdUserLoginPackage : public Package
{
public:
	ReqMdUserLoginPackage();
	~ReqMdUserLoginPackage();
	static ReqMdUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x3001;
	ReqMdUserLoginField* ReqMdUserLogin = nullptr;
};
class RspMdUserLoginPackage : public Package
{
public:
	RspMdUserLoginPackage();
	~RspMdUserLoginPackage();
	static RspMdUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x3002;
	RspMdUserLoginField* RspMdUserLogin = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqMdUserLogoutPackage : public Package
{
public:
	ReqMdUserLogoutPackage();
	~ReqMdUserLogoutPackage();
	static ReqMdUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x3003;
	ReqMdUserLogoutField* ReqMdUserLogout = nullptr;
};
class RspMdUserLogoutPackage : public Package
{
public:
	RspMdUserLogoutPackage();
	~RspMdUserLogoutPackage();
	static RspMdUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x3004;
	RspMdUserLogoutField* RspMdUserLogout = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqMdInitPackage : public Package
{
public:
	ReqMdInitPackage();
	~ReqMdInitPackage();
	static ReqMdInitPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x3005;
	ReqMdInitField* ReqMdInit = nullptr;
};
class RspMdInitPackage : public Package
{
public:
	RspMdInitPackage();
	~RspMdInitPackage();
	static RspMdInitPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x3006;
	RspMdInitField* RspMdInit = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqSubscribeMdPackage : public Package
{
public:
	ReqSubscribeMdPackage();
	~ReqSubscribeMdPackage();
	static ReqSubscribeMdPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x3007;
	ReqSubscribeMdField* ReqSubscribeMd = nullptr;
};
class RspSubscribeMdPackage : public Package
{
public:
	RspSubscribeMdPackage();
	~RspSubscribeMdPackage();
	static RspSubscribeMdPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x3008;
	RspSubscribeMdField* RspSubscribeMd = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class RtnShortMdPackage : public Package
{
public:
	RtnShortMdPackage();
	~RtnShortMdPackage();
	static RtnShortMdPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x3009;
	ShortMdField* ShortMd = nullptr;
};
class RtnExchangeStatusPackage : public Package
{
public:
	RtnExchangeStatusPackage();
	~RtnExchangeStatusPackage();
	static RtnExchangeStatusPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x300A;
	RtnExchangeStatusField* RtnExchangeStatus = nullptr;
};
class RtnMdInitCompletedPackage : public Package
{
public:
	RtnMdInitCompletedPackage();
	~RtnMdInitCompletedPackage();
	static RtnMdInitCompletedPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x300B;
	MdInitCompletedField* MdInitCompleted = nullptr;
};
class ReqAdminUserLoginPackage : public Package
{
public:
	ReqAdminUserLoginPackage();
	~ReqAdminUserLoginPackage();
	static ReqAdminUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4001;
	ReqAdminUserLoginField* ReqAdminUserLogin = nullptr;
};
class RspAdminUserLoginPackage : public Package
{
public:
	RspAdminUserLoginPackage();
	~RspAdminUserLoginPackage();
	static RspAdminUserLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4002;
	RspAdminUserLoginField* RspAdminUserLogin = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAdminUserLogoutPackage : public Package
{
public:
	ReqAdminUserLogoutPackage();
	~ReqAdminUserLogoutPackage();
	static ReqAdminUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4003;
	ReqAdminUserLogoutField* ReqAdminUserLogout = nullptr;
};
class RspAdminUserLogoutPackage : public Package
{
public:
	RspAdminUserLogoutPackage();
	~RspAdminUserLogoutPackage();
	static RspAdminUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4004;
	RspAdminUserLogoutField* RspAdminUserLogout = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class RtnAdminUserLogoutPackage : public Package
{
public:
	RtnAdminUserLogoutPackage();
	~RtnAdminUserLogoutPackage();
	static RtnAdminUserLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4005;
	AdminUserLogoutField* AdminUserLogout = nullptr;
};
class ReqAddRiskUserPackage : public Package
{
public:
	ReqAddRiskUserPackage();
	~ReqAddRiskUserPackage();
	static ReqAddRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4006;
	ReqAddRiskUserField* ReqAddRiskUser = nullptr;
};
class RspAddRiskUserPackage : public Package
{
public:
	RspAddRiskUserPackage();
	~RspAddRiskUserPackage();
	static RspAddRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4007;
	RspAddRiskUserField* RspAddRiskUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateRiskUserPackage : public Package
{
public:
	ReqUpdateRiskUserPackage();
	~ReqUpdateRiskUserPackage();
	static ReqUpdateRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4008;
	ReqUpdateRiskUserField* ReqUpdateRiskUser = nullptr;
};
class RspUpdateRiskUserPackage : public Package
{
public:
	RspUpdateRiskUserPackage();
	~RspUpdateRiskUserPackage();
	static RspUpdateRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4009;
	RspUpdateRiskUserField* RspUpdateRiskUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveRiskUserPackage : public Package
{
public:
	ReqRemoveRiskUserPackage();
	~ReqRemoveRiskUserPackage();
	static ReqRemoveRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x400A;
	ReqRemoveRiskUserField* ReqRemoveRiskUser = nullptr;
};
class RspRemoveRiskUserPackage : public Package
{
public:
	RspRemoveRiskUserPackage();
	~RspRemoveRiskUserPackage();
	static RspRemoveRiskUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x400B;
	RspRemoveRiskUserField* RspRemoveRiskUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddAdminUserPackage : public Package
{
public:
	ReqAddAdminUserPackage();
	~ReqAddAdminUserPackage();
	static ReqAddAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x400C;
	ReqAddAdminUserField* ReqAddAdminUser = nullptr;
};
class RspAddAdminUserPackage : public Package
{
public:
	RspAddAdminUserPackage();
	~RspAddAdminUserPackage();
	static RspAddAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x400D;
	RspAddAdminUserField* RspAddAdminUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateAdminUserPackage : public Package
{
public:
	ReqUpdateAdminUserPackage();
	~ReqUpdateAdminUserPackage();
	static ReqUpdateAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x400E;
	ReqUpdateAdminUserField* ReqUpdateAdminUser = nullptr;
};
class RspUpdateAdminUserPackage : public Package
{
public:
	RspUpdateAdminUserPackage();
	~RspUpdateAdminUserPackage();
	static RspUpdateAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x400F;
	RspUpdateAdminUserField* RspUpdateAdminUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveAdminUserPackage : public Package
{
public:
	ReqRemoveAdminUserPackage();
	~ReqRemoveAdminUserPackage();
	static ReqRemoveAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4010;
	ReqRemoveAdminUserField* ReqRemoveAdminUser = nullptr;
};
class RspRemoveAdminUserPackage : public Package
{
public:
	RspRemoveAdminUserPackage();
	~RspRemoveAdminUserPackage();
	static RspRemoveAdminUserPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4011;
	RspRemoveAdminUserField* RspRemoveAdminUser = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddPrimaryAccountPackage : public Package
{
public:
	ReqAddPrimaryAccountPackage();
	~ReqAddPrimaryAccountPackage();
	static ReqAddPrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4012;
	ReqAddPrimaryAccountField* ReqAddPrimaryAccount = nullptr;
};
class RspAddPrimaryAccountPackage : public Package
{
public:
	RspAddPrimaryAccountPackage();
	~RspAddPrimaryAccountPackage();
	static RspAddPrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4013;
	RspAddPrimaryAccountField* RspAddPrimaryAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdatePrimaryAccountPackage : public Package
{
public:
	ReqUpdatePrimaryAccountPackage();
	~ReqUpdatePrimaryAccountPackage();
	static ReqUpdatePrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4014;
	ReqUpdatePrimaryAccountField* ReqUpdatePrimaryAccount = nullptr;
};
class RspUpdatePrimaryAccountPackage : public Package
{
public:
	RspUpdatePrimaryAccountPackage();
	~RspUpdatePrimaryAccountPackage();
	static RspUpdatePrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4015;
	RspUpdatePrimaryAccountField* RspUpdatePrimaryAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemovePrimaryAccountPackage : public Package
{
public:
	ReqRemovePrimaryAccountPackage();
	~ReqRemovePrimaryAccountPackage();
	static ReqRemovePrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4016;
	ReqRemovePrimaryAccountField* ReqRemovePrimaryAccount = nullptr;
};
class RspRemovePrimaryAccountPackage : public Package
{
public:
	RspRemovePrimaryAccountPackage();
	~RspRemovePrimaryAccountPackage();
	static RspRemovePrimaryAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4017;
	RspRemovePrimaryAccountField* RspRemovePrimaryAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddAccountPackage : public Package
{
public:
	ReqAddAccountPackage();
	~ReqAddAccountPackage();
	static ReqAddAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4018;
	ReqAddAccountField* ReqAddAccount = nullptr;
};
class RspAddAccountPackage : public Package
{
public:
	RspAddAccountPackage();
	~RspAddAccountPackage();
	static RspAddAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4019;
	RspAddAccountField* RspAddAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateAccountPackage : public Package
{
public:
	ReqUpdateAccountPackage();
	~ReqUpdateAccountPackage();
	static ReqUpdateAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x401A;
	ReqUpdateAccountField* ReqUpdateAccount = nullptr;
};
class RspUpdateAccountPackage : public Package
{
public:
	RspUpdateAccountPackage();
	~RspUpdateAccountPackage();
	static RspUpdateAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x401B;
	RspUpdateAccountField* RspUpdateAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveAccountPackage : public Package
{
public:
	ReqRemoveAccountPackage();
	~ReqRemoveAccountPackage();
	static ReqRemoveAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x401C;
	ReqRemoveAccountField* ReqRemoveAccount = nullptr;
};
class RspRemoveAccountPackage : public Package
{
public:
	RspRemoveAccountPackage();
	~RspRemoveAccountPackage();
	static RspRemoveAccountPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x401D;
	RspRemoveAccountField* RspRemoveAccount = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddBaseCommissionPackage : public Package
{
public:
	ReqAddBaseCommissionPackage();
	~ReqAddBaseCommissionPackage();
	static ReqAddBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x401E;
	ReqAddBaseCommissionField* ReqAddBaseCommission = nullptr;
};
class RspAddBaseCommissionPackage : public Package
{
public:
	RspAddBaseCommissionPackage();
	~RspAddBaseCommissionPackage();
	static RspAddBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x401F;
	RspAddBaseCommissionField* RspAddBaseCommission = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateBaseCommissionPackage : public Package
{
public:
	ReqUpdateBaseCommissionPackage();
	~ReqUpdateBaseCommissionPackage();
	static ReqUpdateBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4020;
	ReqUpdateBaseCommissionField* ReqUpdateBaseCommission = nullptr;
};
class RspUpdateBaseCommissionPackage : public Package
{
public:
	RspUpdateBaseCommissionPackage();
	~RspUpdateBaseCommissionPackage();
	static RspUpdateBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4021;
	RspUpdateBaseCommissionField* RspUpdateBaseCommission = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveBaseCommissionPackage : public Package
{
public:
	ReqRemoveBaseCommissionPackage();
	~ReqRemoveBaseCommissionPackage();
	static ReqRemoveBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4022;
	ReqRemoveBaseCommissionField* ReqRemoveBaseCommission = nullptr;
};
class RspRemoveBaseCommissionPackage : public Package
{
public:
	RspRemoveBaseCommissionPackage();
	~RspRemoveBaseCommissionPackage();
	static RspRemoveBaseCommissionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4023;
	RspInfoField* RspInfo = nullptr;
	RspRemoveBaseCommissionField* RspRemoveBaseCommission = nullptr;
};
class ReqAddCommissionGroupPackage : public Package
{
public:
	ReqAddCommissionGroupPackage();
	~ReqAddCommissionGroupPackage();
	static ReqAddCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4024;
	ReqAddCommissionGroupField* ReqAddCommissionGroup = nullptr;
};
class RspAddCommissionGroupPackage : public Package
{
public:
	RspAddCommissionGroupPackage();
	~RspAddCommissionGroupPackage();
	static RspAddCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4025;
	RspAddCommissionGroupField* RspAddCommissionGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateCommissionGroupPackage : public Package
{
public:
	ReqUpdateCommissionGroupPackage();
	~ReqUpdateCommissionGroupPackage();
	static ReqUpdateCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4026;
	ReqUpdateCommissionGroupField* ReqUpdateCommissionGroup = nullptr;
};
class RspUpdateCommissionGroupPackage : public Package
{
public:
	RspUpdateCommissionGroupPackage();
	~RspUpdateCommissionGroupPackage();
	static RspUpdateCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4027;
	RspUpdateCommissionGroupField* RspUpdateCommissionGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveCommissionGroupPackage : public Package
{
public:
	ReqRemoveCommissionGroupPackage();
	~ReqRemoveCommissionGroupPackage();
	static ReqRemoveCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4028;
	ReqRemoveCommissionGroupField* ReqRemoveCommissionGroup = nullptr;
};
class RspRemoveCommissionGroupPackage : public Package
{
public:
	RspRemoveCommissionGroupPackage();
	~RspRemoveCommissionGroupPackage();
	static RspRemoveCommissionGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4029;
	RspRemoveCommissionGroupField* RspRemoveCommissionGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddOptionMarginParamPackage : public Package
{
public:
	ReqAddOptionMarginParamPackage();
	~ReqAddOptionMarginParamPackage();
	static ReqAddOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x402A;
	ReqAddOptionMarginParamField* ReqAddOptionMarginParam = nullptr;
};
class RspAddOptionMarginParamPackage : public Package
{
public:
	RspAddOptionMarginParamPackage();
	~RspAddOptionMarginParamPackage();
	static RspAddOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x402B;
	RspAddOptionMarginParamField* RspAddOptionMarginParam = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateOptionMarginParamPackage : public Package
{
public:
	ReqUpdateOptionMarginParamPackage();
	~ReqUpdateOptionMarginParamPackage();
	static ReqUpdateOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x402C;
	ReqUpdateOptionMarginParamField* ReqUpdateOptionMarginParam = nullptr;
};
class RspUpdateOptionMarginParamPackage : public Package
{
public:
	RspUpdateOptionMarginParamPackage();
	~RspUpdateOptionMarginParamPackage();
	static RspUpdateOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x402D;
	RspUpdateOptionMarginParamField* RspUpdateOptionMarginParam = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveOptionMarginParamPackage : public Package
{
public:
	ReqRemoveOptionMarginParamPackage();
	~ReqRemoveOptionMarginParamPackage();
	static ReqRemoveOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x402E;
	ReqRemoveOptionMarginParamField* ReqRemoveOptionMarginParam = nullptr;
};
class RspRemoveOptionMarginParamPackage : public Package
{
public:
	RspRemoveOptionMarginParamPackage();
	~RspRemoveOptionMarginParamPackage();
	static RspRemoveOptionMarginParamPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x402F;
	RspRemoveOptionMarginParamField* RspRemoveOptionMarginParam = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddTradeOfferPackage : public Package
{
public:
	ReqAddTradeOfferPackage();
	~ReqAddTradeOfferPackage();
	static ReqAddTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4030;
	ReqAddTradeOfferField* ReqAddTradeOffer = nullptr;
};
class RspAddTradeOfferPackage : public Package
{
public:
	RspAddTradeOfferPackage();
	~RspAddTradeOfferPackage();
	static RspAddTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4031;
	RspAddTradeOfferField* RspAddTradeOffer = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateTradeOfferPackage : public Package
{
public:
	ReqUpdateTradeOfferPackage();
	~ReqUpdateTradeOfferPackage();
	static ReqUpdateTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4032;
	ReqUpdateTradeOfferField* ReqUpdateTradeOffer = nullptr;
};
class RspUpdateTradeOfferPackage : public Package
{
public:
	RspUpdateTradeOfferPackage();
	~RspUpdateTradeOfferPackage();
	static RspUpdateTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4033;
	RspUpdateTradeOfferField* RspUpdateTradeOffer = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveTradeOfferPackage : public Package
{
public:
	ReqRemoveTradeOfferPackage();
	~ReqRemoveTradeOfferPackage();
	static ReqRemoveTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4034;
	ReqRemoveTradeOfferField* ReqRemoveTradeOffer = nullptr;
};
class RspRemoveTradeOfferPackage : public Package
{
public:
	RspRemoveTradeOfferPackage();
	~RspRemoveTradeOfferPackage();
	static RspRemoveTradeOfferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4035;
	RspRemoveTradeOfferField* RspRemoveTradeOffer = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddTradeGroupPackage : public Package
{
public:
	ReqAddTradeGroupPackage();
	~ReqAddTradeGroupPackage();
	static ReqAddTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4036;
	ReqAddTradeGroupField* ReqAddTradeGroup = nullptr;
};
class RspAddTradeGroupPackage : public Package
{
public:
	RspAddTradeGroupPackage();
	~RspAddTradeGroupPackage();
	static RspAddTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4037;
	RspAddTradeGroupField* RspAddTradeGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateTradeGroupPackage : public Package
{
public:
	ReqUpdateTradeGroupPackage();
	~ReqUpdateTradeGroupPackage();
	static ReqUpdateTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4038;
	ReqUpdateTradeGroupField* ReqUpdateTradeGroup = nullptr;
};
class RspUpdateTradeGroupPackage : public Package
{
public:
	RspUpdateTradeGroupPackage();
	~RspUpdateTradeGroupPackage();
	static RspUpdateTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4039;
	RspUpdateTradeGroupField* RspUpdateTradeGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveTradeGroupPackage : public Package
{
public:
	ReqRemoveTradeGroupPackage();
	~ReqRemoveTradeGroupPackage();
	static ReqRemoveTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x403A;
	ReqRemoveTradeGroupField* ReqRemoveTradeGroup = nullptr;
};
class RspRemoveTradeGroupPackage : public Package
{
public:
	RspRemoveTradeGroupPackage();
	~RspRemoveTradeGroupPackage();
	static RspRemoveTradeGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x403B;
	RspRemoveTradeGroupField* RspRemoveTradeGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddTradeGroupItemPackage : public Package
{
public:
	ReqAddTradeGroupItemPackage();
	~ReqAddTradeGroupItemPackage();
	static ReqAddTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x403C;
	ReqAddTradeGroupItemField* ReqAddTradeGroupItem = nullptr;
};
class RspAddTradeGroupItemPackage : public Package
{
public:
	RspAddTradeGroupItemPackage();
	~RspAddTradeGroupItemPackage();
	static RspAddTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x403D;
	RspAddTradeGroupItemField* RspAddTradeGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateTradeGroupItemPackage : public Package
{
public:
	ReqUpdateTradeGroupItemPackage();
	~ReqUpdateTradeGroupItemPackage();
	static ReqUpdateTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x403E;
	ReqUpdateTradeGroupItemField* ReqUpdateTradeGroupItem = nullptr;
};
class RspUpdateTradeGroupItemPackage : public Package
{
public:
	RspUpdateTradeGroupItemPackage();
	~RspUpdateTradeGroupItemPackage();
	static RspUpdateTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x403F;
	RspUpdateTradeGroupItemField* RspUpdateTradeGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveTradeGroupItemPackage : public Package
{
public:
	ReqRemoveTradeGroupItemPackage();
	~ReqRemoveTradeGroupItemPackage();
	static ReqRemoveTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4040;
	ReqRemoveTradeGroupItemField* ReqRemoveTradeGroupItem = nullptr;
};
class RspRemoveTradeGroupItemPackage : public Package
{
public:
	RspRemoveTradeGroupItemPackage();
	~RspRemoveTradeGroupItemPackage();
	static RspRemoveTradeGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4041;
	RspRemoveTradeGroupItemField* RspRemoveTradeGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddRiskGroupPackage : public Package
{
public:
	ReqAddRiskGroupPackage();
	~ReqAddRiskGroupPackage();
	static ReqAddRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4042;
	ReqAddRiskGroupField* ReqAddRiskGroup = nullptr;
};
class RspAddRiskGroupPackage : public Package
{
public:
	RspAddRiskGroupPackage();
	~RspAddRiskGroupPackage();
	static RspAddRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4043;
	RspAddRiskGroupField* RspAddRiskGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateRiskGroupPackage : public Package
{
public:
	ReqUpdateRiskGroupPackage();
	~ReqUpdateRiskGroupPackage();
	static ReqUpdateRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4044;
	ReqUpdateRiskGroupField* ReqUpdateRiskGroup = nullptr;
};
class RspUpdateRiskGroupPackage : public Package
{
public:
	RspUpdateRiskGroupPackage();
	~RspUpdateRiskGroupPackage();
	static RspUpdateRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4045;
	RspUpdateRiskGroupField* RspUpdateRiskGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveRiskGroupPackage : public Package
{
public:
	ReqRemoveRiskGroupPackage();
	~ReqRemoveRiskGroupPackage();
	static ReqRemoveRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4046;
	ReqRemoveRiskGroupField* ReqRemoveRiskGroup = nullptr;
};
class RspRemoveRiskGroupPackage : public Package
{
public:
	RspRemoveRiskGroupPackage();
	~RspRemoveRiskGroupPackage();
	static RspRemoveRiskGroupPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4047;
	RspRemoveRiskGroupField* RspRemoveRiskGroup = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddRiskGroupItemPackage : public Package
{
public:
	ReqAddRiskGroupItemPackage();
	~ReqAddRiskGroupItemPackage();
	static ReqAddRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4048;
	ReqAddRiskGroupItemField* ReqAddRiskGroupItem = nullptr;
};
class RspAddRiskGroupItemPackage : public Package
{
public:
	RspAddRiskGroupItemPackage();
	~RspAddRiskGroupItemPackage();
	static RspAddRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4049;
	RspAddRiskGroupItemField* RspAddRiskGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqUpdateRiskGroupItemPackage : public Package
{
public:
	ReqUpdateRiskGroupItemPackage();
	~ReqUpdateRiskGroupItemPackage();
	static ReqUpdateRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x404A;
	ReqUpdateRiskGroupItemField* ReqUpdateRiskGroupItem = nullptr;
};
class RspUpdateRiskGroupItemPackage : public Package
{
public:
	RspUpdateRiskGroupItemPackage();
	~RspUpdateRiskGroupItemPackage();
	static RspUpdateRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x404B;
	RspUpdateRiskGroupItemField* RspUpdateRiskGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveRiskGroupItemPackage : public Package
{
public:
	ReqRemoveRiskGroupItemPackage();
	~ReqRemoveRiskGroupItemPackage();
	static ReqRemoveRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x404C;
	ReqRemoveRiskGroupItemField* ReqRemoveRiskGroupItem = nullptr;
};
class RspRemoveRiskGroupItemPackage : public Package
{
public:
	RspRemoveRiskGroupItemPackage();
	~RspRemoveRiskGroupItemPackage();
	static RspRemoveRiskGroupItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x404D;
	RspRemoveRiskGroupItemField* RspRemoveRiskGroupItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddOrUpdateRiskPackage : public Package
{
public:
	ReqAddOrUpdateRiskPackage();
	~ReqAddOrUpdateRiskPackage();
	static ReqAddOrUpdateRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x404E;
	ReqAddOrUpdateRiskField* ReqAddOrUpdateRisk = nullptr;
};
class RspAddOrUpdateRiskPackage : public Package
{
public:
	RspAddOrUpdateRiskPackage();
	~RspAddOrUpdateRiskPackage();
	static RspAddOrUpdateRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x404F;
	RspAddOrUpdateRiskField* RspAddOrUpdateRisk = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveRiskPackage : public Package
{
public:
	ReqRemoveRiskPackage();
	~ReqRemoveRiskPackage();
	static ReqRemoveRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4050;
	ReqRemoveRiskField* ReqRemoveRisk = nullptr;
};
class RspRemoveRiskPackage : public Package
{
public:
	RspRemoveRiskPackage();
	~RspRemoveRiskPackage();
	static RspRemoveRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4051;
	RspRemoveRiskField* RspRemoveRisk = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddRiskRulePackage : public Package
{
public:
	ReqAddRiskRulePackage();
	~ReqAddRiskRulePackage();
	static ReqAddRiskRulePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4052;
	ReqAddRiskRuleField* ReqAddRiskRule = nullptr;
};
class RspAddRiskRulePackage : public Package
{
public:
	RspAddRiskRulePackage();
	~RspAddRiskRulePackage();
	static RspAddRiskRulePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4053;
	RspAddRiskRuleField* RspAddRiskRule = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddRiskRuleItemPackage : public Package
{
public:
	ReqAddRiskRuleItemPackage();
	~ReqAddRiskRuleItemPackage();
	static ReqAddRiskRuleItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4054;
	ReqAddRiskRuleItemField* ReqAddRiskRuleItem = nullptr;
};
class RspAddRiskRuleItemPackage : public Package
{
public:
	RspAddRiskRuleItemPackage();
	~RspAddRiskRuleItemPackage();
	static RspAddRiskRuleItemPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4055;
	RspAddRiskRuleItemField* RspAddRiskRuleItem = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddRiskTradeScopePackage : public Package
{
public:
	ReqAddRiskTradeScopePackage();
	~ReqAddRiskTradeScopePackage();
	static ReqAddRiskTradeScopePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4056;
	ReqAddRiskTradeScopeField* ReqAddRiskTradeScope = nullptr;
};
class RspAddRiskTradeScopePackage : public Package
{
public:
	RspAddRiskTradeScopePackage();
	~RspAddRiskTradeScopePackage();
	static RspAddRiskTradeScopePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4057;
	RspAddRiskTradeScopeField* RspAddRiskTradeScope = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAddAccountRiskPackage : public Package
{
public:
	ReqAddAccountRiskPackage();
	~ReqAddAccountRiskPackage();
	static ReqAddAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4058;
	ReqAddAccountRiskField* ReqAddAccountRisk = nullptr;
};
class RspAddAccountRiskPackage : public Package
{
public:
	RspAddAccountRiskPackage();
	~RspAddAccountRiskPackage();
	static RspAddAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x4059;
	RspAddAccountRiskField* RspAddAccountRisk = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqRemoveAccountRiskPackage : public Package
{
public:
	ReqRemoveAccountRiskPackage();
	~ReqRemoveAccountRiskPackage();
	static ReqRemoveAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x405A;
	ReqRemoveAccountRiskField* ReqRemoveAccountRisk = nullptr;
};
class RspRemoveAccountRiskPackage : public Package
{
public:
	RspRemoveAccountRiskPackage();
	~RspRemoveAccountRiskPackage();
	static RspRemoveAccountRiskPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x405B;
	RspRemoveAccountRiskField* RspRemoveAccountRisk = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqMoneyTransferPackage : public Package
{
public:
	ReqMoneyTransferPackage();
	~ReqMoneyTransferPackage();
	static ReqMoneyTransferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x405C;
	ReqMoneyTransferField* ReqMoneyTransfer = nullptr;
};
class RspMoneyTransferPackage : public Package
{
public:
	RspMoneyTransferPackage();
	~RspMoneyTransferPackage();
	static RspMoneyTransferPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x405D;
	RspMoneyTransferField* RspMoneyTransfer = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqAuditOrderPackage : public Package
{
public:
	ReqAuditOrderPackage();
	~ReqAuditOrderPackage();
	static ReqAuditOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x405E;
	ReqAuditOrderField* ReqAuditOrder = nullptr;
};
class RspAuditOrderPackage : public Package
{
public:
	RspAuditOrderPackage();
	~RspAuditOrderPackage();
	static RspAuditOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x405F;
	RspAuditOrderField* RspAuditOrder = nullptr;
	RspInfoField* RspInfo = nullptr;
};
class ReqOfferLoginPackage : public Package
{
public:
	ReqOfferLoginPackage();
	~ReqOfferLoginPackage();
	static ReqOfferLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5001;
	ReqOfferLoginField* ReqOfferLogin = nullptr;
};
class RspOfferLoginPackage : public Package
{
public:
	RspOfferLoginPackage();
	~RspOfferLoginPackage();
	static RspOfferLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5002;
	RspOfferLoginField* RspOfferLogin = nullptr;
};
class ReqPrimaryAccountLoginPackage : public Package
{
public:
	ReqPrimaryAccountLoginPackage();
	~ReqPrimaryAccountLoginPackage();
	static ReqPrimaryAccountLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5003;
	ReqPrimaryAccountLoginField* ReqPrimaryAccountLogin = nullptr;
};
class RspPrimaryAccountLoginPackage : public Package
{
public:
	RspPrimaryAccountLoginPackage();
	~RspPrimaryAccountLoginPackage();
	static RspPrimaryAccountLoginPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5004;
	RspPrimaryAccountLoginField* RspPrimaryAccountLogin = nullptr;
};
class ReqPrimaryAccountLogoutPackage : public Package
{
public:
	ReqPrimaryAccountLogoutPackage();
	~ReqPrimaryAccountLogoutPackage();
	static ReqPrimaryAccountLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5005;
	ReqPrimaryAccountLogoutField* ReqPrimaryAccountLogout = nullptr;
};
class RtnPrimaryAccountLogoutPackage : public Package
{
public:
	RtnPrimaryAccountLogoutPackage();
	~RtnPrimaryAccountLogoutPackage();
	static RtnPrimaryAccountLogoutPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5006;
	RtnPrimaryAccountLogoutField* RtnPrimaryAccountLogout = nullptr;
};
class ReqPrimaryAccountInitPackage : public Package
{
public:
	ReqPrimaryAccountInitPackage();
	~ReqPrimaryAccountInitPackage();
	static ReqPrimaryAccountInitPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5007;
	ReqPrimaryAccountInitField* ReqPrimaryAccountInit = nullptr;
};
class RspPrimaryAccountInitPackage : public Package
{
public:
	RspPrimaryAccountInitPackage();
	~RspPrimaryAccountInitPackage();
	static RspPrimaryAccountInitPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5008;
	RspPrimaryAccountInitField* RspPrimaryAccountInit = nullptr;
};
class ReqPrimaryAccountQueryPackage : public Package
{
public:
	ReqPrimaryAccountQueryPackage();
	~ReqPrimaryAccountQueryPackage();
	static ReqPrimaryAccountQueryPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5009;
	ReqPrimaryAccountQueryField* ReqPrimaryAccountQuery = nullptr;
};
class RspPrimaryAccountQueryPackage : public Package
{
public:
	RspPrimaryAccountQueryPackage();
	~RspPrimaryAccountQueryPackage();
	static RspPrimaryAccountQueryPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x500A;
	RspPrimaryAccountQueryField* RspPrimaryAccountQuery = nullptr;
};
class ReqQryOfferOptionInstrumentPackage : public Package
{
public:
	ReqQryOfferOptionInstrumentPackage();
	~ReqQryOfferOptionInstrumentPackage();
	static ReqQryOfferOptionInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x500B;
	ReqQryOfferOptionInstrumentField* ReqQryOfferOptionInstrument = nullptr;
};
class RspQryOfferOptionInstrumentPackage : public Package
{
public:
	RspQryOfferOptionInstrumentPackage();
	~RspQryOfferOptionInstrumentPackage();
	static RspQryOfferOptionInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x500C;
	RspQryOfferOptionInstrumentField* RspQryOfferOptionInstrument = nullptr;
};
class RtnOfferOptionInstrumentPackage : public Package
{
public:
	RtnOfferOptionInstrumentPackage();
	~RtnOfferOptionInstrumentPackage();
	static RtnOfferOptionInstrumentPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x500D;
	OfferOptionInstrumentField* OfferOptionInstrument = nullptr;
};
class ReqOfferOrderPackage : public Package
{
public:
	ReqOfferOrderPackage();
	~ReqOfferOrderPackage();
	static ReqOfferOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x500E;
	ReqOfferOrderField* ReqOfferOrder = nullptr;
};
class ReqOfferCancelOrderPackage : public Package
{
public:
	ReqOfferCancelOrderPackage();
	~ReqOfferCancelOrderPackage();
	static ReqOfferCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x500F;
	ReqOfferCancelOrderField* ReqOfferCancelOrder = nullptr;
};
class RtnOfferOrderPackage : public Package
{
public:
	RtnOfferOrderPackage();
	~RtnOfferOrderPackage();
	static RtnOfferOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5010;
	OfferOrderField* OfferOrder = nullptr;
};
class RtnOfferTradePackage : public Package
{
public:
	RtnOfferTradePackage();
	~RtnOfferTradePackage();
	static RtnOfferTradePackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5011;
	OfferTradeField* OfferTrade = nullptr;
};
class RtnOfferErrorCancelOrderPackage : public Package
{
public:
	RtnOfferErrorCancelOrderPackage();
	~RtnOfferErrorCancelOrderPackage();
	static RtnOfferErrorCancelOrderPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5012;
	OfferErrorCancelOrderField* OfferErrorCancelOrder = nullptr;
};
class RtnOfferCapitalPackage : public Package
{
public:
	RtnOfferCapitalPackage();
	~RtnOfferCapitalPackage();
	static RtnOfferCapitalPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5013;
	OfferCapitalField* OfferCapital = nullptr;
};
class RtnOfferPositionPackage : public Package
{
public:
	RtnOfferPositionPackage();
	~RtnOfferPositionPackage();
	static RtnOfferPositionPackage* Allocate();
	virtual void Deallocate() override;
	virtual void Prepare(SessionIDType sessionID, int messageChain, int msgSeqNum) override;
	virtual int ToStepStream(char* buff, int size) const override;
	virtual bool FromStepStream(char* buff, int startIndex, int endIndex) override;
	virtual int ToXtpStream(char* buff, int size) const override;
	virtual bool FromXtpStream(char* buff, int startIndex, int endIndex) override;
	virtual const char* GetDebugString() const override;
public:
	static constexpr UShortType PackageID = 0x5014;
	OfferPositionField* OfferPosition = nullptr;
};
}
