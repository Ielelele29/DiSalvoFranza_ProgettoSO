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
#include "ConfigUtils.h"

extern char** environ;

//Pid processi
int masterPid = -1;
int inhibitorPid = -1;

//Variabili
int energyExplodeThreshold = -1;

//Code di messaggi
int masterMessageChannelId = -1;
int inhibitorMessageChannelId = -1;

//Memoria condivisa
int statisticsSharedMemoryId = -1;

//Semafori
int statisticsSemaphoreId = -1;


void listenMessages();



void checkError(int sig)
{
    printf("Inhibitor error\n\n\n");
}

void onTerminate(int sig)
{
    if (sig == SIGUSR1)
    {
        killMessageChannel(inhibitorMessageChannelId);
        detachFromSharedMemory(statisticsSharedMemoryId);
        sendMessage(masterMessageChannelId, createMessage(1, "InhibitorStop"));
        printf("END INHIBITOR\n");
        exit(0);
    }
}

int main() {
    //Segnali
    ignoreSignal(SIGINT);
    setSignalAction(SIGSEGV, checkError);
    setSignalAction(SIGUSR1, onTerminate);

    //Pid processi
    masterPid = getppid();
    inhibitorPid = getpid();

    //Config
    energyExplodeThreshold = getConfigValue(ENERGY_EXPLODE_THRESHOLD);
    unloadConfig();

    //Ricezione messaggi
    masterMessageChannelId = getMessageId(masterPid);
    inhibitorMessageChannelId = getMessageId(inhibitorPid);

    //Semafori
    statisticsSemaphoreId = getSemaphore(STATISTICS_SEMAPHORE);

    //Memoria condivisa
    statisticsSharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int)*11);

    sendMessage(masterMessageChannelId, createMessage(1, "InhibitorReady"));
    listenMessages();
    return 0;
}

void listenMessages()
{
    Message message;
    int result = msgrcv(inhibitorMessageChannelId, &message, sizeof(message), 0, 0);
    if (result != -1)
    {
        if (message.messageType == 1) //Terminazione
        {
        /*    if (stringEquals(message.messageText, "Stop"))
            {
                return;
            }
            else */
            if (stringEquals(message.messageText, "Explode"))
            {
                waitAndLockSemaphore(statisticsSemaphoreId);
                int* statistics = getSharedMemory(statisticsSharedMemoryId);
                statistics[AVOIDED_EXPLOSIONS]++;
                if (statistics[ENERGY_AMOUNT] > energyExplodeThreshold)
                {
                    int diff = statistics[ENERGY_AMOUNT] - energyExplodeThreshold;
                    statistics[ABSORBED_ENERGY] += diff;
                    statistics[ENERGY_AMOUNT] -= diff;
                }
                unlockSemaphore(statisticsSemaphoreId);
            }
            else if (stringEquals(message.messageText, "Meltdown"))
            {
                waitAndLockSemaphore(statisticsSemaphoreId);
                int* statistics = getSharedMemory(statisticsSharedMemoryId);
                statistics[AVOIDED_MELTDOWNS]++;
                unlockSemaphore(statisticsSemaphoreId);
            }
        }
        else if (message.messageType == 2)
        {
            char* key = stringBefore(message.messageText, "=");
            char* valueS = stringAfter(message.messageText, "=");
            int energy = atoi(valueS);
            waitAndLockSemaphore(statisticsSemaphoreId);
            int* statistics = getSharedMemory(statisticsSharedMemoryId);
            int actualEnergy = statistics[ENERGY_AMOUNT];
            double perc = actualEnergy/(double)energyExplodeThreshold;
            int finalEnergy = (int)(energy*(1-(0.3*perc)));
            if (stringEquals(key, "AtomSplit"))
            {
                if (actualEnergy + finalEnergy > energyExplodeThreshold) //Evitare esplosioni
                {
                    statistics[AVOIDED_EXPLOSIONS]++;
                    sendMessage(masterMessageChannelId, createMessage(4, "0"));
                }
                else //Rallentamento scissione atomi
                {
                    if (getRandom() < 1-perc)
                    {
                        sendMessage(masterMessageChannelId, createMessage(4, "1"));
                    }
                    else
                    {
                        statistics[DELAYED_ATOM_SPLIT]++;
                        sendMessage(masterMessageChannelId, createMessage(4, "0"));
                    }
                }
            }
            else if (stringEquals(key, "AtomEnergy"))
            {
                sendMessage(masterMessageChannelId, createMessage(5, stringJoin("AtomEnergy=", intToString(finalEnergy))));
            }
            unlockSemaphore(statisticsSemaphoreId);

            /*
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
            free(pidS);*/
        }
    }
    listenMessages();
}