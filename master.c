#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/msg.h>
#include "StringUtils.h"
#include "MessageUtils.h"
#include "SemaphoreUtils.h"
#include "SignalUtils.h"
#include "SharedMemoryUtils.h"
#include "NodeUtils.h"
#include "NumberUtils.h"
#include "ConfigUtils.h"


//Pid processi
pid_t masterPid = -1;
pid_t supplyPid = -1;
pid_t activatorPid = -1;
pid_t inhibitorPid = -1;
Atom atoms;

//Code di messaggi
int masterMessageChannelId = -1;
int supplyMessageChannelId = -1;
int activatorMessageChannelId = -1;
int inhibitorMessageChannelId = -1;
int masterInhibitorMessageChannelId = -1;

//Memoria condivisa
int statisticsSharedMemoryId = -1;

//Semafori
int statisticsSemaphoreId = -1;

//Variabili
boolean isActivatorReady = false;
boolean isSupplyReady = false;
boolean isInhibitorReady = false;

boolean isTerminating = false;
boolean isInhibitorActive = -1;
int cumulativeStatistics[11] = {0};
int processes = 0;

int initialAtoms = -1;
int energyDemand = -1;
int remainingTime = -1;
int minAtomicNumber = -1;
int maxAtomicNumber = -1;


void checkStart();
boolean checkStop();
void listenMessages();
void onPrintStatistics(int sig);
void onActivateDeactivateInhibitor(int sig);
void terminate(TerminationType reason);

int createSupply();
int createActivator();
int createInhibitor();
int createAtom();

int main()
{
    //Lettura configurazione
    createConfig();
    energyDemand = getConfigValue(ENERGY_DEMAND);
    remainingTime = getConfigValue(SIM_DURATION);
    minAtomicNumber = getConfigValue(MIN_N_ATOMICO);
    maxAtomicNumber = getConfigValue(N_ATOM_MAX);
    initialAtoms = getConfigValue(N_ATOMI_INIT);

    printf("ENERGY_DEMAND = %i\n", getConfigValue(ENERGY_DEMAND));
    printf("ENERGY_EXPLODE_THRESHOLD = %i\n", getConfigValue(ENERGY_EXPLODE_THRESHOLD));
    printf("N_NUOVI_ATOMI = %i\n", getConfigValue(N_NUOVI_ATOMI));
    printf("N_ATOMI_INIT = %i\n", getConfigValue(N_ATOMI_INIT));
    printf("N_ATOM_MAX = %i\n", getConfigValue(N_ATOM_MAX));
    printf("MIN_N_ATOMICO = %i\n", getConfigValue(MIN_N_ATOMICO));
    printf("STEP = %i\n", getConfigValue(STEP));
    printf("SIM_DURATION = %i\n", getConfigValue(SIM_DURATION));
    unloadConfig();


    //Richiesta di attivazione inibitore
    char input[1];
    int i = -1;
    while (i != 0 && i != 1)
    {
        printf("Inserisci 1 per attivare l'inibitore, e 0 per disattivarlo. In futuro potrai attivarlo e disattivarlo con Ctrl+c.\n");
        scanf("%s", input);
        //    fgets(input, sizeof(input), stdin);
        sscanf(input, "%i", &i);
        if (i != 0 && i != 1)
        {
            printf("Non hai inserito un numero valido\n");
        }
    }
    isInhibitorActive = i == 1 ? true : false;

    //Pid processi
    masterPid = getpid();

    //Segnali
    setSignalAction(SIGINT, onActivateDeactivateInhibitor);
    setSignalAction(SIGALRM, onPrintStatistics);

    //Creazione semafori
    statisticsSemaphoreId = getSemaphore(STATISTICS_SEMAPHORE);

    //Memoria condivisa
    statisticsSharedMemoryId = getSharedMemoryId(STATISTICS_SHARED_MEMORY, sizeof(int)*11);
    clearSharedMemory(statisticsSharedMemoryId);

    //Sblocco semafori
    unlockSemaphore(statisticsSemaphoreId);

    processes += createInhibitor() + 1;
    processes += createSupply() + 1;
    processes += createActivator() + 1;

    //Code di messaggi
    masterMessageChannelId = getMessageId(masterPid);
    supplyMessageChannelId = getMessageId(supplyPid);
    activatorMessageChannelId = getMessageId(activatorPid);
    inhibitorMessageChannelId = getMessageId(inhibitorPid);
    masterInhibitorMessageChannelId = getMessageId(MaxPid+1);

    for (int i = 0; i < initialAtoms; i++)
    {
        if (createAtom() == -1)
        {
            break;
        }
    }
    listenMessages();
    return 0;
}

