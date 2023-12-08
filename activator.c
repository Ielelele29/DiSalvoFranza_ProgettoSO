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

void tick();
void split();
int MIN_N_ATOMICO = -1;
int ENERGY_EXPLODE_THRESHOLD = -1;

int main() {

    int msgId = getMessageId(getpid());
    Message message = createEmptyMessage();
    while(true)
    {
        if(msgrcv(msgId, &message, sizeof(message), 0, 0) != -1)
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
                split();
            }

        }
    }
}

void split()
{
    // prelevo pid che mi servono;
    pid_t pidMaster = getppid();
    int sem = getSemaphore(MASTER_SIGNAL_SEMAPHORE);
    waitAndLockSemaphore(sem);
    sendMessage(pidMaster, createMessage(stringJoin("atomList=", intToString(getpid()))));
    sendSignal(pidMaster, SIGUSR1);


    sendMessage(atomoPid, createMessage(2,stringJoin("ENERGY_EXPLODE_THRESHOLD=", intToString(ENERGY_EXPLODE_THRESHOLD)));
    sendMessage(atomoPid, createMessage(2,stringJoin("MIN_N_ATOMICO=", intToString(MIN_N_ATOMICO)));
    sendMessage(atomoPid, createMessage(1,"split"););

}