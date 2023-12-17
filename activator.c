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

//boolean checkMessage();
void listenMessage();
void splitAtoms();
void waitNano();



void checkError(int sig)
{
    printf("Activator error\n\n\n");
}

void onTerminate(int sig)
{
    if (sig == SIGUSR1)
    {
        killMessageChannel(activatorMessageChannelId);
        detachFromSharedMemory(statisticsSharedMemoryId);
        sendMessage(masterMessageChannelId, createMessage(1, "ActivatorStop"));
        printf("END ACTIVATOR\n");
        exit(0);
    }
}

int main()
{
    //Segnali
    ignoreSignal(SIGINT);
    setSignalAction(SIGSEGV, checkError);
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

int i = 0;
void splitAtoms()
{
 //   if (checkMessage())
    {
        printf("START SPLIT ATOMS %i\n\n", masterMessageChannelId);
        Atom atoms = NULL;
        sendMessage(masterMessageChannelId, createMessage(1,"AtomList"));
        while(true)
        {
            Message message = createEmptyMessage();
            int result = msgrcv(activatorMessageChannelId, &message, sizeof(message), 0, 0);
            if (result != -1)
            {
     //           printf("Activator message = %s\n", message.messageText);
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

        int j = 0;
        while (atoms != NULL)
        {
            if ((i+j)%3 == 0)
            {
                sendMessage(getMessageId(getNodeValue(atoms)), createMessage(1, "Split"));
                waitAndLockSemaphore(statisticsSemaphoreId);
                int* statistics = getSharedMemory(statisticsSharedMemoryId);
                statistics[ACTIVATION_AMOUNT]++;
                unlockSemaphore(statisticsSemaphoreId);
            }
            atoms = getNextNode(atoms);
            j++;
        }
        i++;
        waitNano();
        splitAtoms();
    }
}

void waitNano()
{
    double nanoTime = 1000000000;//step*(getRandom()+0.4);
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
                splitAtoms();
                return;
            }
        }
    }
    listenMessage();
}
/*
boolean checkMessage()
{
    Message message = createEmptyMessage();
    int result = msgrcv(activatorMessageChannelId, &message, sizeof(message), 0, IPC_NOWAIT);
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
}*/















































