void listenMessages()
{
    if (masterMessageChannelId != -1)
    {
        Message message = createEmptyMessage();
        int result = msgrcv(masterMessageChannelId, &message, sizeof(message), 0, 0);
        if (result != -1)
        {
            if (message.messageType == 1)
            {
                if (stringEquals(message.messageText, "SupplyReady"))
                {
                    isSupplyReady = true;
                    checkStart();
                }
                else if (stringEquals(message.messageText, "ActivatorReady"))
                {
                    isActivatorReady = true;
                    checkStart();
                }
                else if (stringEquals(message.messageText, "InhibitorReady"))
                {
                    isInhibitorReady = true;
                    checkStart();
                }
                else if (stringEquals(message.messageText, "AtomList"))
                {
                    Atom readingAtom = atoms;
                    while (readingAtom != NULL)
                    {
                        sendMessage(activatorMessageChannelId, createMessage(2, stringJoin("AtomPid=", intToString(getNodeValue(readingAtom)))));
                        readingAtom = getNextNode(readingAtom);
                    }
                    sendMessage(activatorMessageChannelId, createMessage(1, "AtomEnd"));
                }
                else if (stringEquals(message.messageText, "Meltdown"))
                {
                    if (isInhibitorActive)
                    {
                        sendMessage(inhibitorMessageChannelId, createMessage(1, "Meltdown"));
                    }
                    else
                    {
                        terminate(MELTDOWN);
                    }
                }
                else if (stringEquals(message.messageText, "Explode"))
                {
                    if (isInhibitorActive)
                    {
                        sendMessage(inhibitorMessageChannelId, createMessage(1, "Explode"));
                    }
                    else
                    {
                        terminate(EXPLODE);
                    }
                }
            }
            else if (message.messageType == 2)
            {
                char* key = stringBefore(message.messageText, "=");
                char* value = stringAfter(message.messageText, "=");
                if (stringEquals(key, "AtomCreate"))
                {
                    processes++;
                    addNode(&atoms, atoi(value));
                }
                else if (stringEquals(key, "AtomDie"))
                {
                    processes--;
                    atoms = removeNode(searchNodeValue(atoms, atoi(value)));
                }
                free(key);
                free(value);
            }
            else if (message.messageType == 3)
            {
                char* pidS = stringBefore(message.messageText, ";");
                char* text = stringAfter(message.messageText, ";");
                char* key = stringBefore(text, "=");
                char* valueS = stringAfter(text, "=");
                if (stringEquals(key, "AtomSplit"))
                {
                    int atomMessageChannelId = getMessageId(atoi(pidS));
                    if (isInhibitorActive)
                    {
                        sendMessage(inhibitorMessageChannelId, createMessage(2, stringJoin("AtomSplit=", valueS)));
                        message = createEmptyMessage();
                        result = -1;
                        while (result == -1)
                        {
                            result = msgrcv(masterInhibitorMessageChannelId, &message, sizeof(message), 4, 0);
                        }
                        sendMessage(atomMessageChannelId, createMessage(4, message.messageText));
                    }
                    else
                    {
                        sendMessage(atomMessageChannelId, createMessage(4, "1"));
                    }
                }
                else if (stringEquals(key, "AtomEnergy"))
                {
                    int atomMessageChannelId = getMessageId(atoi(pidS));
                    if (isInhibitorActive)
                    {
                        sendMessage(inhibitorMessageChannelId, createMessage(2, stringJoin("AtomEnergy=", valueS)));
                        message = createEmptyMessage();
                        result = -1;
                        while (result == -1)
                        {
                            result = msgrcv(masterInhibitorMessageChannelId, &message, sizeof(message), 5, 0);
                        }
                        sendMessage(atomMessageChannelId, createMessage(2, message.messageText));
                    }
                    else
                    {
                        sendMessage(atomMessageChannelId, createMessage(2, stringJoin("AtomEnergy=", valueS)));
                    }
                }
                free(pidS);
                free(text);
                free(key);
                free(valueS);
            }
        }
        listenMessages();
    }
}

