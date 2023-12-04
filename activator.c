#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <string.h>
#include "StringUtils.h"
#include "MessageUtils.h"

int main() {

    key_t key = 10002;
    int msgId = msgget(key, IPC_CREAT | 0644);

    errno = 0;
    printf("id = %i\n", msgId);
    Message message = createEmptyMessage();
    msgrcv(msgId, &message, sizeof(message), 1, 0);
    printf("Error = %s\n", strerror(errno));
    printf("Messaggio con id %li arrivato = %s", message.messageType, message.messageText);
    return 0;
}