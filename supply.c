#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "CustomTypes.h"
#include <stdlib.h>
#include <sys/msg.h>
#include <string.h>
#include "StringUtils.h"
#include "MessageUtils.h"
#include "SemaphoreUtils.h"
#include "SignalUtils.h"
#include <signal.h>
#include "NumberUtils.h"
#include "ConfigUtils.h"
#include "SharedMemoryUtils.h"

//Pid processi
int masterPid = -1;
int supplyPid = -1;

//Code di messaggi
int masterMessageChannelId = -1;
int supplyMessageChannelId = -1;

//Memoria condivisa
int statisticsSharedMemoryId = -1;

//Semafori
int statisticsSemaphoreId = -1;


int step = -1;
int atomsAmount = -1;
int minAtomicNumber = -1;
int maxAtomicNumber = -1;


void listenMessage();
void createAtom();

void onTerminate(int sig)
{
    if (sig == SIGUSR1)
    {
        if (isLockedByThisProcess(statisticsSemaphoreId))
        {
            unlockSemaphore(statisticsSemaphoreId);
        }
        killMessageChannel(supplyMessageChannelId);
        detachFromSharedMemory(statisticsSharedMemoryId);
        sendMessage(masterMessageChannelId, createMessage(1, "SupplyStop"));
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
    supplyPid = getpid();

    //Code di messaggi
    masterMessageChannelId = getMessageId(masterPid);
    supplyMessageChannelId = getMessageId(supplyPid);

    //Semafori
    statisticsSemaphoreId = getSemaphore(STATISTICS_SEMAPHORE);

    //Memoria condivisa
    statisticsSharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int)*11);

    //Inizializzazione variabili
    step = getConfigValue(STEP);
    atomsAmount = getConfigValue(N_NUOVI_ATOMI);
    minAtomicNumber = getConfigValue(MIN_N_ATOMICO);
    maxAtomicNumber = getConfigValue(N_ATOM_MAX);
    unloadConfig();

    sendMessage(masterMessageChannelId, createMessage(1, "SupplyReady"));
    listenMessage();
    return 0;
}



void generateAtoms()
{
    for (int i = 0; i < atomsAmount; i++)
    {
        createAtom();
    }
    struct timespec timeToSleep;
    timeToSleep.tv_sec = step/1000000000;
    timeToSleep.tv_nsec = step%1000000000;
    nanosleep(&timeToSleep, NULL);
    generateAtoms();
}

void createAtom()
{
    pid_t atomPid = fork();
    if (atomPid == -1)
    {
        sendMessage(masterMessageChannelId, createMessage(1, "Meltdown"));
        return;
    }
    else if (atomPid == 0)
    {
        int atomicNumber = getRandomIntBetween(minAtomicNumber, maxAtomicNumber);
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {
                stringJoin("AtomicNumber=", intToString(atomicNumber)),
                stringJoin("MasterPid=", intToString(masterPid)),
                NULL};
    //    printf("Atomo creato correttamente da Supply\n");
        execve("./Atom", forkArgs, forkEnv);
        printf("Errore Processo Atomo\n");
        return;
    }
}

void listenMessage()
{
    Message message = createEmptyMessage();
    int result = msgrcv(supplyMessageChannelId, &message, sizeof(message), 0, 0);
    if (result != -1)
    {
        if (message.messageType == 1)
        {
            if (stringEquals(message.messageText, "Start"))
            {
                generateAtoms();
                return;
            }
        }
    }
    listenMessage();
}