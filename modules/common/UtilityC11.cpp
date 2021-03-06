// UtilityC11.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

//#include "pch.h"
#include <iostream>
#include <string>
#include "UtilityToolsC11.h"
#include <functional>


struct MyStruct
{
    MyStruct(const std::string &strValue)
    {
        printf("strvalue is %s\r\n", strValue.c_str());
    }

    MyStruct(std::string &&strValue)
    {
        printf("right strvalue is %s\r\n", strValue.c_str());
    }

    MyStruct(int i, int k)
    {
        printf("i,k is %d, %d\r\n", i, k);
    }
};

typedef std::function<void(int, int)> CallFunc;

struct Caller
{
    void func(int a, int b)
    {
        printf("a is %d and b is %d\r\n", a, b);
    }
};

void g_func(int a, int b)
{
    printf("global a is %d and b is %d\r\n", a, b);
}

int main()
{
    std::string strValue = "test";

    auto p1 = SingletonC11<MyStruct>::CreateInstance(strValue);
    SingletonC11<MyStruct>::DestroyInstance();

    auto p2 = SingletonC11<MyStruct>::CreateInstance(std::move(strValue));
    SingletonC11<MyStruct>::DestroyInstance();

    auto p3 = SingletonC11<MyStruct>::CreateInstance(10, 20);
    SingletonC11<MyStruct>::DestroyInstance();

    Events<CallFunc> events;

    Caller cer;
    auto key1 = events.Register(std::bind(&Caller::func, &cer, std::placeholders::_1, std::placeholders::_2));
    auto key2 = events.Register(g_func);

    auto fn = [&](int a, int b) -> void
    {
        printf("lamda a is %d and b is %d\r\n", a, b);
    };
    auto key3 = events.Register(fn);

    events.Notify(1, 2);

    events.UnRegister(key1);
    events.UnRegister(key2);
    events.UnRegister(key3);

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
