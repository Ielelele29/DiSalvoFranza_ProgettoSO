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

void tick();
void createAtoms();
void waitNano();
int N_NUOVI_ATOMI= -1;
int MIN_N_ATOMICO = -1;
int STEP = -1;
int N_ATOM_MAX= -1;
int ENERGY_EXPLODE_THRESHOLD = -1;
int PID_INHIBITOR = -1;
extern char **environ;

int main() {

    ignoreSignal(SIGINT);

    char** env = environ;
    while (*env != NULL)
    {
        if(stringStartsWith(*env,"N_NUOVI_ATOMI="))
        {
            N_NUOVI_ATOMI = atoi(stringAfter(*env,"N_NUOVI_ATOMI="));
        }
        else if(stringStartsWith(*env,"STEP="))
        {
            STEP = atoi(stringAfter(*env,"STEP="));
        }
        else if(stringStartsWith(*env,"PID_INHIBITOR="))
        {
            PID_INHIBITOR = atoi(stringAfter(*env,"PID_INHIBITOR="));
        }
        else if(stringStartsWith(*env,"ENERGY_EXPLODE_THRESHOLD="))
        {
            ENERGY_EXPLODE_THRESHOLD = atoi(stringAfter(*env,"ENERGY_EXPLODE_THRESHOLD="));
        }
        else if(stringStartsWith(*env,"N_ATOM_MAX="))
        {
            N_ATOM_MAX = atoi(stringAfter(*env,"N_ATOM_MAX="));
        }
        else if(stringStartsWith(*env,"MIN_N_ATOMICO="))
        {
            MIN_N_ATOMICO = atoi(stringAfter(*env,"MIN_N_ATOMICO="));
        }
        else
        {
            printf("Error wrong environ data");
        }
        *env++;
    }

    int msgId = getMessageId(getpid());
    Message message = createEmptyMessage();
    while(true)
    {
        if(msgrcv(msgId, &message, sizeof(message), 0, 0) != -1)
        {
            /*if (message.messageType == 2)
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
        int sem = getSemaphore(MASTER_SIGNAL_SEMAPHORE);
        //sendMessage(atomPid, createMessage(2, stringJoin("N_ATOM_MAX=", intToString(N_ATOM_MAX))));
        waitAndLockSemaphore(sem);
        sendMessage(pidMaster, createMessage(2, stringJoin("atomCreate=", intToString(atomPid))));
        i++;
    }
    sendSignal(pidMaster, SIGUSR1);

}