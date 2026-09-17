#pragma once
#include <functional>
#include <string>

namespace Spark::Core
{
template <typename Func, typename... Args> struct AspectAfter
{
    AspectAfter(Func& f, const std::string& funcName) : func_(std::forward<Func>(f)), funcName_(funcName) {}

    template <typename T> void InvokeAfter(Args&&... args, T&& aspect)
    {
        func_(std::forward<Args>(args)...);
        aspect.After(funcName_.c_str());
    }

    template <typename T, typename... AP> void InvokeAfter(Args&&... args, T&& aspectAfter, AP&&... aspectAfters)
    {
        InvokeAfter(std::forward<Args>(args)..., AP()...);
        aspectAfter.After(funcName_.c_str());
    }

private:
    Func func_;
    std::string funcName_;
};

template <typename... AP, typename... Args, typename Func> void InvokeAfter(Func&& f, const std::string& funcName, Args&&... args)
{
    AspectAfter<Func, Args...> asp(std::forward<Func>(f), funcName);
    asp.InvokeAfter(std::forward<Args>(args)..., AP()...);
}
}
