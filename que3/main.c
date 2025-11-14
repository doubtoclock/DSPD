#include <stdio.h>
#include "funcs.h"

void main()
{
    // MENU
    printf("1. Calculate Power\n");
    printf("2. Calculate Factorial\n");
    printf("3. Calculate GCD of Two Numbers\n");

    int option;
    printf("\n\nSelect a valid Option : ");
    scanf("%d", &option);

    if (option == 1)
    {
        int b, p;
        printf("Enter Base (> 0) : ");
        scanf("%d", &b);
        printf("Enter Power (>= 0) : ");
        scanf("%d", &p);

        if (b == 0 && p == 0)
        {
            printf("Cannot rise base 0 to power 0 (undefined)");
        }
        else if (b >= 0 && p >= 0)
        {
            long result = power(b, p);
            printf("Ans : %d", result);
        }
        else
        {
            printf("Base & Power must be positive.");
        }
    }
    else if (option == 2)
    {
        int a;
        printf("Enter Natural Number : ");
        scanf("%d", &a);

        if (a < 0)
        {
            printf("Number must be positive.");
        }
        else
        {
            long result = factorial(a);
            printf("Ans : %d", result);
        }
    }
    else if (option == 3)
    {
        int b, p;
        printf("Enter 1st No (> 0) : ");
        scanf("%d", &b);
        printf("Enter 2nd No (> 0) : ");
        scanf("%d", &p);

        if (b >= 0 && p >= 0)
        {
            long result = gcd(b, p);
            printf("Ans : %d", result);
        }
        else
        {
            printf("Numbers must be positive.");
        }
    }
    else
    {
        printf("Not a valid option.");
    }
}