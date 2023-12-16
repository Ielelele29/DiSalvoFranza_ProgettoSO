#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#define _GNU_SOURCE
#include "CustomTypes.h"
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include "StringUtils.h"
#include "MessageUtils.h"
#include "SemaphoreUtils.h"
#include "SignalUtils.h"
#include <signal.h>
#include "NumberUtils.h"
#include "NodeUtils.h"
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
boolean checkMessage();
void createAtom();



void checkError(int sig)
{
    printf("Supply error\n\n\n");
}

int main()
{
    //Segnali
    ignoreSignal(SIGINT);
    setSignalAction(SIGSEGV, checkError);

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

    printf("Master secondo supply = %i\n", masterMessageChannelId);
    sendMessage(masterMessageChannelId, createMessage(1, "SupplyReady"));
    listenMessage();

    killMessageChannel(supplyMessageChannelId);
    detachFromSharedMemory(statisticsSharedMemoryId);
    sendMessage(masterMessageChannelId, createMessage(1, "SupplyStop"));
    printf("END SUPPLY\n");
    return 0;
}



void generateAtoms()
{
    if (checkMessage())
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
}

void createAtom()
{
 //   printf("Creazione atomo da Supply\n");
    pid_t atomPid = fork();
    if (atomPid == -1)
    {
        printf("Errore durante la creazione del processo Atomo\n");
        sendMessage(masterMessageChannelId, createMessage(1, "Meltdown"));
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

boolean checkMessage()
{
    Message message = createEmptyMessage();
    int result = msgrcv(supplyMessageChannelId, &message, sizeof(message), 0, IPC_NOWAIT);
    if (result != -1)
    {
        if (message.messageType == 1)
        {
            if (stringEquals(message.messageText, "Stop"))
            {
                return false;
            }
        }
    }
    return true;
}











































































/*
void tick();
void createAtoms();
void waitNano();
int N_NUOVI_ATOMI= -1;
int MIN_N_ATOMICO = -1;
int STEP = -1;
int N_ATOM_MAX= -1;
int ENERGY_EXPLODE_THRESHOLD = -1;
int PID_INHIBITOR = -1;
int messageChannelId = -1;
extern char **environ;

int main() {

    ignoreSignal(SIGINT);

    char** env = environ;
    while (*env != NULL)
    {
        if(stringStartsWith(*env,"N_NUOVI_ATOMI="))
        {
            N_NUOVI_ATOMI = atoi(stringAfter(*env,"="));
        }
        else if(stringStartsWith(*env,"STEP="))
        {
            STEP = atoi(stringAfter(*env,"="));
        }
        else if(stringStartsWith(*env,"PID_INHIBITOR="))
        {
            PID_INHIBITOR = atoi(stringAfter(*env,"="));
        }
        else if(stringStartsWith(*env,"ENERGY_EXPLODE_THRESHOLD="))
        {
            ENERGY_EXPLODE_THRESHOLD = atoi(stringAfter(*env,"="));
        }
        else if(stringStartsWith(*env,"N_ATOM_MAX="))
        {
            N_ATOM_MAX = atoi(stringAfter(*env,"="));
        }
        else if(stringStartsWith(*env,"MIN_N_ATOMICO="))
        {
            MIN_N_ATOMICO = atoi(stringAfter(*env,"="));
        }
        else
        {
            printf("Error wrong environ data");
        }
        *env++;
    }

    messageChannelId = getMessageId(getpid());
    while(true)
    {
        Message message = createEmptyMessage();
        if(msgrcv(messageChannelId, &message, sizeof(message), 0, 0) != -1)
        {
            printf("Messagione supply %s \n", message.messageText);
            if (message.messageType == 2)
            {
                if(stringStartsWith(message.messageText,"N_NUOVI_ATOMI="))
                {
                    N_NUOVI_ATOMI = atoi(stringAfter(message.messageText,"N_NUOVI_ATOMI="));
                }
                else if(stringStartsWith(message.messageText,"STEP="))
                {
                    STEP = atoi(stringAfter(message.messageText,"STEP="));
                    tick();
                    break;
                }
                else if(stringStartsWith(message.messageText,"N_ATOM_MAX="))
                {
                    N_ATOM_MAX = atoi(stringAfter(message.messageText,"N_ATOM_MAX="));
                }
                else
                {
                    printf("Error invalid message!\n");
                    printf("Waiting for a new message...\n");

                }
            }
            else
            {
                printf("Error invalid message!(invalid type of message)\n");
                printf("Waiting for a new message...\n");
            }
            if (message.messageType == 1)
            {
                if(stringStartsWith(message.messageText,"start"))
                {
                    tick();
                    break;
                }
                else
                {
                    printf("Error receiving message!\n");
                    printf("Waiting for a new message...\n");
                    printf("tricheco1\n");
                }
            }
            else
            {
                printf("Error invalid message!\n");
                printf("Waiting for a new message...\n");
                printf("tricheco2\n");
            }
        }
        else
        {
            printf("Error receiving message!\n");
            printf("Waiting for a new message...\n");
            printf("tricheco3\n");
        }
    }
    killMessageChannel(getpid());
    printf("SUPPLY SPENTO\n");
    return 0;
}

void tick(){

    messageChannelId = getMessageId(getpid());

    while(true)
    {
        Message message = createEmptyMessage();
        if(msgrcv(messageChannelId, &message, sizeof(message), 1, IPC_NOWAIT) == -1)
        {
            waitNano();
            createAtoms();
        }
        else
        {
            if (stringEquals(message.messageText, "term"))
            {
                printf("Power termination!\n");
                break;
            }
            else
            {
                waitNano();
                createAtoms();
            }

        }
    }
}

void waitNano()
{
    struct timespec timeToSleep;
    timeToSleep.tv_sec = STEP/1000000000;
    timeToSleep.tv_nsec = STEP%1000000000;
    nanosleep(&timeToSleep, NULL);
}

void createAtoms()
{
    int i = 0;
    printf("Crea %d_NUOVI_ATOMI \n", N_NUOVI_ATOMI);
    while(i < N_NUOVI_ATOMI)
    {
        printf("Creazione processo Atomo da Supply...\n");
        pid_t atomPid = fork();
        if (atomPid == -1)
        {
            printf("Errore durante la creazione del processo Atomo\n");
            //TODO meltdown
        }
        else if (atomPid == 0)
        {
            printf("FORKATO SUPPLY\n");
            int N_ATOMICO = getRandomIntBetween(MIN_N_ATOMICO, N_ATOM_MAX);
            char *forkArgs[] = {NULL};
            char* forkEnv[] = {
                    stringJoin("ENERGY_EXPLODE_THRESHOLD=", intToString(ENERGY_EXPLODE_THRESHOLD)),
                    stringJoin("MIN_N_ATOMICO=", intToString(MIN_N_ATOMICO)),
                    stringJoin("N_ATOMICO=", intToString(N_ATOMICO)),
                    stringJoin("PID_MASTER=",intToString(getppid())),
                    stringJoin("N_ATOM_MAX=",intToString(N_ATOM_MAX)),
                    stringJoin("PID_INHIBITOR=", intToString(PID_INHIBITOR)),
                    NULL};
            printf("Creo il %d° processo atomo con supply\n", i+1);
            execve("./Atom", forkArgs, forkEnv);
            printf("Errore Processo Atomo\n");
            return;
        }

        //sendMessage(atomPid, createMessage(2, stringJoin("N_ATOM_MAX=", intToString(N_ATOM_MAX))));
        i++;
    }
}*/