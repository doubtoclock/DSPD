#include "conversion.h"
#include <stdio.h>
#include <string.h>

float convert(float meter, char unit[])
{
    if (strcmp(unit, "cm") == 0)
    {
        return meter * 100;
    }
    else if (strcmp(unit, "mm") == 0)
    {
        return meter * 1000;
    }

    else if (strcmp(unit, "km") == 0)
    {
        return meter / 1000;
    }
    else{
        return -1;
    }
}
