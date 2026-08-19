#include "Aspect/AspectTest.h"
#include <Spark/Core/Aspect/Aspect.h>
#include <Spark/Core/Aspect/AspectBefore.h>
#include <Spark/Core/Aspect/AspectAfter.h>
#include <iostream>

namespace spark::core
{
void CoreFunc()
{
    std::cout << "core function called" << std::endl;
}
void TestAspect()
{
    Invoke<AA>(CoreFunc, "core_func");
    Invoke<AA, BB, CC>(CoreFunc, "core_func");
    //InvokeBefore<AA, BB, CC>(CoreFunc, "core_func");
    //InvokeAfter<AA, BB, CC>(CoreFunc, "core_func");
}
}
