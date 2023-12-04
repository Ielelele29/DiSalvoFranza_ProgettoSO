//
// Created by lelelele29 on 04/12/23.
//

#ifndef DISALVOFRANZA_PROGETTOSO_MESSAGEUTILS_H
#define DISALVOFRANZA_PROGETTOSO_MESSAGEUTILS_H

#include "CustomTypes.h"

Message createMessage(long type, const char* string);

Message createEmptyMessage();

void sendMessage(pid_t targetPid, Message message);

void killMessageChannel(pid_t targetPid);

#endif //DISALVOFRANZA_PROGETTOSO_MESSAGEUTILS_H