/*
void tick();
void split();
int STEP = -1;
int indice = 0;
int messageChannelId = -1;
int sharedMemoryId = -1;
int statisticsSemaphore = -1;
int masterSignalSemaphore = -1;
extern char **environ;

int main() {

    ignoreSignal(SIGINT);
    sharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int)*10);
    statisticsSemaphore = getSemaphore(STATISTICS_SEMAPHORE);
    sem = getSemaphore(MASTER_SIGNAL_SEMAPHORE);

    char** env = environ;
    while (*env != NULL)
    {
        if (stringStartsWith(*env,"STEP="))
        {
            STEP = atoi(stringAfter(*env, "="));
        }
        else
        {
            printf("Error wrong environ data");
        }
        *env++;
    }

    messageChannelId = getMessageId(getpid());
    while (true)
    {
        Message message = createEmptyMessage();
        if (msgrcv(messageChannelId, &message, sizeof(message), 1, 0) != -1)
        {
            if (message.messageType == 2)
            {
                if(stringStartsWith(message.messageText,"MIN_N_ATOMICO="))
                {
                    MIN_N_ATOMICO = atoi(stringAfter(message.messageText, "MIN_N_ATOMICO="));
                    tick();
                    break;
                }
                else if(stringStartsWith(message.messageText,"ENERGY_EXPLODE_THRESHOLD="))
                {
                    ENERGY_EXPLODE_THRESHOLD = atoi(stringAfter(message.messageText, "ENERGY_EXPLODE_THRESHOLD="));
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
                printf("Error invalid message!(invalid type of message)\n");
                printf("Waiting for a new message...\n");
            }
            if (message.messageType == 1)
            {
                if (stringStartsWith(message.messageText,"start"))
                {
                    tick();
                    break;
                }
                else
                {
                    printf("Error receiving message!\n");
                    printf("Waiting for a new message...\n");
                    printf("CuloAllegro1\n");
                }

            }
            else
            {
                printf("Error receiving message!\n");
                printf("Waiting for a new message...\n");
                printf("CuloAllegro2\n");
            }
        }
        else
        {
            printf("Error receiving message!\n");
            printf("Waiting for a new message...\n");
            printf("CuloAllegro3\n");
        }
    }
    killMessageChannel(getpid());
    return 0;
}

void waitNano()
{
    double nanoTime = STEP*(getRandom()+0.4);
    printf("s to sleep %i\n", (int)nanoTime/1000000000);
    printf("ns to sleep %i\n", (int)nanoTime%1000000000);
    struct timespec timeToSleep;
    timeToSleep.tv_sec = (int)nanoTime/1000000000;
    timeToSleep.tv_nsec = (int)nanoTime%1000000000;
    nanosleep(&timeToSleep, NULL);
}

void tick(){

    messageChannelId = getMessageId(getpid());

    while (true)
    {
        Message message = createEmptyMessage();
        if (msgrcv(messageChannelId, &message, sizeof(message), 1, IPC_NOWAIT) == -1)
        {
            waitNano();
            split();
        }
        else
        {
            if (stringEquals(message.messageText, "term"))
            {
                printf("Activator termination!\n");
                break;
            }
            else
            {
                waitNano();
                split();
            }

        }
    }
}

void split()
{
    Atom atoms = NULL;
    // prelevo pid che mi servono;
    pid_t pidMaster = getppid();
    pid_t pidActivator = getpid();
    waitAndLockSemaphore(sem);
    sendMessage(pidMaster, createMessage(2,stringJoin("atomList=", intToString(getpid()))));
    sendSignal(pidMaster, SIGUSR1);
    messageChannelId = getMessageId(getpid());
    printf("pidMaster secondo activator %i\n", pidMaster);
    while(true)
    {
        Message message = createEmptyMessage();
        int msgr = msgrcv(messageChannelId, &message, sizeof(message), 2, 0);
        if (msgr != -1)
        {
            printf("mESSAGGIO ACT = %s\n", message.messageText);
            if(stringStartsWith(message.messageText,"atomPid="))
            {
                addNode(&atoms,atoi(stringAfter(message.messageText, "=")));
            }
            else if (stringEquals(message.messageText,"atomEnd"))
            {
                break;
            }
            else
            {
                printf("Error receiving message!\n");
                printf("Waiting for a new message...\n");
                printf("CuloAllegroDiverso1\n");
            }
        }
        else
        {
            perror("Errore messaggio: \n");
            printf("Error receiving message!\n");
            printf("Waiting for a new message...\n");
            printf("CuloAllegroDiverso2\n");
        }
    }


    int j = 0;

    printf("Nodesime = %i\n", nodeSize(atoms));
    while (j < nodeSize(atoms))
    {
        printf("j = %i %i %i %i\n", j, indice, j+indice, (j+indice)%3);
        if ((j+indice)%3 == 0)
        {
            sendMessage(atoms->value, createMessage(1,"split"));
            printf("locking masterSignalSemaphore\n");

            waitAndLockSemaphore(statisticsSemaphore);
            printf("locked masterSignalSemaphore\n");

            int* statistics = getSharedMemory(sharedMemoryId);
            statistics[ACTIVATION_AMOUNT]++;
            unlockSemaphore(statisticsSemaphore);
            printf("unlock masterSignalSemaphore\n");

        }
        atoms = getNextNode(atoms);
        j++;
    }
    indice = indice+1%3;


    //sendMessage(atomoPid, createMessage(2,stringJoin("ENERGY_EXPLODE_THRESHOLD=", intToString(ENERGY_EXPLODE_THRESHOLD)));
    //sendMessage(atomoPid, createMessage(2,stringJoin("MIN_N_ATOMICO=", intToString(MIN_N_ATOMICO)));


}*/