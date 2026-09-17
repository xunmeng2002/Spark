#pragma once
#include <functional>
#include <string>

namespace Spark::Core
{
template <typename Func, typename... Args>
struct AspectBefore
{
    AspectBefore(Func& f, const std::string& funcName) : func_(std::forward<Func>(f)), funcName_(funcName) {}

    template <typename T>
    void InvokeBefore(Args&&... args, T&& aspect)
    {
        aspect.Before(funcName_.c_str());
        func_(std::forward<Args>(args)...);
    }

    template <typename T, typename... AP>
    void InvokeBefore(Args&&... args, T&& aspectBefore, AP&&... aspectBefores)
    {
        aspectBefore.Before(funcName_.c_str());
        InvokeBefore(std::forward<Args>(args)..., AP()...);
    }

private:
    Func func_;
    std::string funcName_;
};

template <typename... AP, typename... Args, typename Func>
void InvokeBefore(Func&& f, const std::string& funcName, Args&&... args)
{
    AspectBefore<Func, Args...> asp(std::forward<Func>(f), funcName);
    asp.InvokeBefore(std::forward<Args>(args)..., AP()...);
}
}
