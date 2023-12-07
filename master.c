#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>
#include "StringUtils.h"
#include "MessageUtils.h"
#include "SemaphoreUtils.h"
#include "SignalUtils.h"

int ENERGY_DEMAND = -1;
int ENERGY_EXPLODE_THRESHOLD = -1;
int N_NUOVI_ATOMI = -1;
int N_ATOMI_INIT = -1;
int N_ATOM_MAX = -1;
int MIN_N_ATOMICO = -1;
int STEP = -1;
int SIM_DURATION = -1;

pid_t supplyPid = -1;
pid_t activatorPid = -1;

int messageReceiveChannel;

Atom atoms;

void readConfig();
void tick();
void onReceiveMessage(int sig);

int main() {
    printf("Hello, World 4!\n");

    readConfig();
    printf("ENERGY_DEMAND = %i\n", ENERGY_DEMAND);
    printf("ENERGY_EXPLODE_THRESHOLD = %i\n", ENERGY_EXPLODE_THRESHOLD);
    printf("N_NUOVI_ATOMI = %i\n", N_NUOVI_ATOMI);
    printf("N_ATOMI_INIT = %i\n", N_ATOMI_INIT);
    printf("N_ATOM_MAX = %i\n", N_ATOM_MAX);
    printf("MIN_N_ATOMICO = %i\n", MIN_N_ATOMICO);
    printf("STEP = %i\n", STEP);
    printf("SIM_DURATION = %i\n", SIM_DURATION);


    messageReceiveChannel = msgget(getpid(), IPC_CREAT | 0644);
    setSignalAction(SIGUSR1, onReceiveMessage);
    int signalSemaphore = getSemaphore(MASTER_SIGNAL_SEMAPHORE);
    unlockSemaphore(signalSemaphore);
    printf("pid padre %i", getpid());
    /*
    printf("Creazione processo Alimentazione...\n");
    pid_t pid = fork();
    if (pid < 0)
    {
        printf("Errore durante la creazione del processo Alimentazione\n");
        return 0;
    }
    else if (pid == 0)
    {
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {NULL};
        printf("Processo Alimentazione creato correttamente\n");
        execve("./Supply", forkArgs, forkEnv);
        printf("Errore Processo Alimentazione\n");
        return 0;
    }
    supplyPid = pid;
    sendMessage(supplyPid, createMessage(2, stringJoin("N_NUOVI_ATOMI=", intToString(N_NUOVI_ATOMI))));
    sendMessage(supplyPid, createMessage(2, stringJoin("N_ATOM_MAX=", intToString(N_ATOM_MAX))));

    printf("riga = %s\n", stringJoin("N_NUOVI_ATOMI=", intToString(N_NUOVI_ATOMI)));

    printf("Creazione processo Attivatore...\n");
    pid = fork();
    if (pid < 0)
    {
        printf("Errore durante la creazione del processo Attivatore\n");
        return 0;
    }
    else if (pid == 0)
    {
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {NULL};
        printf("Processo Attivatore creato correttamente\n");
        execve("./Activator", forkArgs, forkEnv);
        printf("Errore Processo Attivatore\n");
        return 0;
    }
    activatorPid = pid;
    sendMessage(activatorPid, createMessage(2, stringJoin("MIN_N_ATOMICO=", intToString(MIN_N_ATOMICO))));
*/

    for (int i = 0; i < N_ATOMI_INIT; i++)
    {
        printf("Creazione processo Atomo...\n");
        sleep(1);
        pid_t atomPid = fork();
        if (atomPid == -1)
        {
            printf("Errore durante la creazione del processo Atomo\n");
            return 0;
        }
        else if (atomPid == 0)
        {
            char* forkArgs[] = {NULL};
            char* forkEnv[] = {NULL};
            printf("Processo Atomo creato correttamente\n");
            execve("./Atom", forkArgs, forkEnv);
            printf("Errore Processo Atomo\n");
            return 0;
        }
        addNode(&atoms, atomPid);
    }


    Atom readingAtom = atoms;
    while (readingAtom != NULL)
    {
        printf("Atom PID = %i\n", getNodeValue(readingAtom));
        if (!hasNextNode(atoms))
        {
            break;
        }
        readingAtom = getNextNode(readingAtom);
    }
    sendMessage(supplyPid, createMessage(2, stringJoin("STEP=", intToString(STEP))));
    while (SIM_DURATION > 0)
    {
        tick();
        sleep(1);
        SIM_DURATION--;
    }
    sendMessage(supplyPid, createMessage(1, "term"));
    killMessageChannel(supplyPid);
    sendMessage(activatorPid, createMessage(1, "term"));
    killMessageChannel(activatorPid);
    killMessageChannel(messageReceiveChannel);
    deleteSemaphore(signalSemaphore);
    return 0;
}

