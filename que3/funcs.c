#include "funcs.h"

long power(int base, int power)
{
    long ans = 1;
    for (int i = 0; i < power; i++)
    {
        ans = ans * base;
    }
    return ans;
}

long factorial(int num)
{
    long ans = 1;
    if (num > 1)
    {
        for (int i = 2; i <= num; i++)
        {
            ans = ans * i;
        }
    }
    return ans;
}

int gcd(int a, int b)
{
    int remainder;
    do
    {
        remainder = a % b;
        a = b;
        b = remainder;
    } while (remainder != 0);
    return a;
}