void checkLastMessages()
{
    Message message = createEmptyMessage();
    int result = msgrcv(masterMessageChannelId, &message, sizeof(message), 0, 0);
    if (result != -1)
    {
        if (message.messageType == 1)
        {
            if (stringEquals(message.messageText, "SupplyStop"))
            {
                isSupplyReady = false;
                processes--;
            }
            else if (stringEquals(message.messageText, "ActivatorStop"))
            {
                isActivatorReady = false;
                processes--;
            }
            else if (stringEquals(message.messageText, "InhibitorStop"))
            {
                isInhibitorReady = false;
                processes--;
            }
            else if (stringEquals(message.messageText, "AtomList"))
            {
                sendSignal(activatorPid, SIGUSR1);
            }
        }
        else if (message.messageType == 2)
        {
            char* key = stringBefore(message.messageText, "=");
            char* value = stringAfter(message.messageText, "=");
            int pidValue = atoi(value);
            if (stringEquals(key, "AtomCreate"))
            {
                sendSignal(pidValue, SIGUSR1);
            }
            else if (stringEquals(key, "AtomDie"))
            {
                processes--;
                Atom toRemove = searchNodeValue(atoms, pidValue);
                if (toRemove != NULL)
                {
                    atoms = removeNode(toRemove);
                }
            }
            free(key);
            free(value);
        }
    }
    if (!checkStop())
    {
        checkLastMessages();
    }
}

void checkStopAtomsMessages(int pid)
{
    Message message = createEmptyMessage();
    int result = msgrcv(masterMessageChannelId, &message, sizeof(message), 0, 0);
    if (result != -1)
    {
        if (message.messageType == 1)
        {
            if (stringEquals(message.messageText, "SupplyStop"))
            {
                isSupplyReady = false;
                processes--;
            }
            else if (stringEquals(message.messageText, "ActivatorStop"))
            {
                isActivatorReady = false;
                processes--;
            }
            else if (stringEquals(message.messageText, "InhibitorStop"))
            {
                isInhibitorReady = false;
                processes--;
            }
        }
        else if (message.messageType == 2)
        {
            boolean removed = false;
            char* key = stringBefore(message.messageText, "=");
            char* value = stringAfter(message.messageText, "=");
            int pidValue = atoi(value);
            if (stringEquals(key, "AtomDie"))
            {
                processes--;
                Atom toRemove = searchNodeValue(atoms, pidValue);
                if (toRemove != NULL)
                {
                    atoms = removeNode(toRemove);
                }
                if (pidValue == pid)
                {
                    removed = true;
                }
            }
            else if (stringEquals(key, "AtomCreate"))
            {
                sendSignal(pidValue, SIGUSR1);
            }
            free(key);
            free(value);
            if (removed)
            {
                return;
            }
        }
        checkStopAtomsMessages(pid);
    }
}

void checkStart()
{
    if (isSupplyReady && isActivatorReady && isInhibitorReady)
    {
        sendMessage(supplyMessageChannelId, createMessage(1, "Start"));
        sendMessage(activatorMessageChannelId, createMessage(1, "Start"));
        sendMessage(inhibitorMessageChannelId, createMessage(1, "Start"));
        alarm(1);
    }
}

