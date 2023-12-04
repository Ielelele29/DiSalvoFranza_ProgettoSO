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
int N_NUOVI_ATOMI = -1;

int main() {

    key_t key = getpid();
    int msgId = msgget(key, IPC_CREAT | 0644);
    Message message = createEmptyMessage();
    while(true)
    {
        msgrcv(msgId, &message, sizeof(message), 0, 0);
        if(message.messageType == 1)
        {
            if (stringEquals(message.messageText, "tick"))
            {
                tick();
            }
            else if (stringEquals(message.messageText, "term"))
            {
                break;
            }
            else
            {
                printf("Error invalid message!\n");
                break;
            }
        }
        else if (message.messageType == 2)
        {
            // message N_NUOVI_ATOMI
            if(stringStartsWith(message.messageText,"N_NUOVI_ATOMI="))
            {
                N_NUOVI_ATOMI = atoi(stringAfter(message.messageText,"N_NUOVI_ATOMI="));
            }
            else
            {
                printf("Error invalid message!\n");
                break;
            }
        }
        else
        {
            printf("Error invalid message!\n");
            break;
        }
    }

    return 0;
}

void tick(){
    printf("Supply tick\n");

    printf("Ogni STEP nanosecondi\n");

}