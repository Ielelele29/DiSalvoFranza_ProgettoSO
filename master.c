#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>
#include <time.h>
#include "StringUtils.h"
#include "MessageUtils.h"
#include "SemaphoreUtils.h"
#include "SignalUtils.h"
#include "SharedMemoryUtils.h"
#include "NodeUtils.h"
#include "NumberUtils.h"

//Variabili da config
int ENERGY_DEMAND = -1;
int ENERGY_EXPLODE_THRESHOLD = -1;
int N_NUOVI_ATOMI = -1;
int N_ATOMI_INIT = -1;
int N_ATOM_MAX = -1;
int MIN_N_ATOMICO = -1;
int STEP = -1;
int SIM_DURATION = -1;

//Pid processi creati
pid_t supplyPid = -1;
pid_t activatorPid = -1;
pid_t inhibitorPid = -1;
Atom atoms;

struct timespec sleepTime, remaining;

int isInhibitorActive = -1;


//Code di messaggi
int messageReceiveChannel = -1;

//Memoria condivisa
int statisticsSharedMemoryId = -1;
int inhibitorSharedMemoryId = -1;

//Semafori
int signalSemaphore = -1;
int statisticsSemaphore = -1;
int inhibitorSemaphore = -1;

//Statistiche
int cumulativeStatistics[10] = {0};

void readConfig();
void printStatistics();
void onReceiveMessage(int sig);
void onActivateDeactivateInhibitor(int sig);
void terminate(TerminationType reason);

int createSupply();
int createActivator();
int createInhibitor();
int createAtom();

int main() {
    printf("Hello, World 4!\n");
    //Lettura configurazione
    readConfig();


    printf("ENERGY_DEMAND = %i\n", ENERGY_DEMAND);
    printf("ENERGY_EXPLODE_THRESHOLD = %i\n", ENERGY_EXPLODE_THRESHOLD);
    printf("N_NUOVI_ATOMI = %i\n", N_NUOVI_ATOMI);
    printf("N_ATOMI_INIT = %i\n", N_ATOMI_INIT);
    printf("N_ATOM_MAX = %i\n", N_ATOM_MAX);
    printf("MIN_N_ATOMICO = %i\n", MIN_N_ATOMICO);
    printf("STEP = %i\n", STEP);
    printf("SIM_DURATION = %i\n", SIM_DURATION);


    //Richiesta di attivazione inibitore
    char input[1];
    while (isInhibitorActive != 0 && isInhibitorActive != 1)
    {
        printf("Inserisci 1 per attivare l'inibitore, e 0 per bloccarlo\n");
        scanf("%s", input);
        //    fgets(input, sizeof(input), stdin);
        sscanf(input, "%i", &isInhibitorActive);
        if (isInhibitorActive != 0 && isInhibitorActive != 1)
        {
            printf("Non hai inserito un numero valido\n");
        }
    }


    //Inizializzazione memoria condivisa statistiche
    statisticsSemaphore = getSemaphore(STATISTICS_SEMAPHORE);
    statisticsSharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int) * 10);
    clearSharedMemory(statisticsSharedMemoryId);
    unlockSemaphore(statisticsSemaphore);

    //Inizializzazione memoria condivisa inibitore
    inhibitorSemaphore = getSemaphore(INHIBITOR_SEMAPHORE);
    inhibitorSharedMemoryId = getSharedMemoryId(INHIBITOR_SHARED_MEMORY, sizeof(int));
    clearSharedMemory(inhibitorSharedMemoryId);
    int* inhibitorMemory = getSharedMemory(inhibitorSharedMemoryId);
    inhibitorMemory[0] = isInhibitorActive;
    unlockSemaphore(inhibitorSemaphore);
    setSignalAction(SIGINT, onActivateDeactivateInhibitor);

    //Inizializzazione ricezione messaggi tramite segnali
    signalSemaphore = getSemaphore(MASTER_SIGNAL_SEMAPHORE);
    setSignalAction(SIGUSR1, onReceiveMessage);
    messageReceiveChannel = getMessageId(getpid());
    unlockSemaphore(signalSemaphore);





    //Creazione processi ausiliari
    createSupply();
    createActivator();
    createInhibitor(isInhibitorActive);

    for (int i = 0; i < N_ATOMI_INIT; i++)
    {
        createAtom();
    }