boolean checkStop()
{
    if (!isSupplyReady && !isActivatorReady && !isInhibitorReady)
    {
        return true;
    }
    return false;
}

void onPrintStatistics(int sig)
{
    if (sig == SIGALRM && !isTerminating)
    {
        //Ottenimento accesso alla memoria condivisa
        waitAndLockSemaphore(statisticsSemaphoreId);
        int* statistics = getSharedMemory(statisticsSharedMemoryId);
        statistics[ENERGY_CONSUMED] += energyDemand;

        //Stampa dati riguardanti le operazioni relative all'ultimo secondo
        printf("\n\nOperazioni nell'ultimo secondo:\n");
        printf("Attivazione/i dell'attivatore: %i\n", statistics[ACTIVATION_AMOUNT]);
        printf("Numero di scissioni: %i\n", statistics[SPLIT_AMOUNT]);
        printf("Energia prodotta: %i\n", statistics[ENERGY_PRODUCED]);
        printf("Energia consumata: %i\n", statistics[ENERGY_CONSUMED]);
        printf("Atomi creati: %i\n", statistics[CREATED_ATOMS]);
        printf("Scorie: %i\n", statistics[DEAD_ATOMS]);
        printf("[Inibitore] Scissioni degli atomi rallentate: %i\n", statistics[DELAYED_ATOM_SPLIT]);
        printf("[Inibitore] Terminazioni per EXPLODE evitate: %i\n", statistics[AVOIDED_EXPLOSIONS]);
        printf("[Inibitore] Terminazioni per MELTDOWNS evitate: %i\n", statistics[AVOIDED_MELTDOWNS]);
        printf("[Inibitore] Energia assorbita: %i\n", statistics[ABSORBED_ENERGY]);

        //Aggiunta alle statistiche totali delle statistiche dell'ultimo secondo
        for (int i = 0; i < 11; i++)
        {
            cumulativeStatistics[i] += statistics[i];
        }

        cumulativeStatistics[ENERGY_AMOUNT] -= energyDemand;

        //Stampa dati riguardanti tutte le operazioni dall'avvio del programma
        printf("\n\nOperazioni dall'avvio del programma:\n");
        printf("Attivazione/i dell'attivatore: %i\n", cumulativeStatistics[ACTIVATION_AMOUNT]);
        printf("Numero di scissioni: %i\n", cumulativeStatistics[SPLIT_AMOUNT]);
        printf("Energia prodotta: %i\n", cumulativeStatistics[ENERGY_PRODUCED]);
        printf("Energia consumata: %i\n", cumulativeStatistics[ENERGY_CONSUMED]);
        printf("Energia attuale: %i\n", cumulativeStatistics[ENERGY_AMOUNT]);
        printf("Atomi creati: %i\n", cumulativeStatistics[CREATED_ATOMS]);
        printf("Scorie: %i\n", cumulativeStatistics[DEAD_ATOMS]);
        printf("[Inibitore] Scissioni degli atomi rallentate: %i\n", cumulativeStatistics[DELAYED_ATOM_SPLIT]);
        printf("[Inibitore] Terminazioni per EXPLODE evitate: %i\n", cumulativeStatistics[AVOIDED_EXPLOSIONS]);
        printf("[Inibitore] Terminazioni per MELTDOWNS evitate: %i\n", cumulativeStatistics[AVOIDED_MELTDOWNS]);
        printf("[Inibitore] Energia assorbita: %i\n", cumulativeStatistics[ABSORBED_ENERGY]);

        //Reset memoria condivisa, controllo e prelievo ENERGY_DEMAND
        clearSharedMemory(statisticsSharedMemoryId);
        if (cumulativeStatistics[ENERGY_AMOUNT] < 0)
        {
            terminate(BLACKOUT);
        }
        statistics[ENERGY_AMOUNT] = cumulativeStatistics[ENERGY_AMOUNT];
        cumulativeStatistics[ENERGY_AMOUNT] = 0;
        unlockSemaphore(statisticsSemaphoreId);
        remainingTime--;
        if (remainingTime > 0)
        {
            alarm(1);
        }
        else
        {
            terminate(TIMEOUT);
        }
    }
}

