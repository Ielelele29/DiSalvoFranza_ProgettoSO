#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#define _GNU_SOURCE
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include "StringUtils.h"
#include "MessageUtils.h"

void tick();
void createAtoms();
void waitNano();
int N_NUOVI_ATOMI = -1;
int STEP = -1;

int main() {

    key_t key = getpid();
    int msgId = msgget(key, IPC_CREAT | 0644);
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

void tick(){

    key_t key = getpid();
    int msgId = msgget(key, IPC_CREAT | 0644);
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
    printf("Crea N_NUOVI_ATOMI %d\n",N_NUOVI_ATOMI);
}