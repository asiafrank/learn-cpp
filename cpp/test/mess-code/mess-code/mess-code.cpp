// mess-code.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <string>

void ByValue(int a)
{
    a = a + 1;
}

void ByRef(int& a)
{
    a = a + 1;
}

void ByPointer(int* a)
{
    *a = *a + 1;
    printf("%d\n", &a);
}

typedef struct x 
{
    char a;
    double b;
};

template <typename T> auto fref(const T& a, const T& b)->decltype(a[3])
{
    return a[3];
}

int main()
{
    int v = 1;
    int *vp = &v;
    printf("%d\n", &vp);
    ByPointer(vp);
    printf("%d\n", v);
    printf("%d\n", &vp);

    printf("%d\n", sizeof(x));
    printf("%d\n", sizeof(char));
    printf("%d\n", sizeof(double));

    int a[10], b[10];
    for (size_t i = 0; i < 10; i++)
    {
        a[i] = i;
    }

    std::cout << fref(a, b) << std::endl;

    std::cout << "-----dddddddddddddd" << std::endl;
    {
        char x = '中'; // Windows 平台 cpp 使用 UTF-16 编码，中文一个字符按两个字节存储，被截取为单个字节
        std::cout << x << std::endl;

        std::string s = "中"; // Windows 平台 cpp 使用 UTF-16 编码，占 2 个字节
        std::cout << "---" << s << " size " << s.size() << std::endl;
    }
    return 0;
}
