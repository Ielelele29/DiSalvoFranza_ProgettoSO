#include <stdio.h>
#include "CustomTypes.h"
#include <unistd.h>
#include <time.h>
#define _GNU_SOURCE
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



int MIN_N_ATOMICO = -1;
boolean isActive();
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
int msgId = -1;
extern char **environ;

int main() {

    ignoreSignal(SIGINT);
    sharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int)*10);
    statisticsSemaphore = getSemaphore(STATISTICS_SEMAPHORE);
    inhibitorSemaphore = getSemaphore(INHIBITOR_SEMAPHORE);
    inhibitorSharedMemoryId = getSharedMemoryId(INHIBITOR_SHARED_MEMORY, sizeof(int));

    waitAndLockSemaphore(statisticsSemaphore);
    int* statistics = getSharedMemory(sharedMemoryId);
    statistics[CREATED_ATOMS]++;
    unlockSemaphore(statisticsSemaphore);
    msgId = getMessageId(getpid());

    char** env = environ;
    while (*env != NULL)
    {
        if(stringStartsWith(*env,"N_ATOMICO="))
        {
            N_ATOMICO = atoi(stringAfter(*env,"N_ATOMICO="));
        }
        else if(stringStartsWith(*env,"ENERGY_EXPLODE_THRESHOLD="))
        {
            ENERGY_EXPLODE_THRESHOLD = atoi(stringAfter(*env,"ENERGY_EXPLODE_THRESHOLD="));
        }
        else if(stringStartsWith(*env,"MIN_N_ATOMICO="))
        {
            MIN_N_ATOMICO = atoi(stringAfter(*env,"MIN_N_ATOMICO="));
        }
        else if(stringStartsWith(*env,"PID_INHIBITOR="))
        {
            PID_INHIBITOR = atoi(stringAfter(*env,"PID_INHIBITOR="));
        }
        else if(stringStartsWith(*env,"PID_MASTER="))
        {
            PID_MASTER = atoi(stringAfter(*env,"PID_MASTER="));
        }
        else if(stringStartsWith(*env,"N_ATOM_MAX="))
        {
            N_ATOM_MAX = atoi(stringAfter(*env,"N_ATOM_MAX="));
        }
        else
        {
            printf("Error wrong environ data");
        }

        *env++;
    }

    setAtomFunction();

    Message message = createEmptyMessage();
    while(true)
    {
        if(msgrcv(msgId, &message, sizeof(message), 0, 0) != -1)
        {
            /*if (message.messageType == 2)
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
            }*/
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
                }

            }
            else
            {
                printf("Error invalid message!\n");
                printf("Waiting for a new message...\n");
            }
        }
        else
        {
            printf("Error receiving message!\n");
            printf("Waiting for a new message...\n");
        }
    }

    return 0;
}

void split()
{
    if (N_ATOMICO > MIN_N_ATOMICO)
    {
        /* effettuare scissione*/
        int childAtomicNumber = getRandomIntBetween(MIN_N_ATOMICO,N_ATOMICO);
        N_ATOMICO -= childAtomicNumber;
        setAtomFunction();
        int energy = atomSplitFunction(N_ATOMICO,childAtomicNumber);
        if(isActive())
        {
            sendMessage(PID_INHIBITOR,createMessage(2,stringJoin(intToString(getpid()),stringJoin(";SPLIT_ENERGY_PRODUCED=",intToString(energy)))));
            Message message = createEmptyMessage();
            if(msgrcv(msgId, &message, sizeof(message), 2, 0) != -1)
            {
                if(stringEquals(message.messageText,"1"))
                {

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
            }

        }
        else
        {

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
    int sem = getSemaphore(MASTER_SIGNAL_SEMAPHORE);
    pid_t atomPid = fork();
    if (atomPid == -1)
    {
        printf("Errore durante la creazione del processo Atomo\n");
        if(isActive())
        {
            sendMessage(PID_INHIBITOR,createMessage(2,stringJoin(intToString(getpid()),";MELTDOWN")));
            Message message = createEmptyMessage();
            if(msgrcv(msgId, &message, sizeof(message), 2, 0) != -1)
            {
                if(stringEquals(message.messageText,"1"))
                {
                    waitAndLockSemaphore(sem);
                    sendMessage(PID_MASTER, createMessage(1, "MELTDOWN"));
                    sendSignal(PID_MASTER, SIGUSR1);
                }
            }
            else
            {
                printf("Error receiving message!\n");
                printf("Waiting for a new message...\n");
            }
        }
        else
        {
            waitAndLockSemaphore(sem);
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
                stringJoin("PID_MASTER=",intToString(getppid())),
                stringJoin("N_ATOM_MAX=",intToString(N_ATOM_MAX)),
                NULL};
        execve("./Atom", forkArgs, forkEnv);
        printf("Errore Processo Atomo\n");
        return;
    }

    //sendMessage(atomPid, createMessage(2, stringJoin("N_ATOM_MAX=", intToString(N_ATOM_MAX))));
    waitAndLockSemaphore(sem);
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
        waitAndLockSemaphore(sem);
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
