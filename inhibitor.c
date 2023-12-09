#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>
#include "StringUtils.h"
#include "MessageUtils.h"
#include "SemaphoreUtils.h"
#include "SignalUtils.h"
#include "SharedMemoryUtils.h"
#include "NumberUtils.h"

extern char** environ;

//Variabili
int ENERGY_EXPLODE_THRESHOLD = -1;


//Memoria condivisa
int statisticsSharedMemoryId = -1;
int inhibitorSharedMemoryId = -1;

//Semafori
int statisticsSemaphore = -1;
int inhibitorSemaphore = -1;


boolean isActive();

int main() {

    //Lettura variabili d'ambiente
    char** env = environ;
    char* envKey;
    char* envValue;
    while (*env != NULL)
    {
        envKey = stringBefore(*env, "=");
        envValue = stringAfter(*env, "=");
        if (stringEquals(envKey, "ENERGY_EXPLODE_THRESHOLD"))
        {
            ENERGY_EXPLODE_THRESHOLD = atoi(envValue);
        }
        env++;
    }
    free(envKey);
    free(envValue);

    //Inizializzazione statistiche
    statisticsSemaphore = getSemaphore(STATISTICS_SEMAPHORE);
    statisticsSharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int) * 10);

    //Inizializzazione inibitore
    ignoreSignal(SIGINT);
    inhibitorSemaphore = getSemaphore(INHIBITOR_SEMAPHORE);
    inhibitorSharedMemoryId = getSharedMemoryId(INHIBITOR_SHARED_MEMORY, sizeof(int));

    //Ricezione messaggi
    int messageChannelId = getMessageId(getpid());
    Message message;
    while (true)
    {
        msgrcv(messageChannelId, &message, sizeof(message), 0, 0);
        if (message.messageType == 1) //Terminazione
        {
            if (stringEquals(message.messageText, "term"))
            {
                break;
            }
        }
        else if (message.messageType == 2)
        {
            char* pidS = stringBefore(message.messageText, ";");
            int pid = atoi(pidS);
            if (isActive())
            {
                char* text = stringAfter(message.messageText, ";");
                char* key = stringBefore(text, "=");
                char* value = stringAfter(text, "=");
                if (stringEquals(key, "SPLIT_ENERGY_PRODUCED"))
                {
                    int energy = atoi(value);
                    waitAndLockSemaphore(statisticsSemaphore);
                    int* memory = getSharedMemory(statisticsSharedMemoryId);
                    int energyAmount = memory[ENERGY_AMOUNT];
                    if (energyAmount + energy > ENERGY_EXPLODE_THRESHOLD) //Evitare esplosioni
                    {
                        memory[AVOIDED_EXPLOSIONS]++;
                        sendMessage(pid, createMessage(2, "0"));
                    }
                    else //Rallentamento scissione atomi
                    {
                        double perc = energyAmount/(double)ENERGY_EXPLODE_THRESHOLD;
                        if (getRandom() < 1-perc)
                        {
                            sendMessage(pid, createMessage(2, "1"));
                        }
                        else
                        {
                            memory[DELAYED_ATOM_SPLIT]++;
                            sendMessage(pid, createMessage(2, "0"));
                        }
                    }
                    unlockSemaphore(statisticsSemaphore);
                }
                else if (stringEquals(key, "MELTDOWN")) //Evitare meltdown
                {
                    sendMessage(pid, createMessage(2, "0"));
                    waitAndLockSemaphore(statisticsSemaphore);
                    int* memory = getSharedMemory(statisticsSharedMemoryId);
                    memory[AVOIDED_MELTDOWNS]++;
                    unlockSemaphore(statisticsSemaphore);
                }
                free(text);
                free(key);
                free(value);
            }
            else
            {
                sendMessage(pid, createMessage(2, "1"));
            }
            free(pidS);
        }
    }
    printf("END INHIBITOR\n");
    return 0;
}

boolean isActive()
{
    waitAndLockSemaphore(inhibitorSemaphore);
    int* memory = getSharedMemory(inhibitorSharedMemoryId);
    boolean isActive = *memory ? true : false;
    unlockSemaphore(inhibitorSemaphore);
    return isActive;
}