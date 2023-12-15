#include <stdlib.h>
#include <stdio.h>
#include "ConfigUtils.h"
#include "StringUtils.h"
#include "SemaphoreUtils.h"
#include "SharedMemoryUtils.h"

int configSharedMemory = -1;
int configSemaphore = -1;

void readLine(char* line)
{
    //Pulizia linea da spazi ed altri caratteri extra
    line = stringClearChar(line, ' ');
    if (stringEndsWith(line, "\0"))
    {
        line = stringCut(line, 0, stringLength(line) - 1);
    }
    if (stringEndsWith(line, "\n"))
    {
        line = stringCut(line, 0, stringLength(line) - 1);
    }

    //Ottenimento valori prima e dopo l'uguale
    char* key = stringBefore(line, "=");
    char* valueS = stringAfter(line, "=");
    int value = atoi(valueS);

    //Impostazione valori delle variabili lette da config
    if (stringEquals(key, "ENERGY_DEMAND"))
    {
        setConfigValue(ENERGY_DEMAND, value);
    }
    else if (stringEquals(key, "ENERGY_EXPLODE_THRESHOLD"))
    {
        setConfigValue(ENERGY_EXPLODE_THRESHOLD, value);
    }
    else if (stringEquals(key, "N_NUOVI_ATOMI"))
    {
        setConfigValue(N_NUOVI_ATOMI, value);
    }
    else if (stringEquals(key, "N_ATOMI_INIT"))
    {
        setConfigValue(N_ATOMI_INIT, value);
    }
    else if (stringEquals(key, "N_ATOM_MAX"))
    {
        setConfigValue(N_ATOM_MAX, value);
    }
    else if (stringEquals(key, "MIN_N_ATOMICO"))
    {
        setConfigValue(MIN_N_ATOMICO, value);
    }
    else if (stringEquals(key, "STEP"))
    {
        setConfigValue(STEP, value);
    }
    else if (stringEquals(key, "SIM_DURATION"))
    {
        setConfigValue(SIM_DURATION, value);
    }
    free(key);
    free(valueS);
}

void loadConfig()
{
    configSemaphore = getSemaphore(CONFIG_SEMAPHORE);
    configSharedMemory = getSharedMemoryId(CONFIG_SHARED_MEMORY, sizeof(int)*8);
}

void createConfig()
{
    if (configSemaphore == -1)
    {
        loadConfig();
        clearSharedMemory(configSharedMemory);
        unlockSemaphore(configSemaphore);

        FILE* file = fopen("./config.txt", "r");
        if (file != NULL)
        {
            char* line = emptyString(100);
            while (fgets(line, 100, file) != NULL)
            {
                readLine(line);
            }
            free(line);
            fclose(file);
        }
    }
}

void deleteConfig()
{
    if (configSemaphore != -1)
    {
        waitAndLockSemaphore(configSemaphore);
        deleteSharedMemory(configSharedMemory);
        deleteSemaphore(configSemaphore);
        configSemaphore = -1;
        configSharedMemory = -1;
    }
}

void setConfigValue(Config config, int value)
{
    if (configSemaphore == -1)
    {
        loadConfig();
    }
    waitAndLockSemaphore(configSemaphore);
    int* mem = getSharedMemory(configSharedMemory);
    mem[config] = value;
    unlockSemaphore(configSemaphore);
}

int getConfigValue(Config config)
{
    if (configSemaphore == -1)
    {
        loadConfig();
    }
    waitAndLockSemaphore(configSemaphore);
    int* mem = getSharedMemory(configSharedMemory);
    int value = mem[config];
    unlockSemaphore(configSemaphore);
    return value;
}