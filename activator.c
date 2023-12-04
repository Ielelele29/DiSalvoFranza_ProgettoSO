#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include "StringUtils.h"
#include "MessageUtils.h"

void tick();

int main() {

#if 0
    key_t key = 10002;
    int msgId = msgget(key, IPC_CREAT | 0644);

    errno = 0;
    printf("id = %i\n", msgId);
    Message message = createEmptyMessage();
    msgrcv(msgId, &message, sizeof(message), 1, 0);
    printf("Error = %s\n", strerror(errno));
    printf("Messaggio con id %li arrivato = %s", message.messageType, message.messageText);
#endif

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
    // TODO comunicazione scissione atomi
}