#include <stdio.h>
#include "factorial.h"

void main()
{
    int a;
    long long result;
    printf("Enter Natural Number of which factorial is calculated : ");
    scanf("%d", &a);

    result = a<0?-1:fact(a);

    printf(result<0? "Negative number not possible\n" : "Ans : %lld\n",result);
}