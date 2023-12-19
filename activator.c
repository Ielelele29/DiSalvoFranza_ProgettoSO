#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include "StringUtils.h"
#include "MessageUtils.h"
#include "CustomTypes.h"
#include "SemaphoreUtils.h"
#include "SignalUtils.h"
#include <signal.h>
#include "NodeUtils.h"
#include <time.h>
#include "NumberUtils.h"
#include "SharedMemoryUtils.h"
#include "ConfigUtils.h"


//Pid processi
int masterPid = -1;
int activatorPid = -1;

//Code di messaggi
int masterMessageChannelId = -1;
int activatorMessageChannelId = -1;

//Memoria condivisa
int statisticsSharedMemoryId = -1;

//Semafori
int statisticsSemaphoreId = -1;

int step = -1;

void listenMessage();
void waitNano();
void splitAtoms(int j);

void onTerminate(int sig)
{
    if (sig == SIGUSR1)
    {
        if (isLockedByThisProcess(statisticsSemaphoreId))
        {
            unlockSemaphore(statisticsSemaphoreId);
        }
        killMessageChannel(activatorMessageChannelId);
        detachFromSharedMemory(statisticsSharedMemoryId);
        sendMessage(masterMessageChannelId, createMessage(1, "ActivatorStop"));
        exit(0);
    }
}

int main()
{
    //Segnali
    ignoreSignal(SIGINT);
    setSignalAction(SIGUSR1, onTerminate);

    //Pid processi
    masterPid = getppid();
    activatorPid = getpid();

    //Code di messaggi
    masterMessageChannelId = getMessageId(masterPid);
    activatorMessageChannelId = getMessageId(activatorPid);

    //Semafori
    statisticsSemaphoreId = getSemaphore(STATISTICS_SEMAPHORE);

    //Memoria condivisa
    statisticsSharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int)*11);

    //Inizializzazione variabili
    step = getConfigValue(STEP);
    unloadConfig();

    sendMessage(masterMessageChannelId, createMessage(1, "ActivatorReady"));
    listenMessage();


    return 0;
}

Atom atoms = NULL;
void readAtoms()
{
    atoms = NULL;
    sendMessage(masterMessageChannelId, createMessage(1,"AtomList"));
    while(true)
    {
        Message message = createEmptyMessage();
        int result = msgrcv(activatorMessageChannelId, &message, sizeof(message), 0, 0);
        if (result != -1)
        {
            if (message.messageType == 1)
            {
                if (stringEquals(message.messageText, "AtomEnd"))
                {
                    break;
                }
            }
            else if (message.messageType == 2)
            {
                char* key = stringBefore(message.messageText, "=");
                char* value = stringAfter(message.messageText, "=");
                if (stringStartsWith(key, "AtomPid"))
                {
                    addNode(&atoms, atoi(value));
                }
                free(key);
                free(value);
            }
        }
    }
    splitAtoms(0);
    readAtoms();
}

int i = 0;
void splitAtoms(int j)
{
    int k = 0;
    while (atoms != NULL && k < 50)
    {
        if ((i+j)%3 == 0)
        {
            sendMessage(getMessageId(getNodeValue(atoms)), createMessage(1, "Split"));
            waitAndLockSemaphore(statisticsSemaphoreId);
            int* statistics = getSharedMemory(statisticsSharedMemoryId);
            statistics[ACTIVATION_AMOUNT]++;
            unlockSemaphore(statisticsSemaphoreId);
            k++;
        }
        atoms = getNextNode(atoms);
        j++;
    }
    i++;
    waitNano();
    if (atoms != NULL)
    {
        splitAtoms(j);
    }
}



void waitNano()
{
    double nanoTime = step*(getRandom()+0.4);
    struct timespec timeToSleep;
    timeToSleep.tv_sec = (int) (nanoTime/1000000000);
    timeToSleep.tv_nsec = (int) ((int)nanoTime%1000000000);
    nanosleep(&timeToSleep, NULL);
}

void listenMessage()
{
    Message message = createEmptyMessage();
    int result = msgrcv(activatorMessageChannelId, &message, sizeof(message), 0, 0);
    if (result != -1)
    {
        if (message.messageType == 1)
        {
            if (stringEquals(message.messageText, "Start"))
            {
                readAtoms();
                return;
            }
        }
    }
    listenMessage();
}