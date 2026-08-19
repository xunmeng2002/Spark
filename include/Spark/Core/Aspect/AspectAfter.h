#pragma once
#include <functional>
#include <string>

namespace spark::core
{
template <typename Func, typename... Args>
struct AspectAfter
{
    AspectAfter(Func& f, const std::string& funcName)
        : m_Func(std::forward<Func>(f)), m_FuncName(funcName)
    {
    }

    template <typename T>
    void InvokeAfter(Args&&... args, T&& aspect)
    {
        m_Func(std::forward<Args>(args)...);
        aspect.After(m_FuncName.c_str());
    }

    template <typename T, typename... AP>
    void InvokeAfter(Args&&... args, T&& aspectAfter, AP&&... aspectAfters)
    {
        InvokeAfter(std::forward<Args>(args)..., AP()...);
        aspectAfter.After(m_FuncName.c_str());
    }

private:
    Func m_Func;
    std::string m_FuncName;
};


template <typename... AP, typename... Args, typename Func>
void InvokeAfter(Func&& f, const std::string& funcName, Args&&... args)
{
    AspectAfter<Func, Args...> asp(std::forward<Func>(f), funcName);
    asp.InvokeAfter(std::forward<Args>(args)..., AP()...);
}
}
