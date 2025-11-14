#include <stdio.h>
#include <stdlib.h>
#include "conversion.h"

void main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Write: %s <value in meters> <unit>\n", argv[0]);
        printf("UNITS : km, mm, cm, in\n");
        return;
    }

    float meters = atof(argv[1]);
    float result = convert(meters, argv[2]);

    if (result == -1)
    {
        printf("Invaild unit!");
    }
    else
    {
        printf("\n%f meters = %f %s\n", meters, result, argv[2]);
    }
}