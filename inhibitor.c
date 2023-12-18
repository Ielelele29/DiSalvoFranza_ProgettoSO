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
int masterInhibitorMessageChannelId = -1;
int inhibitorMessageChannelId = -1;

//Memoria condivisa
int statisticsSharedMemoryId = -1;

//Semafori
int statisticsSemaphoreId = -1;


void listenMessages();

void onTerminate(int sig)
{
    if (sig == SIGUSR1)
    {
        killMessageChannel(inhibitorMessageChannelId);
        killMessageChannel(masterInhibitorMessageChannelId);
        detachFromSharedMemory(statisticsSharedMemoryId);
        sendMessage(masterMessageChannelId, createMessage(1, "InhibitorStop"));
        exit(0);
    }
}

int main() {
    //Segnali
    ignoreSignal(SIGINT);
    setSignalAction(SIGUSR1, onTerminate);

    //Pid processi
    masterPid = getppid();
    inhibitorPid = getpid();

    //Config
    energyExplodeThreshold = getConfigValue(ENERGY_EXPLODE_THRESHOLD);
    unloadConfig();

    //Ricezione messaggi
    masterMessageChannelId = getMessageId(masterPid);
    masterInhibitorMessageChannelId = getMessageId(MaxPid+1);
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
            boolean isLocked = true;
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
                    unlockSemaphore(statisticsSemaphoreId);
                    isLocked = false;
                    sendMessage(masterInhibitorMessageChannelId, createMessage(4, "0"));
                }
                else //Rallentamento scissione atomi
                {
                    if (getRandom() < 1-perc)
                    {
                        sendMessage(masterInhibitorMessageChannelId, createMessage(4, "1"));
                    }
                    else
                    {
                        statistics[DELAYED_ATOM_SPLIT]++;
                        unlockSemaphore(statisticsSemaphoreId);
                        isLocked = false;
                        sendMessage(masterInhibitorMessageChannelId, createMessage(4, "0"));
                    }
                }
            }
            else if (stringEquals(key, "AtomEnergy"))
            {
                sendMessage(masterInhibitorMessageChannelId, createMessage(5, stringJoin("AtomEnergy=", intToString(finalEnergy))));
            }
            if (isLocked)
            {
                unlockSemaphore(statisticsSemaphoreId);
            }
            free(key);
            free(valueS);
        }
    }
    listenMessages();
}