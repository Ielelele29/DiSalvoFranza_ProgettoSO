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

void tick();
void split();
int STEP = -1;
int indice = 0;
int msgId = -1;
int sharedMemoryId = -1;
int statisticsSemaphore = -1;
int sem = -1;
extern char **environ;

int main() {

    ignoreSignal(SIGINT);
    sharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int)*10);
    statisticsSemaphore = getSemaphore(STATISTICS_SEMAPHORE);
    sem = getSemaphore(MASTER_SIGNAL_SEMAPHORE);

    char** env = environ;
    while (*env != NULL)
    {
        if(stringStartsWith(*env,"STEP="))
        {
            STEP = atoi(stringAfter(*env,"="));
        }
        else
        {
            printf("Error wrong environ data");
        }
        *env++;
    }

    msgId = getMessageId(getpid());
    Message message = createEmptyMessage();
    while(true)
    {
        if(msgrcv(msgId, &message, sizeof(message), 0, 0) != -1)
        {
            /*if (message.messageType == 2)
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
            }*/
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

    return 0;
}

void waitNano()
{
    double nanoTime = STEP*(getRandom()+0.4);
    struct timespec timeToSleep;
    timeToSleep.tv_sec = (int)nanoTime/1000000000;
    timeToSleep.tv_nsec = (int)nanoTime%1000000000;
    nanosleep(&timeToSleep, NULL);
}

void tick(){

    msgId = getMessageId(getpid());

    while(true)
    {
        Message message = createEmptyMessage();
        if(msgrcv(msgId, &message, sizeof(message), 1, IPC_NOWAIT) == -1)
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
    msgId = getMessageId(pidMaster);
    printf("pidMaster secondo activator %i\n", pidMaster);
    while(true)
    {
        Message message = createEmptyMessage();
        if(msgrcv(msgId, &message, sizeof(message), 2, 0) == -1)
        {
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
            printf("Error receiving message!\n");
            printf("Waiting for a new message...\n");
            printf("CuloAllegroDiverso2\n");
        }
    }


    int i = 0;

    while(i < nodeSize(atoms))
    {
        if(i+indice%3 == 0)
        {
            sendMessage(atoms->value, createMessage(1,"split"));
            waitAndLockSemaphore(statisticsSemaphore);
            int* statistics = getSharedMemory(sharedMemoryId);
            statistics[ACTIVATION_AMOUNT]++;
            unlockSemaphore(statisticsSemaphore);
        }
        atoms = getNextNode(atoms);
        i++;
    }
    indice = indice+i%3;


    //sendMessage(atomoPid, createMessage(2,stringJoin("ENERGY_EXPLODE_THRESHOLD=", intToString(ENERGY_EXPLODE_THRESHOLD)));
    //sendMessage(atomoPid, createMessage(2,stringJoin("MIN_N_ATOMICO=", intToString(MIN_N_ATOMICO)));


}