void tick()
{
    printf("Master Tick\n");
}

void onReceiveMessage(int sig)
{
    Message message = createEmptyMessage();
    if (msgrcv(messageReceiveChannel, &message, sizeof(message), 2, IPC_NOWAIT) != -1)
    {
        char* request = stringBefore(message.messageText, "=");
        char* process = stringAfter(message.messageText, "=");
        if (stringEquals(request, "atomList"))
        {
            int processPid = atoi(process);
            Atom readingAtom = atoms;
            while (readingAtom != NULL)
            {
                sendMessage(processPid, createMessage(2, stringJoin("atomPid=", intToString(readingAtom->value))));
                readingAtom = readingAtom->nextNode;
            }
            sendMessage(processPid, createMessage(2, "atomEnd"));
        }
        else if (stringEquals(request, "atomKill"))
        {
            int processPid = atoi(process);
            Atom toDelete = searchNodeValue(atoms, processPid);
            if (toDelete != NULL)
            {
                removeNode(toDelete);
            }
        }
        free(request);
        free(process);
    }
    int semId = getSemaphore(MASTER_SIGNAL_SEMAPHORE);
    unlockSemaphore(semId);
}




void readLine(char* line)
{
    line = stringClearChar(line, ' ');
    if (stringEndsWith(line, "\0"))
    {
        line = stringCut(line, 0, stringLength(line) - 1);
    }
    if (stringEndsWith(line, "\n"))
    {
        line = stringCut(line, 0, stringLength(line) - 1);
    }
    char* key = stringBefore(line, "=");
    char* value = stringAfter(line, "=");

    if (stringEquals(key, "ENERGY_DEMAND"))
    {
        ENERGY_DEMAND = atoi(value);
    }
    else if (stringEquals(key, "ENERGY_EXPLODE_THRESHOLD"))
    {
        ENERGY_EXPLODE_THRESHOLD = atoi(value);
    }
    else if (stringEquals(key, "N_NUOVI_ATOMI"))
    {
        N_NUOVI_ATOMI = atoi(value);
    }
    else if (stringEquals(key, "N_ATOMI_INIT"))
    {
        N_ATOMI_INIT = atoi(value);
    }
    else if (stringEquals(key, "N_ATOM_MAX"))
    {
        N_ATOM_MAX = atoi(value);
    }
    else if (stringEquals(key, "MIN_N_ATOMICO"))
    {
        MIN_N_ATOMICO = atoi(value);
    }
    else if (stringEquals(key, "STEP"))
    {
        STEP = atoi(value);
    }
    else if (stringEquals(key, "SIM_DURATION"))
    {
        SIM_DURATION = atoi(value);
    }
    free(key);
    free(value);
}

void readConfig()
{
    FILE* file = fopen("./config.txt", "r");
    if (file != NULL)
    {
        char* line = emptyString(100);
        while (fgets(line, 100, file) != NULL)
        {
            readLine(line);
        }
        free(line);
        fclose(file);
    }
}
