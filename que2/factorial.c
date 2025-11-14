#include "factorial.h"

long long fact(int num)
{
    long long ans = 1;
    if (num > 1)
    {
        for (int i = 2; i <= num; i++)
        {
            ans = ans * i;
        }
    }
    return ans;
}