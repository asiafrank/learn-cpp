// mess-code.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

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

int main()
{
    int v = 1;
    int *vp = &v;
    printf("%d\n", &vp);
    ByPointer(vp);
    printf("%d\n", v);
    printf("%d\n", &vp);
    return 0;
}
