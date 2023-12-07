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

int N_ATOM_MAX = -1;
int MIN_N_ATOMICO = -1;
boolean split();
int numAtom = 0;


int main() {

    int i = 0;
    int msgId = getMessageId(getpid());
    Message message = createEmptyMessage();
    while(true)
    {
        if(msgrcv(msgId, &message, sizeof(message), 0, 0) != -1)
        {
            if (message.messageType == 2)
            {
                if(stringStartsWith(message.messageText,"N_ATOM_MAX="))
                {
                    N_ATOM_MAX = atoi(stringAfter(message.messageText,"N_ATOM_MAX="));
                    srand(time(NULL));
                    numAtom = rand()%N_ATOM_MAX; //implict +0
                    i++;
                }
                else if(stringStartsWith(message.messageText,"MIN_N_ATOMICO="))
                {
                    MIN_N_ATOMICO = atoi(stringAfter(message.messageText,"MIN_N_ATOMICO="));
                    i++;
                }
                else
                {
                    printf("Error invalid message!\n");
                    printf("Waiting for a new message...\n");

                }
            }
            else if (message.messageType == 1 && i>=2)
            {
                if (stringEquals(message.messageText, "split"))
                {
                    if(split())
                    {
                        printf("Scissione effettuata con successo\n");
                    }
                    else
                    {
                        // TODO killo il processo
                        break;
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
                printf("Error invalid message or not arrived all needed data!\n");
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

boolean split()
{
    if (numAtom > MIN_N_ATOMICO)
    {
        /* effettuare scissione*/
    }
    else
    {
        //TODO comunico scoria
        return false;
        //diventa scoria
    }
}