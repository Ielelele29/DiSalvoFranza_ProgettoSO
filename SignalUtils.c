//
// Created by lelelele29 on 07/12/23.
//

#include "SignalUtils.h"
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

void setSignalAction(int signal, void (function)(int))
{
    struct sigaction action;
    action.sa_handler = function;
    action.sa_flags = SA_NODEFER;
    sigemptyset(&action.sa_mask);
    sigaction(signal, &action, NULL);

}

void sendSignal(int pid, int signal)
{
    if (kill(pid, signal) != 0)
    {
        perror("Errore nell'invio del segnale");
    }
}