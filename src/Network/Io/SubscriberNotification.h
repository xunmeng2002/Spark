#pragma once
#include <Spark/Core/Logger/Logger.h>
#include <Spark/Types.h>

#include <exception>

namespace Spark::Network
{
template <typename InvokeSubscriberNotification>
void NotifySubscriberSafely(const char* notificationName, SessionIdType sessionId, InvokeSubscriberNotification&& invokeSubscriberNotification)
{
    try
    {
        invokeSubscriberNotification();
    }
    catch (const std::exception& notificationFailure)
    {
        WriteLog(Spark::Core::LogLevel::Error, "Subscriber %s Threw. SessionId:%lld, Reason:%s", notificationName, sessionId,
                 notificationFailure.what());
    }
    catch (...)
    {
        WriteLog(Spark::Core::LogLevel::Error, "Subscriber %s Threw An Unknown Exception. SessionId:%lld", notificationName, sessionId);
    }
}
}
