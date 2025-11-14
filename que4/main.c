#include <stdio.h>
#include <stdlib.h>
#include "conversion.h"

void main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage- %s <value in meters> <unit>\n", argv[0]);
        printf("units: km,mm,cm\n");
        return;
    }

    float meters = atof(argv[1]);
    float result = convert(meters, argv[2]);

    printf(result==-1? "\nInvalid\n": "\n%f meters = %f %s\n", meters, result, argv[2]);
}