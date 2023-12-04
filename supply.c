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

int main() {

    key_t key = getpid();
    int msgId = msgget(key, IPC_CREAT | 0644);
    Message message = createEmptyMessage();
    while(true)
    {
        msgrcv(msgId, &message, sizeof(message), 1, 0);
        if (stringEquals(message.messageText, "tick") == 1)
        {
            tick();
        }
        else if (stringEquals(message.messageText, "term") == 1)
        {
            break;
        }
        else
        {
            printf("Error = %s\n", strerror(errno));
            break;
        }

    }




    return 0;
}

void tick(){
    printf("Supply tick\n");

    printf("Ogni STEP nanosecondi\n");

}