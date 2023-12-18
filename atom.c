#include <stdio.h>
#include "CustomTypes.h"
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include "StringUtils.h"
#include "MessageUtils.h"
#include "SemaphoreUtils.h"
#include "SharedMemoryUtils.h"
#include "SignalUtils.h"
#include <signal.h>
#include "NumberUtils.h"
#include "SignalUtils.h"
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



void checkError(int sig)
{
    printf("Atom error\n\n\n");
}

void onTerminate(int sig)
{
    if (sig == SIGUSR1)
    {
        killMessageChannel(atomMessageChannelId);
        detachFromSharedMemory(statisticsSharedMemoryId);
        sendMessage(masterMessageChannelId, createMessage(2, stringJoin("AtomDie=", intToString(atomPid))));
        //   printf("END ATOM\n");
        exit(0);
    }
}

int main()
{
    printf("A1\n");
    ignoreSignal(SIGINT);
    setSignalAction(SIGSEGV, checkError);
    setSignalAction(SIGUSR1, onTerminate);

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
        env++;
    }
    printf("A2\n");

    //Pid processi
    atomPid = getpid();

    //Code di messaggi
    masterMessageChannelId = getMessageId(masterPid);
    atomMessageChannelId = getMessageId(atomPid);
    printf("A3\n");

    //Semafori
    statisticsSemaphoreId = getSemaphore(STATISTICS_SEMAPHORE);

    //Memoria condivisa
    statisticsSharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int)*11);

    //Config
    minAtomicNumber = getConfigValue(MIN_N_ATOMICO);
    energyExplodeThreshold = getConfigValue(ENERGY_EXPLODE_THRESHOLD);
    unloadConfig();
    printf("A4\n");

    //Statistics
    waitAndLockSemaphore(statisticsSemaphoreId);
    int* statistics = getSharedMemory(statisticsSharedMemoryId);
    statistics[CREATED_ATOMS]++;
    unlockSemaphore(statisticsSemaphoreId);

    printf("A5\n");
 //   printf("[Atom %i] Send Create\n", atomPid);
    sendMessage(masterMessageChannelId, createMessage(2, stringJoin("AtomCreate=", intToString(atomPid))));
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
    //    printf("[Atom] Messaggio = %s\n", message.messageText);
        if (message.messageType == 1)
        {
        /*    if (stringEquals(message.messageText, "Stop"))
            {
                return;
            }
            else*/
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
 //   printf("[Atom] Splitting with atomic number = %i\n", atomicNumber);
    if (atomicNumber > minAtomicNumber)
    {
        // effettuare scissione
        int childAtomicNumber = getRandomIntBetween(minAtomicNumber, atomicNumber-1);
 //       printf("[Atom] Child atomic number = %i\n", childAtomicNumber);
        atomicNumber -= childAtomicNumber;
        int energy = getSplitEnergy(atomicNumber, childAtomicNumber);
  //      printf("%i Fase 1\n", atomPid);
        sendMessage(masterMessageChannelId, createMessage(3, stringJoin(intToString(atomPid), stringJoin(";AtomSplit=", intToString(energy)))));
        Message message = createEmptyMessage();
        int result = msgrcv(atomMessageChannelId, &message, sizeof(message), 0, 0);
    //    printf("[Atom] %i message 1 = %s\n", getpid(), message.messageText);
        if (result != -1)
        {
        /*    if (message.messageType == 1)
            {
                if (stringEquals(message.messageText, "Stop"))
                {
                    printf("STOPPONE 1\n\n\n");
                    return true;
                }
            }
            else */
            if (message.messageType == 4)
            {
                boolean canCreate = stringEquals(message.messageText, "1") ? true : false;
                if (canCreate)
                {
                    pid_t childAtomPid = fork();
                    if (childAtomPid == -1)
                    {
                //        printf("Errore durante la creazione del processo Atomo\n");
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
                //    printf("[Atom] %i message 2 = %s\n", getpid(), message.messageText);
                    if (result != -1)
                    {
                    /*    if (message.messageType == 1)
                        {
                            if (stringEquals(message.messageText, "Stop"))
                            {
                                printf("STOPPONE 2\n\n\n");
                                return true;
                            }
                        }
                        else */
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
     //               printf("[Atom %i] Energia: %i --> %i\n", atomPid, energy, finalEnergy);
                    waitAndLockSemaphore(statisticsSemaphoreId);
                    int* statistics = getSharedMemory(statisticsSharedMemoryId);
                    statistics[SPLIT_AMOUNT]++;
                    statistics[ABSORBED_ENERGY] += energy - finalEnergy;
                    statistics[ENERGY_AMOUNT] += finalEnergy;
                    statistics[ENERGY_PRODUCED] += energy;
                    if (statistics[ENERGY_AMOUNT] > energyExplodeThreshold)
                    {
                        sendMessage(masterMessageChannelId, createMessage(1, "Explode"));
                    }
                    unlockSemaphore(statisticsSemaphoreId);
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




















































































/*
int MIN_N_ATOMICO = -1;
boolean isActive();
boolean isTerminating();
void split();
void createAtom();
void setAtomFunction();
int N_ATOMICO = -1;
int ENERGY_EXPLODE_THRESHOLD = -1;
int N_ATOM_MAX = -1;
int sharedMemoryId = -1;
int statisticsSemaphore = -1;
int inhibitorSharedMemoryId = -1;
int inhibitorSemaphore = -1;
int PID_INHIBITOR = -1;
int (*atomSplitFunction)(int,int);
int atomSplitFunction0(int,int);
int atomSplitFunction1(int,int);
int atomSplitFunction2(int,int);
int PID_MASTER = -1;
int messageChannelId = -1;
int masterSignalSemaphore = -1;
extern char **environ;

int main() {
    if (isTerminating())
    {
        printf("STOP ATOM\n\n");
        return 0;
    }
    ignoreSignal(SIGINT);

    char** env = environ;
    while (*env != NULL)
    {
        if(stringStartsWith(*env,"N_ATOMICO="))
        {
            N_ATOMICO = atoi(stringAfter(*env,"="));
        }
        else if(stringStartsWith(*env,"ENERGY_EXPLODE_THRESHOLD="))
        {
            ENERGY_EXPLODE_THRESHOLD = atoi(stringAfter(*env,"="));
        }
        else if(stringStartsWith(*env,"MIN_N_ATOMICO="))
        {
            MIN_N_ATOMICO = atoi(stringAfter(*env,"="));
        }
        else if(stringStartsWith(*env,"PID_INHIBITOR="))
        {
            PID_INHIBITOR = atoi(stringAfter(*env,"="));
        }
        else if(stringStartsWith(*env,"PID_MASTER="))
        {
            PID_MASTER = atoi(stringAfter(*env,"="));
        }
        else if(stringStartsWith(*env,"N_ATOM_MAX="))
        {
            N_ATOM_MAX = atoi(stringAfter(*env,"="));
        }
        else
        {
            printf("Error wrong environ data");
        }

        *env++;
    }

    sharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int)*10);
    statisticsSemaphore = getSemaphore(STATISTICS_SEMAPHORE);
    inhibitorSemaphore = getSemaphore(INHIBITOR_SEMAPHORE);
    inhibitorSharedMemoryId = getSharedMemoryId(INHIBITOR_SHARED_MEMORY, sizeof(int));
    masterSignalSemaphore = getSemaphore(MASTER_SIGNAL_SEMAPHORE);

    waitAndLockSemaphore(masterSignalSemaphore);
    sendMessage(PID_MASTER, createMessage(2, stringJoin("atomCreate=", intToString(atomPid))));
    sendSignal(PID_MASTER, SIGUSR1);
    printf("ATOM SEND SIGNAL %i\n", PID_MASTER);

    waitAndLockSemaphore(statisticsSemaphore);
    int* statistics = getSharedMemory(sharedMemoryId);
    statistics[CREATED_ATOMS]++;
    unlockSemaphore(statisticsSemaphore);
    messageChannelId = getMessageId(atomPid);



    setAtomFunction();

    while(true)
    {
        Message message = createEmptyMessage();
        if(msgrcv(messageChannelId, &message, sizeof(message), 0, 0) != -1)
        {
            printf("MESSAGGIONE = %s\n", message.messageText);
            if (message.messageType == 2)
            {
                if(stringStartsWith(message.messageText,"N_ATOM_MAX="))
                {
                    N_ATOM_MAX = atoi(stringAfter(message.messageText,"N_ATOM_MAX="));
                    srand(time(NULL));
                    numAtom = rand()%N_ATOM_MAX; //implict +0
                }
                if(stringStartsWith(message.messageText,"MIN_N_ATOMICO="))
                {
                    MIN_N_ATOMICO = atoi(stringAfter(message.messageText,"MIN_N_ATOMICO="));
                }
                else if(stringStartsWith(message.messageText,"ENERGY_EXPLODE_THRESHOLD="))
                {
                    ENERGY_EXPLODE_THRESHOLD = atoi(stringAfter(message.messageText,"ENERGY_EXPLODE_THRESHOLD="));
                }
                else
                {
                    printf("Error invalid message!\n");
                    printf("Waiting for a new message...\n");

                }
            }
            if (message.messageType == 1)
            {
                if (stringEquals(message.messageText, "split"))
                {
                    split();
                }
                else if (stringEquals(message.messageText, "term"))
                {
                    break;
                }
                else
                {
                    printf("Error invalid message!\n");
                    printf("Waiting for a new message...\n");
                    printf("francodicicco1\n");
                }

            }
            else
            {
                printf("Error invalid message!\n");
                printf("Waiting for a new message...\n");
                printf("francodicicco2\n");
            }
        }
        else
        {
            printf("Error receiving message!\n");
            printf("Waiting for a new message...\n");
            printf("francodicicco3\n");
        }
    }
    killMessageChannel(atomPid);
    return 0;
}

void split()
{
    if (N_ATOMICO > MIN_N_ATOMICO)
    {
        // effettuare scissione
        int childAtomicNumber = getRandomIntBetween(MIN_N_ATOMICO,N_ATOMICO);
        N_ATOMICO -= childAtomicNumber;
        setAtomFunction();
        int energy = atomSplitFunction(N_ATOMICO,childAtomicNumber);
        if(isActive())
        {
            sendMessage(PID_INHIBITOR,createMessage(2,stringJoin(intToString(atomPid),stringJoin(";SPLIT_ENERGY_PRODUCED=",intToString(energy)))));
            Message message = createEmptyMessage();
            if(msgrcv(messageChannelId, &message, sizeof(message), 2, 0) != -1)
            {
                if(stringEquals(message.messageText,"1"))
                {
                    createAtom(childAtomicNumber,energy);
                }
                else
                {
                    //rallento scissione
                }
            }
            else
            {
                printf("Error receiving message!\n");
                printf("Waiting for a new message...\n");
                printf("errore split assurdosplit\n");
            }

        }
        else
        {
            createAtom(childAtomicNumber,energy);
        }

    }
    else
    {
        waitAndLockSemaphore(statisticsSemaphore);
        int* statistics = getSharedMemory(sharedMemoryId);
        statistics[DEAD_ATOMS]++;
        unlockSemaphore(statisticsSemaphore);
        sendMessage(PID_MASTER, createMessage(2, "atomKill"));
        exit(0);
        //diventa scoria
    }
}

int atomSplitFunction0(int parentAtomicNumber,int childAtomicNumber)
{
    return parentAtomicNumber*parentAtomicNumber + childAtomicNumber*childAtomicNumber;
}

int atomSplitFunction1(int parentAtomicNumber,int childAtomicNumber)
{
    return 2*parentAtomicNumber*childAtomicNumber;
}

int atomSplitFunction2(int parentAtomicNumber,int childAtomicNumber)
{
    return (int)((parentAtomicNumber*childAtomicNumber)/3);
}

boolean isActive()
{
    waitAndLockSemaphore(inhibitorSemaphore);
    int* memory = getSharedMemory(inhibitorSharedMemoryId);
    boolean isActive = *memory ? true : false;
    unlockSemaphore(inhibitorSemaphore);
    return isActive;
}

void createAtom(int childAtomicNumber, int energy)
{
    pid_t atomPid = fork();
    if (atomPid == -1)
    {
        printf("Errore durante la creazione del processo Atomo\n");
        if(isActive())
        {
            sendMessage(PID_INHIBITOR,createMessage(2,stringJoin(intToString(atomPid),";MELTDOWN")));
            Message message = createEmptyMessage();
            if(msgrcv(messageChannelId, &message, sizeof(message), 2, 0) != -1)
            {
                if(stringEquals(message.messageText,"1"))
                {
                    waitAndLockSemaphore(masterSignalSemaphore);
                    sendMessage(PID_MASTER, createMessage(1, "MELTDOWN"));
                    sendSignal(PID_MASTER, SIGUSR1);
                }
            }
            else
            {
                printf("Error receiving message!\n");
                printf("Waiting for a new message...\n");
                printf("firifillo1\n");
            }
        }
        else
        {
            waitAndLockSemaphore(masterSignalSemaphore);
            sendMessage(PID_MASTER, createMessage(1, "MELTDOWN"));
            sendSignal(PID_MASTER, SIGUSR1);
        }
    }
    else if (atomPid == 0)
    {
        char *forkArgs[] = {NULL};
        char* forkEnv[] = {
                stringJoin("ENERGY_EXPLODE_THRESHOLD=", intToString(ENERGY_EXPLODE_THRESHOLD)),
                stringJoin("MIN_N_ATOMICO=", intToString(MIN_N_ATOMICO)),
                stringJoin("N_ATOMICO=", intToString(childAtomicNumber)),
                stringJoin("PID_MASTER=",intToString(PID_MASTER)),
                stringJoin("N_ATOM_MAX=",intToString(N_ATOM_MAX)),
                NULL};
        execve("./Atom", forkArgs, forkEnv);
        printf("Errore Processo Atomo\n");
        return;
    }

    //sendMessage(atomPid, createMessage(2, stringJoin("N_ATOM_MAX=", intToString(N_ATOM_MAX))));
    waitAndLockSemaphore(masterSignalSemaphore);
    sendMessage(PID_MASTER, createMessage(2, stringJoin("atomCreate=", intToString(atomPid))));
    sendSignal(PID_MASTER, SIGUSR1);


    waitAndLockSemaphore(statisticsSemaphore);
    int* statistics = getSharedMemory(sharedMemoryId);
    statistics[SPLIT_AMOUNT]++;
    statistics[ENERGY_AMOUNT] += energy;
    statistics[ENERGY_PRODUCED] += energy;
    boolean explode = statistics[ENERGY_AMOUNT] > ENERGY_EXPLODE_THRESHOLD;
    unlockSemaphore(statisticsSemaphore);

    if (explode)
    {
        waitAndLockSemaphore(masterSignalSemaphore);
        sendMessage(PID_MASTER, createMessage(1, "EXPLODE"));
        sendSignal(PID_MASTER, SIGUSR1);
    }

}

void setAtomFunction()
{
    int N_FUNCTION = getAtomFunction(N_ATOMICO,N_ATOM_MAX);

    if (N_FUNCTION == 0)
    {
        atomSplitFunction = &atomSplitFunction0;
    }
    else if(N_FUNCTION == 1)
    {
        atomSplitFunction = &atomSplitFunction1;
    }
    else
    {
        atomSplitFunction = &atomSplitFunction2;
    }
}

boolean isTerminating()
{
    int semId = getSemaphore(TERMINATING_SEMAPHORE);
    int memId = getSharedMemoryId(TERMINATING_SHARED_MEMORY, sizeof(int));
    waitAndLockSemaphore(semId);
    int* mem = getSharedMemory(memId);
    boolean term = mem[0];
    unlockSemaphore(semId);
    printf("ATOM TERMINATING %i\n", term);
    return term;
}
*/