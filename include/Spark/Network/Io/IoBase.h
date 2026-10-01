#pragma once
#include <Spark/Network/NetworkExport.h>
#include <Spark/Types.h>
#include <Spark/TemplateLib/Buffer/LinearBuffer.h>
#include <Spark/Network/Io/Connect.h>
#include <Spark/Network/Io/IoUtility.h>
#include <atomic>
#include <string>
#include <chrono>
#include <cstddef>
#include <mutex>
#include <list>
#include <map>
#include <condition_variable>

namespace Spark::Network
{
class IoSubscriber
{
public:
    virtual void OnConnect(SessionIdType sessionId, const char* ip, int port) = 0;
    virtual void OnDisConnect(SessionIdType sessionId, const char* ip, int port) = 0;
    // 收包缓冲由 IO 层持有、仅在本次回调期间有效：订阅者不得归还、不得留存该指针，
    // 需要留存请当场拷贝。data 不保证以 NUL 结尾，有效字节数以 length 为准。
    virtual void OnRecv(SessionIdType sessionId, const char* data, size_t length) = 0;
};

class NETWORK_EXPORTS IoBase
{
public:
    IoBase(ServerTypeType serverType, const char* addressName, int milliSeconds);
    virtual ~IoBase();
    void Subscribe(IoSubscriber* subscriber);
    void UnSubscribe();
    virtual void SetTimeOut(int milliSeconds);

    virtual bool Init() { return true; }
    virtual bool ConnectToServer(const char* addressName) { return false; }
    virtual void DisConnect(SessionIdType sessionId);
    virtual void DisConnectAll();
    virtual void Send(SessionIdType sessionId, LinearBuffer<BufferSize>* buffer) = 0;
    // 发送缓冲须由此取得：缓冲的归还在 IO 层内部完成，而 ObjectPool 实例不跨模块共享，
    // 应用侧自行 Allocate 得到的对象会被归还进一个非属主的池。
    LinearBuffer<BufferSize>* AllocateSendBuffer();

    virtual void HandleIoEvent() = 0;

protected:
    // 会话号低位段的容量：取号式为「毫秒时间戳 × 本值 ＋ 当毫秒内序号」，序号在此处回绕。
    // 同一 IoBase 在一个毫秒内取号超过本值即产生重复会话号，重复者被 AddConnect 拒绝。
    static constexpr SessionIdType SessionIdSequencePerMillisecond = 100LL;

    virtual void DoDisConnect();
    virtual void DoSend(Connect* connect) = 0;
    virtual void DoRecv(Connect* connect) = 0;

    // 受理则登记并通知订阅者、返回 true；会话号已被占用则返回 false，且不触碰 connect
    // （不登记、不归还、不关连接）——收场由调用方按各 IO 模型自行完成。
    virtual bool AddConnect(Connect* connect);
    virtual void RemoveConnect(Connect* connect);
    // 归还一条未被受理的连接。与 RemoveConnect 的区别只有一点，但很关键：不触碰 connects_。
    // 被拒连接的会话号正是在册那条的号，按号 erase 抹掉的是在册的会话。
    void DiscardRefusedConnect(Connect* connect);
    virtual Connect* GetConnect(SessionIdType sessionId);

    void NotifySubscriberRecvSafely(const Connect* connect, const char* data, size_t length);

    SessionIdType GetSessionId();

    ServerTypeType serverType_;
    std::string addressName_;
    std::string address_;
    std::string port_;
    std::chrono::milliseconds timeOut_;
    IoSubscriber* ioSubscriber_;
    // 应用线程与 IO 线程都会经 ConnectToServer 取号，非原子的读-改-写会让两条连接取到同一个号。
    std::atomic<SessionIdType> lastSessionIndex_;

    std::map<SessionIdType, Connect*> connects_;
    std::mutex connectsMutex_;
    std::list<SessionIdType> disConnectSessionIds_;
    std::mutex disConnectSessionIdsMutex_;

    std::mutex mutex_;
};
}
