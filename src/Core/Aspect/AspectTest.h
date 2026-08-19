#pragma once
#include <iostream>


namespace spark::core
{
struct AA
{
    void Before(const char* funcName) { std::cout << funcName << ": AA Before" << std::endl; }
    void After(const char* funcName) { std::cout << funcName << ": AA After" << std::endl; }
};
struct BB
{
    void Before(const char* funcName) { std::cout << funcName << ": BB Before" << std::endl; }
    void After(const char* funcName) { std::cout << funcName << ": BB After" << std::endl; }
};
struct CC
{
    void Before(const char* funcName) { std::cout << funcName << ": CC Before" << std::endl; }
    void After(const char* funcName) { std::cout << funcName << ": CC After" << std::endl; }
};


void CoreFunc();
void TestAspect();
}
