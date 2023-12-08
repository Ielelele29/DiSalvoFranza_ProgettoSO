//
// Created by lelelele29 on 08/12/23.
//

#include <stdlib.h>
#include <math.h>
#include "NumberUtils.h"

int getRandomIntBetween(int min, int max)
{
    if (min > max)
    {
        int temp = min;
        min = max;
        max = temp;
    }
    int range = max - min;
    int random = floor(getRandom()*range);
    return random+min;
}

double getRandom()
{
    double random = rand()/(double)RAND_MAX;
    if (random == 1)
    {
        random = 0.999999999999999;
    }
    return random;
}