/*
    Atom readingAtom = atoms;
    while (readingAtom != NULL)
    {
        printf("Atom PID = %i\n", getNodeValue(readingAtom));
        if (!hasNextNode(atoms))
        {
            break;
        }
        readingAtom = getNextNode(readingAtom);
    }*/

    //Inizio simulazione
    sendMessage(supplyPid, createMessage(1, "start"));
    sendMessage(activatorPid, createMessage(1, "start"));
    sendMessage(inhibitorPid, createMessage(1, "start"));
/*
    for (int i = 0; i < 10; i++)
    {
        sleepTime.tv_sec = 10;
        printf("StartPause\n");
        nanosleep(&sleepTime, &remaining);
        printf("EndPause\n");
    }*/
    while (SIM_DURATION > 0)
    {
        sleepTime.tv_sec = 1;
        nanosleep(&sleepTime, &remaining);
        printStatistics();
        SIM_DURATION--;
    }

    //Fine simulazione
    terminate(TIMEOUT);
    return 0;
}

void printStatistics()
{
    //Ottenimento accesso alla memoria condivisa
    waitAndLockSemaphore(statisticsSemaphore);
    int* statistics = getSharedMemory(statisticsSharedMemoryId);

    //Stampa dati riguardanti le operazioni relative all'ultimo secondo
    printf("\n\nOperazioni nell'ultimo secondo:\n");
    printf("Attivazione/i dell'attivatore: %i\n", statistics[ACTIVATION_AMOUNT]);
    printf("Numero di scissioni: %i\n", statistics[SPLIT_AMOUNT]);
    printf("Energia prodotta: %i\n", statistics[ENERGY_PRODUCED]);
    printf("Energia consumata: %i\n", statistics[ENERGY_CONSUMED]);
    printf("Atomi creati: %i\n", statistics[CREATED_ATOMS]);
    printf("Scorie: %i\n", statistics[DEAD_ATOMS]);
    printf("\n-----INIBITORE-----\n");
    printf("Scissioni degli atomi rallentate: %i\n", statistics[DELAYED_ATOM_SPLIT]);
    printf("Terminazioni per EXPLODE evitate: %i\n", statistics[AVOIDED_EXPLOSIONS]);
    printf("Terminazioni per MELTDOWNS evitate: %i\n", statistics[AVOIDED_MELTDOWNS]);

    //Aggiunta alle statistiche totali delle statistiche dell'ultimo secondo
    for (int i = 0; i < 9; i++)
    {
        cumulativeStatistics[i] += statistics[i];
    }

    //Stampa dati riguardanti tutte le operazioni dall'avvio del programma
    printf("\n\nOperazioni dall'avvio del programma:\n");
    printf("Attivazione/i dell'attivatore: %i\n", cumulativeStatistics[ACTIVATION_AMOUNT]);
    printf("Numero di scissioni: %i\n", cumulativeStatistics[SPLIT_AMOUNT]);
    printf("Energia prodotta: %i\n", cumulativeStatistics[ENERGY_PRODUCED]);
    printf("Energia consumata: %i\n", cumulativeStatistics[ENERGY_CONSUMED]);
    printf("Atomi creati: %i\n", cumulativeStatistics[CREATED_ATOMS]);
    printf("Scorie: %i\n", cumulativeStatistics[DEAD_ATOMS]);
    printf("\n-----INIBITORE-----\n");
    printf("Scissioni degli atomi rallentate: %i\n", cumulativeStatistics[DELAYED_ATOM_SPLIT]);
    printf("Terminazioni per EXPLODE evitate: %i\n", cumulativeStatistics[AVOIDED_EXPLOSIONS]);
    printf("Terminazioni per MELTDOWNS evitate: %i\n", cumulativeStatistics[AVOIDED_MELTDOWNS]);

    //Reset memoria condivisa, controllo e prelievo ENERGY_DEMAND
    clearSharedMemory(statisticsSharedMemoryId);
    if (cumulativeStatistics[ENERGY_AMOUNT] >= ENERGY_DEMAND)
    {
        statistics[ENERGY_CONSUMED] += ENERGY_DEMAND;
        cumulativeStatistics[ENERGY_AMOUNT] -= ENERGY_DEMAND;
        statistics[ENERGY_AMOUNT] = cumulativeStatistics[ENERGY_AMOUNT];
    }
    else
    {
        terminate(BLACKOUT);
    }
    unlockSemaphore(statisticsSemaphore);
}

