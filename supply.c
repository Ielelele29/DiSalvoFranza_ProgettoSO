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

void tick();
void createAtoms();
void waitNano();
int N_NUOVI_ATOMI= -1;
int STEP = -1;
int N_ATOM_MAX= -1;

int main() {

    int msgId = getMessageId(getpid());
    Message message = createEmptyMessage();
    while(true)
    {
        if(msgrcv(msgId, &message, sizeof(message), 0, 0) != -1)
        {
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
        }
        else
        {
            printf("Error receiving message!\n");
            printf("Waiting for a new message...\n");
        }
    }

    return 0;
}

void tick(){

    int msgId = getMessageId(getpid());
    Message message = createEmptyMessage();

    while(true)
    {
        if(msgrcv(msgId, &message, sizeof(message), 1, IPC_NOWAIT) == -1)
        {
            waitNano();
            createAtoms();
        }
        else {
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
    pid_t pidMaster = getppid();
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
            char *forkArgs[] = {NULL};
            char *forkEnv[] = {NULL};
            printf("Creo il %d° processo atomo con supply\n", i+1);
            execve("./Atom", forkArgs, forkEnv);
            printf("Errore Processo Atomo\n");
            return;
        }
        int sem = getSemaphore(MASTER_SIGNAL_SEMAPHORE);
        sendMessage(atomPid, createMessage(2, stringJoin("N_ATOM_MAX=", intToString(N_ATOM_MAX))));
        waitAndLockSemaphore(sem);
        sendMessage(pidMaster, createMessage(2, stringJoin("atomCreate=", intToString(atomPid))));
        i++;
    }
    sendSignal(pidMaster, SIGUSR1);

}