#include <stdio.h>
#include "CustomTypes.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <string.h>
#include "StringUtils.h"
#include "MessageUtils.h"
#include "SemaphoreUtils.h"
#include "SharedMemoryUtils.h"
#include "SignalUtils.h"
#include <signal.h>
#include "NumberUtils.h"
#include "ConfigUtils.h"

//Pid processi
int masterPid = -1;
int atomPid = -1;

//Code di messaggi
int masterMessageChannelId = -1;
int atomMessageChannelId = -1;

//Memoria condivisa
int statisticsSharedMemoryId = -1;

//Semafori
int statisticsSemaphoreId = -1;

int atomicNumber = -1;
int minAtomicNumber = -1;
int energyExplodeThreshold = -1;

extern char** environ;

void listenMessages();
boolean split();
int getSplitEnergy(int firstAtomicNumber, int secondAtomicNumber);

void onTerminate(int sig)
{
    if (sig == SIGUSR1)
    {
        if (isLockedByThisProcess(statisticsSemaphoreId))
        {
            unlockSemaphore(statisticsSemaphoreId);
        }
        killMessageChannel(atomMessageChannelId);
        detachFromSharedMemory(statisticsSharedMemoryId);
        sendMessage(masterMessageChannelId, createMessage(2, stringJoin("AtomDie=", intToString(atomPid))));
        exit(0);
    }
}

int main()
{
    ignoreSignal(SIGINT);
    setSignalAction(SIGUSR1, onTerminate);

    boolean sendCreate = true;
    char** env = environ;
    while (*env != NULL)
    {
        if (stringStartsWith(*env,"MasterPid="))
        {
            masterPid = atoi(stringAfter(*env,"="));
        }
        else if (stringStartsWith(*env,"AtomicNumber="))
        {
            atomicNumber = atoi(stringAfter(*env,"="));
        }
        else if (stringEquals(*env, "AlreadyAdded"))
        {
            sendCreate = false;
        }
        env++;
    }

    //Pid processi
    atomPid = getpid();

    //Code di messaggi
    masterMessageChannelId = getMessageId(masterPid);
    atomMessageChannelId = getMessageId(atomPid);

    //Semafori
    statisticsSemaphoreId = getSemaphore(STATISTICS_SEMAPHORE);

    //Memoria condivisa
    statisticsSharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int)*11);

    //Config
    minAtomicNumber = getConfigValue(MIN_N_ATOMICO);
    energyExplodeThreshold = getConfigValue(ENERGY_EXPLODE_THRESHOLD);
    unloadConfig();

    //Statistics
    waitAndLockSemaphore(statisticsSemaphoreId);
    int* statistics = getSharedMemory(statisticsSharedMemoryId);
    statistics[CREATED_ATOMS]++;
    unlockSemaphore(statisticsSemaphoreId);

    if (sendCreate)
    {
        sendMessage(masterMessageChannelId, createMessage(2, stringJoin("AtomCreate=", intToString(atomPid))));
    }
    listenMessages();

    killMessageChannel(atomMessageChannelId);
    detachFromSharedMemory(statisticsSharedMemoryId);
    sendMessage(masterMessageChannelId, createMessage(2, stringJoin("AtomDie=", intToString(atomPid))));
    return 0;
}


void listenMessages()
{
    Message message = createEmptyMessage();
    int result = msgrcv(atomMessageChannelId, &message, sizeof(message), 1, 0);
    if (result != -1)
    {
        if (message.messageType == 1)
        {
            if (stringEquals(message.messageText, "Split"))
            {
                if (split())
                {
                    return;
                }
            }
        }
    }
    listenMessages();
}

boolean split()
{
    if (atomicNumber > minAtomicNumber)
    {
        int childAtomicNumber = getRandomIntBetween(minAtomicNumber, atomicNumber-1);
        atomicNumber -= childAtomicNumber;
        int energy = getSplitEnergy(atomicNumber, childAtomicNumber);
        sendMessage(masterMessageChannelId, createMessage(3, stringJoin(intToString(atomPid), stringJoin(";AtomSplit=", intToString(energy)))));
        Message message = createEmptyMessage();
        int result = msgrcv(atomMessageChannelId, &message, sizeof(message), 0, 0);
        if (result != -1)
        {
            if (message.messageType == 4)
            {
                boolean canCreate = stringEquals(message.messageText, "1") ? true : false;
                if (canCreate)
                {
                    pid_t childAtomPid = fork();
                    if (childAtomPid == -1)
                    {
                        sendMessage(masterMessageChannelId, createMessage(1, "Meltdown"));
                        return false;
                    }
                    else if (childAtomPid == 0)
                    {
                        char* forkArgs[] = {NULL};
                        char* forkEnv[] = {
                                stringJoin("AtomicNumber=", intToString(childAtomicNumber)),
                                stringJoin("MasterPid=", intToString(masterPid)),
                                NULL};
                        execve("./Atom", forkArgs, forkEnv);
                        printf("Errore Processo Atomo\n");
                        return false;
                    }
                    int finalEnergy = energy;
                    sendMessage(masterMessageChannelId, createMessage(3, stringJoin(intToString(atomPid), stringJoin(";AtomEnergy=", intToString(energy)))));
                    message = createEmptyMessage();
                    result = msgrcv(atomMessageChannelId, &message, sizeof(message), 0, 0);
                    if (result != -1)
                    {
                        if (message.messageType == 2)
                        {
                            char* key = stringBefore(message.messageText, "=");
                            char* value = stringAfter(message.messageText, "=");
                            if (stringEquals(key, "AtomEnergy"))
                            {
                                finalEnergy = atoi(value);
                            }
                        }
                    }
                    waitAndLockSemaphore(statisticsSemaphoreId);
                    int* statistics = getSharedMemory(statisticsSharedMemoryId);
                    statistics[SPLIT_AMOUNT]++;
                    statistics[ABSORBED_ENERGY] += energy - finalEnergy;
                    statistics[ENERGY_AMOUNT] += finalEnergy;
                    statistics[ENERGY_PRODUCED] += energy;
                    boolean explode = statistics[ENERGY_AMOUNT] > energyExplodeThreshold;
                    unlockSemaphore(statisticsSemaphoreId);
                    if (explode)
                    {
                        sendMessage(masterMessageChannelId, createMessage(1, "Explode"));
                    }
                    return false;
                }
            }
        }
        return false;
    }
    else
    {
        waitAndLockSemaphore(statisticsSemaphoreId);
        int* statistics = getSharedMemory(statisticsSharedMemoryId);
        statistics[DEAD_ATOMS]++;
        unlockSemaphore(statisticsSemaphoreId);
        return true;
    }
}

int getSplitEnergy(int firstAtomicNumber, int secondAtomicNumber)
{
    return firstAtomicNumber * secondAtomicNumber - max(firstAtomicNumber, secondAtomicNumber);
}