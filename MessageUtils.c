//
// Created by lelelele29 on 04/12/23.
//

#include "MessageUtils.h"
#include "CustomTypes.h"

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
    message.messageType = 1;
    int i = 0;
    while (i < 100)
    {
        message.messageText[i] = '\0';
        i++;
    }
    return message;
}
