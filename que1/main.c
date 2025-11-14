#include <stdio.h>
#include "mathlib.h"

void main()
{
    // MENU
    printf("1. Add two numbers\n");
    printf("2. Subtract two numbers\n");
    printf("3. Multiply two numbers\n");
    printf("4. Divide two numbers\n");

    int option;
    printf("\n\nSelect a valid Option : ");
    scanf("%d", &option);

    if(!(option<=4 && option >=1))
    {
        printf("Invalid choice\n");
        return;
    }

    float x, y;
    double result;
    int isError = 0;

    printf("Enter 1st Number : ");
    scanf("%f", &x);
    printf("Enter 2nd Number : ");
    scanf("%f", &y);

    switch(option)
    {
        case 1:
            result = add(x, y); break;
        case 2:
            result = subtract(x, y); break;
        case 3:
            result = multiply(x, y);break;
        case 4:
            if (y == 0)
            {
                printf("Cannot Divide by Zero\n");
                isError = 1;
            }
            result = divide(x, y);
    }
    if (!isError)
        printf("%.2f\n", result);
}