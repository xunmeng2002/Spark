#pragma once
#include <functional>
#include <string>

namespace Spark::Core
{
template <typename Func, typename... Args>
struct Aspect
{
    Aspect(Func&& f, const std::string& funcName) : func_(std::forward<Func>(f)), funcName_(funcName) {}

    void Invoke(Args&&... args) { func_(std::forward<Args>(args)...); }
    template <typename T, typename... AP>
    void Invoke(Args&&... args, T&& aspect, AP&&... aspects)
    {
        aspect.Before(funcName_.c_str());
        Invoke(std::forward<Args>(args)..., std::forward<AP>(aspects)...);
        aspect.After(funcName_.c_str());
    }

private:
    Func func_;
    std::string funcName_;
};

template <typename... AP, typename... Args, typename Func>
void Invoke(Func&& f, const std::string& funcName, Args&&... args)
{
    Aspect<Func, Args...> asp(std::forward<Func>(f), funcName);
    asp.Invoke(std::forward<Args>(args)..., AP()...);
}
}
