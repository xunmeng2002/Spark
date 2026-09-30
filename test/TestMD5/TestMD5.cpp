#include <Spark/Core/MD5/MD5.h>
#include <iostream>
using namespace std;

using namespace Spark::Core;

int main()
{
    string src = "HelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHelloWorld";

    string s = getMD5(reinterpret_cast<const unsigned char*>(src.c_str()), static_cast<int>(src.length()));
    cout << s;
    return 0;
}
