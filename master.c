#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "StringUtils.h"

int ENERGY_DEMAND = -1;
int ENERGY_EXPLODE_THRESHOLD = -1;
int N_NUOVI_ATOMI = -1;
int N_ATOMI_INIT = -1;
int N_ATOMI_MAX = -1;
int MIN_N_ATOMICO = -1;
int STEP = -1;
int SIM_DURATION = -1;

void readConfig();

int main() {
    printf("Hello, World 4!\n");
    printf("key = %s\nvalue = %s\n", stringBefore("string=25", "="), stringAfter("string=25", "="));
    readConfig();
    printf("ENERGY_DEMAND = %i\n", ENERGY_DEMAND);
    printf("ENERGY_EXPLODE_THRESHOLD = %i\n", ENERGY_EXPLODE_THRESHOLD);
    printf("N_NUOVI_ATOMI = %i\n", N_NUOVI_ATOMI);
    printf("N_ATOMI_INIT = %i\n", N_ATOMI_INIT);
    printf("N_ATOMI_MAX = %i\n", N_ATOMI_MAX);
    printf("MIN_N_ATOMICO = %i\n", MIN_N_ATOMICO);
    printf("STEP = %i\n", STEP);
    printf("SIM_DURATION = %i\n", SIM_DURATION);
    return 0;
}


void readLine(char* line)
{
    line = stringClearChar(line, ' ');
    if (stringEndsWith(line, "\0"))
    {
        line = stringCut(line, 0, stringLength(line) - 2);
    }
    if (stringEndsWith(line, "\n"))
    {
        line = stringCut(line, 0, stringLength(line) - 1);
    }
    char* key = stringBefore(line, "=");
    char* value = stringAfter(line, "=");

    if (stringEquals(key, "ENERGY_DEMAND"))
    {
        ENERGY_DEMAND = atoi(value);
    }
    else if (stringEquals(key, "ENERGY_EXPLODE_THRESHOLD"))
    {
        ENERGY_EXPLODE_THRESHOLD = atoi(value);
    }
    else if (stringEquals(key, "N_NUOVI_ATOMI"))
    {
        N_NUOVI_ATOMI = atoi(value);
    }
    else if (stringEquals(key, "N_ATOMI_INIT"))
    {
        N_ATOMI_INIT = atoi(value);
    }
    else if (stringEquals(key, "N_ATOMI_MAX"))
    {
        N_ATOMI_MAX = atoi(value);
    }
    else if (stringEquals(key, "MIN_N_ATOMICO"))
    {
        MIN_N_ATOMICO = atoi(value);
    }
    else if (stringEquals(key, "STEP"))
    {
        STEP = atoi(value);
    }
    else if (stringEquals(key, "SIM_DURATION"))
    {
        SIM_DURATION = atoi(value);
    }


}

void readConfig()
{
    FILE* file = fopen("./config.txt", "r");
    if (file != NULL)
    {
        char* line = emptyString(100);
        while (fgets(line, 100, file) != NULL)
        {
            readLine(line);
        }
        fclose(file);
    }
}
