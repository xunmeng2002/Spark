#include <Spark/Core/MD5/MD5.h>
#include <iostream>
using namespace std;

using namespace spark::core;

int main()
{
    string src = "HelloWorldHelloWorldHelloWorldHelloWorldHelloWorldHelloWorld";

    string s = getMD5((const unsigned char*)src.c_str(), (int)src.length());
    cout << s;
    return 0;
}