void terminate(TerminationType reason)
{
    isTerminating = true;
    ignoreSignal(SIGALRM);
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

    sendSignal(supplyPid, SIGUSR1);
    sendSignal(activatorPid, SIGUSR1);
    sendSignal(inhibitorPid, SIGUSR1);
    unlockSemaphore(statisticsSemaphoreId);
    checkLastMessages();

    //Terminazione atomi
    atoms = getFirstNode(atoms);
    Atom lastAtom;
    while (atoms != NULL)
    {
        lastAtom = atoms;
        int atomPid = atoms->value;
        sendSignal(atomPid, SIGUSR1);
        checkStopAtomsMessages(atomPid);
        if (atoms == lastAtom)
        {
            atoms = getNextNode(atoms);
        }
    }

    //Chiusura risorse IPC
    killMessageChannel(masterMessageChannelId);
    detachFromSharedMemory(statisticsSharedMemoryId);
    deleteSharedMemory(statisticsSharedMemoryId);
    deleteSemaphore(statisticsSemaphoreId);
    deleteConfig();
    exit(0);
}

void onActivateDeactivateInhibitor(int sig)
{
    if (sig == SIGINT)
    {
        if (isInhibitorActive)
        {
            printf("\nInibitore disattivato!\n");
        }
        else
        {
            printf("\nInibitore attivato!\n");
        }
        isInhibitorActive = !isInhibitorActive;
    }
}

int createSupply()
{
    pid_t pid = fork();
    if (pid < 0) //Errore di creazione della fork
    {
        if (isInhibitorActive)
        {
            sendMessage(inhibitorMessageChannelId, createMessage(1, "Meltdown"));
        }
        else
        {
            terminate(MELTDOWN);
        }
        return -1;
    }
    else if (pid == 0) //Processo Alimentazione
    {
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {NULL};
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
    pid_t pid = fork();
    if (pid < 0) //Errore di creazione della fork
    {
        if (isInhibitorActive)
        {
            sendMessage(inhibitorMessageChannelId, createMessage(1, "Meltdown"));
        }
        else
        {
            terminate(MELTDOWN);
        }
        return -1;
    }
    else if (pid == 0) //Processo Attivatore
    {
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {NULL};
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
    pid_t pid = fork();
    if (pid == -1) //Errore di creazione della fork
    {
        if (isInhibitorActive)
        {
            sendMessage(inhibitorMessageChannelId, createMessage(1, "Meltdown"));
        }
        else
        {
            terminate(MELTDOWN);
        }
        return -1;
    }
    else if (pid == 0) //Processo Inibitore
    {
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {NULL};
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
    pid_t atomPid = fork();
    if (atomPid == -1) //Errore di creazione della fork
    {
        if (isInhibitorActive)
        {
            sendMessage(inhibitorMessageChannelId, createMessage(1, "Meltdown"));
        }
        else
        {
            terminate(MELTDOWN);
        }
        return -1;
    }
    else if (atomPid == 0) //Processo Atomo
    {
        int atomicNumber = getRandomIntBetween(minAtomicNumber, maxAtomicNumber);
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {
                stringJoin("MasterPid=", intToString(masterPid)),
                stringJoin("AtomicNumber=", intToString(atomicNumber)),
                "AlreadyAdded",
                NULL};
        execve("./Atom", forkArgs, forkEnv);
        printf("Errore Processo Atomo\n");
        removeNode(searchNodeValue(atoms, getpid()));
        processes--;
        return -1;
    }
    addNode(&atoms, atomPid);
    processes++;
    return 0;
}