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
    if (targetPid != -1)
    {
        if (msgsnd(getMessageId(targetPid), &message, sizeof(message), 0) == -1)
        {
            perror("Send message error");
        }
    }
    else
    {
        printf("Send message pid error: %i\nMessage: %s\n", targetPid, message.messageText);
    }
}

int getMessageId(pid_t targetPid)
{
    if (targetPid != -1)
    {
        int id = msgget(getKey(targetPid), IPC_CREAT | 0644);
        if (id == -1)
        {
            perror("Get message channel id error");
        }
        return id;
    }
    return -1;
}

void killMessageChannel(pid_t targetPid)
{
    if (targetPid != -1)
    {
        if (msgctl(getMessageId(targetPid), IPC_RMID, NULL) == -1)
        {
            perror("Kill message channel error");
        }
    }
}
