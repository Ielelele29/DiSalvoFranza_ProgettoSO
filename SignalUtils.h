//
// Created by lelelele29 on 07/12/23.
//

#ifndef DISALVOFRANZA_PROGETTOSO_SIGNALUTILS_H
#define DISALVOFRANZA_PROGETTOSO_SIGNALUTILS_H

void setSignalAction(int signal, void (function)());

void sendSignal(int pid, int signal);


#endif //DISALVOFRANZA_PROGETTOSO_SIGNALUTILS_H
