#pragma once
#include <functional>
#include <string>


namespace spark::core
{
template <typename Func, typename... Args>
struct AspectBefore
{
    AspectBefore(Func& f, const std::string& funcName)
        : m_Func(std::forward<Func>(f)), m_FuncName(funcName)
    {
    }

    template <typename T>
    void InvokeBefore(Args&&... args, T&& aspect)
    {
        aspect.Before(m_FuncName.c_str());
        m_Func(std::forward<Args>(args)...);
    }

    template <typename T, typename... AP>
    void InvokeBefore(Args&&... args, T&& aspectBefore, AP&&... aspectBefores)
    {
        aspectBefore.Before(m_FuncName.c_str());
        InvokeBefore(std::forward<Args>(args)..., AP()...);
    }

private:
    Func m_Func;
    std::string m_FuncName;
};


template <typename... AP, typename... Args, typename Func>
void InvokeBefore(Func&& f, const std::string& funcName, Args&&... args)
{
    AspectBefore<Func, Args...> asp(std::forward<Func>(f), funcName);
    asp.InvokeBefore(std::forward<Args>(args)..., AP()...);
}
}