void terminate(TerminationType reason)
{
    if (reason == TIMEOUT)
    {
        printf("Spegnimento programma... (TIMEOUT)\n");
    }
    else if (reason == EXPLODE)
    {
        printf("Spegnimento programma... (EXPLODE)\n");
    }
    else if (reason == BLACKOUT)
    {
        printf("Spegnimento programma... (BLACKOUT)\n");
    }
    else if (reason == MELTDOWN)
    {
        printf("Spegnimento programma... (MELTDOWN)\n");
    }

    //Terminazione atomi
    while (atoms != NULL)
    {
        int pid = atoms->value;
        sendMessage(pid, createMessage(1, "term"));
        killMessageChannel(pid);
        atoms = getNextNode(atoms);
    }

    //Chiusura risorse IPC
    sendMessage(supplyPid, createMessage(1, "term"));
    killMessageChannel(supplyPid);
    sendMessage(activatorPid, createMessage(1, "term"));
    killMessageChannel(activatorPid);
    sendMessage(inhibitorPid, createMessage(1, "term"));
    killMessageChannel(inhibitorPid);
    killMessageChannel(getpid());
    deleteSharedMemory(inhibitorSharedMemoryId);
    deleteSharedMemory(statisticsSharedMemoryId);
    deleteSemaphore(signalSemaphore);
    deleteSemaphore(statisticsSemaphore);
    exit(0);
}

void onActivateDeactivateInhibitor(int sig)
{
    if (sig == SIGINT)
    {
        waitAndLockSemaphore(inhibitorSemaphore);
        int* memory = getSharedMemory(inhibitorSharedMemoryId);
        if (isInhibitorActive)
        {
            printf("\nInibitore disattivato!\n");
        }
        else
        {
            printf("\nInibitore attivato!\n");
        }
        isInhibitorActive = !isInhibitorActive;
        *memory = isInhibitorActive;
        unlockSemaphore(inhibitorSemaphore);
        nanosleep(&remaining, &remaining);
    }
}

void onReceiveMessage(int sig)
{
    if (sig == SIGUSR1)
    {
        Message message = createEmptyMessage();
        while (msgrcv(messageReceiveChannel, &message, sizeof(message), 0, IPC_NOWAIT) != -1)
        {
            if (message.messageType == 1)
            {
                if (stringEquals(message.messageText, "EXPLODE"))
                {
                    terminate(EXPLODE);
                }
                else if (stringEquals(message.messageText, "MELTDOWN"))
                {
                    terminate(MELTDOWN);
                }
            }
            else if (message.messageType == 2)
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
                else if (stringEquals(request, "atomCreate"))
                {
                    int processPid = atoi(process);
                    if (processPid > 0)
                    {
                        addNode(&atoms, processPid);
                    }
                }
                free(request);
                free(process);
            }
        }
        unlockSemaphore(signalSemaphore);
    }
}

