//
// Created by lelelele29 on 08/12/23.
//

#include <stdlib.h>
#include <math.h>
#include <time.h>
#include "NumberUtils.h"
#include "CustomTypes.h"

boolean isInitialized = false;
double getRandom()
{
    if (!isInitialized)
    {
        srand(time(NULL));
        isInitialized = true;
    }
    double random = rand()/(double)RAND_MAX;
    if (random == 1)
    {
        random = 0.999999999999999;
    }
    return random;
}

int getRandomIntBetween(int min, int max)
{
    if (min > max)
    {
        int temp = min;
        min = max;
        max = temp;
    }
    int range = max - min + 1;
    int random = floor(getRandom()*range);
    return random+min;
}

double floor(double num)
{
    int integerPart = (int) num;
    if (integerPart < 0 && integerPart != num)
    {
        integerPart--;
    }
    return integerPart;
}

int getAtomFunction(int N_ATOMICO, int N_ATOM_MAX)
{
    if(N_ATOMICO <= N_ATOM_MAX*0.2)
    {
        return 0;
    }
    else if (N_ATOMICO <= N_ATOM_MAX*0.6) //N_ATOMICO > N_ATOM_MAX*0.2 (implicit)
    {
        return 1;
    }
    else
    {
        return 2;
    }
}

int max(int num1, int num2)
{
    return num1 > num2 ? num1 : num2;
}