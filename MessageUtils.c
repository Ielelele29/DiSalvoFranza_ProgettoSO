#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdio.h>
#include "MessageUtils.h"
#include "CustomTypes.h"
#include "KeyUtils.h"

Message createMessage(long type, const char* string)
{
    Message message = createEmptyMessage();
    message.messageType = type;
    int i = 0;
    while (i < 100 && string[i] != '\0')
    {
        message.messageText[i] = string[i];
        i++;
    }
    return message;
}

Message createEmptyMessage()
{
    Message message;
    message.messageType = -1;
    int i = 0;
    while (i < 100)
    {
        message.messageText[i] = '\0';
        i++;
    }
    return message;
}

void sendMessage(pid_t targetPid, Message message)
{
    if (msgsnd(getMessageId(targetPid), &message, sizeof(message), 0) == -1)
    {
        perror("Send message error");
    }
}

int getMessageId(pid_t targetPid)
{
    int id = msgget(getKey(targetPid), IPC_CREAT | 0644);
    if (id == -1)
    {
        perror("Get message channel id error");
    }
    return id;
}

void killMessageChannel(pid_t targetPid)
{
    if (msgctl(getMessageId(targetPid), IPC_RMID, NULL) == -1)
    {
        perror("Kill message channel error");
    }
}