int createSupply()
{
    printf("Creazione processo Alimentazione...\n");
    pid_t pid = fork();
    if (pid < 0) //Errore di creazione della fork
    {
        printf("Errore durante la creazione del processo Alimentazione\n");
        return -1;
    }
    else if (pid == 0) //Processo Alimentazione
    {
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {
                stringJoin("N_NUOVI_ATOMI=", intToString(N_NUOVI_ATOMI)),
                stringJoin("MIN_N_ATOMICO=", intToString(MIN_N_ATOMICO)),
                stringJoin("N_ATOM_MAX=", intToString(N_ATOM_MAX)),
                stringJoin("STEP=", intToString(STEP)),
                NULL};
        printf("Processo Alimentazione creato correttamente\n");
        execve("./Supply", forkArgs, forkEnv);
        printf("Errore Processo Alimentazione\n");
        return -1;
    }

    //Processo Master
    supplyPid = pid;
    return 0;
}

int createActivator()
{
    printf("Creazione processo Attivatore...\n");
    pid_t pid = fork();
    if (pid < 0) //Errore di creazione della fork
    {
        printf("Errore durante la creazione del processo Attivatore\n");
        return -1;
    }
    else if (pid == 0) //Processo Attivatore
    {
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {
                stringJoin("STEP=", intToString(STEP)),
                NULL};
        printf("Processo Attivatore creato correttamente\n");
        execve("./Activator", forkArgs, forkEnv);
        printf("Errore Processo Attivatore\n");
        return -1;
    }

    //Processo Master
    activatorPid = pid;
    return 0;
}

int createInhibitor()
{
    printf("Creazione processo Inibitore...\n");
    pid_t pid = fork();
    if (pid == -1) //Errore di creazione della fork
    {
        printf("Errore durante la creazione del processo Inibitore\n");
        return -1;
    }
    else if (pid == 0) //Processo Inibitore
    {
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {
                stringJoin("ENERGY_EXPLODE_THRESHOLD=", intToString(ENERGY_EXPLODE_THRESHOLD)),
                NULL};
        printf("Processo Inibitore creato correttamente\n");
        execve("./Inhibitor", forkArgs, forkEnv);
        printf("Errore Processo Inibitore\n");
        return -1;
    }

    //Processo Master
    inhibitorPid = pid;
    return 0;
}

int createAtom()
{
    printf("Creazione processo Atomo...\n");
    pid_t atomPid = fork();
    if (atomPid == -1) //Errore di creazione della fork
    {
        printf("Errore durante la creazione del processo Atomo\n");
        return -1;
    }
    else if (atomPid == 0) //Processo Atomo
    {
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {
                stringJoin("ENERGY_EXPLODE_THRESHOLD=", intToString(ENERGY_EXPLODE_THRESHOLD)),
                stringJoin("MIN_N_ATOMICO=", intToString(MIN_N_ATOMICO)),
                stringJoin("N_ATOMICO=", intToString(getRandomIntBetween(MIN_N_ATOMICO, N_ATOM_MAX))),
                NULL};
        printf("Processo Atomo creato correttamente\n");
        execve("./Atom", forkArgs, forkEnv);
        printf("Errore Processo Atomo\n");
        return -1;
    }

    //Processo Master
    addNode(&atoms, atomPid);
    return 0;
}




void readLine(char* line)
{
    //Pulizia linea da spazi ed altri caratteri extra
    line = stringClearChar(line, ' ');
    if (stringEndsWith(line, "\0"))
    {
        line = stringCut(line, 0, stringLength(line) - 1);
    }
    if (stringEndsWith(line, "\n"))
    {
        line = stringCut(line, 0, stringLength(line) - 1);
    }

    //Ottenimento valori prima e dopo l'uguale
    char* key = stringBefore(line, "=");
    char* value = stringAfter(line, "=");

    //Impostazione valori delle variabili lette da config
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
