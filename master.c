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
#include "KeyUtils.h"


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

void checkError(int sig)
{
    printf("Master error\n\n\n");
    exit(0);
}

int main()
{
    printf("Hello, World 4! PID = %i\n", getpid());
    printf("pidMaster secondo master %i\n", getpid());

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
    while (isInhibitorActive != 0 && isInhibitorActive != 1)
    {
        printf("Inserisci 1 per attivare l'inibitore, e 0 per disattivarlo\n");
        scanf("%s", input);
        //    fgets(input, sizeof(input), stdin);
        sscanf(input, "%i", &isInhibitorActive);
        if (isInhibitorActive != 0 && isInhibitorActive != 1)
        {
            printf("Non hai inserito un numero valido\n");
        }
    }

    //Pid processi
    masterPid = getpid();

    //Segnali
    setSignalAction(SIGINT, onActivateDeactivateInhibitor);
    setSignalAction(SIGALRM, onPrintStatistics);
//    setSignalAction(SIGSEGV, checkError);


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
            printf("BREKK\n");
            break;
        }
    }
    printf("MEX\n");
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
      //      printf("[Master] Messaggio ricevuto: %s\n", message.messageText);
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
     //               printf("[Master] Quantità atomi: %i\n", nodeSize(atoms));
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
        //                printf("Sending\n");
                        sendMessage(inhibitorMessageChannelId, createMessage(1, "Meltdown"));
         //               printf("Sended\n");
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
    //                    printf("%s Fase 2\n", pidS);
                        sendMessage(inhibitorMessageChannelId, createMessage(2, stringJoin("AtomSplit=", valueS)));
                        message = createEmptyMessage();
           //             printf("WAIT MESSAGE\n");
                        result = msgrcv(masterInhibitorMessageChannelId, &message, sizeof(message), 4, 0);
          //              printf("%i MESSAGE = %s\n", result, message.messageText);
                        if (result != -1)
                        {
                            sendMessage(atomMessageChannelId, createMessage(4, message.messageText));
                        }
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
                        result = msgrcv(masterInhibitorMessageChannelId, &message, sizeof(message), 5, 0);
                        if (result != -1)
                        {
                            sendMessage(atomMessageChannelId, createMessage(2, message.messageText));
                        }
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
        printf("[Master] Last message = %s\n", message.messageText);
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
            if (stringEquals(key, "AtomCreate"))
            {
                sendSignal(atoi(value), SIGUSR1);
            }
            else if (stringEquals(key, "AtomDie"))
            {
                processes--;
                atoms = removeNode(searchNodeValue(atoms, atoi(value)));
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
        printf("[Master] Atom stop message = %s     Remaining: %i\n", message.messageText, processes-1);
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
            if (stringEquals(key, "AtomDie"))
            {
                int pidValue = atoi(value);
                if (pidValue == pid)
                {
                    processes--;
                    atoms = removeNode(searchNodeValue(atoms, atoi(value)));
                    removed = true;
                }
            }
            else if (stringEquals(key, "AtomCreate"))
            {
                sendSignal(atoi(value), SIGUSR1);
            }
            free(key);
            free(value);
            if (removed)
            {
                return;
            }
        }
        checkStopAtomsMessages(pid);
    }/*
    if (processes != 0)
    {
        checkStopAtomsMessages();
    }*/
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
/*
    sendMessage(supplyMessageChannelId, createMessage(1, "Stop"));
    sendMessage(activatorMessageChannelId, createMessage(1, "Stop"));
    sendMessage(inhibitorMessageChannelId, createMessage(1, "Stop"));*/

    sendSignal(supplyPid, SIGUSR1);
    sendSignal(activatorPid, SIGUSR1);
    sendSignal(inhibitorPid, SIGUSR1);
    sleep(3);
    checkLastMessages();

    //Terminazione atomi
    Atom readingAtom = getFirstNode(atoms);
    printf("STOPPING ATOMS %i\n", nodeSize(readingAtom));
 //   printValues(readingAtom);
    while (readingAtom != NULL)
    {
        int atomPid = readingAtom->value;
        sendSignal(atomPid, SIGUSR1);
    //    sendMessage(getMessageId(atomPid), createMessage(1, "Stop"));
        readingAtom = getNextNode(readingAtom);
        checkStopAtomsMessages(atomPid);
    }
    Message message = createEmptyMessage();
    while (msgrcv(masterMessageChannelId, &message, sizeof(message), 0, IPC_NOWAIT) != -1)
    {
        message = createEmptyMessage();
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
    printf("Creazione processo Alimentazione...\n");
    pid_t pid = fork();
    if (pid < 0) //Errore di creazione della fork
    {
   //     printf("Errore durante la creazione del processo Alimentazione\n");
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
   //     printf("Errore durante la creazione del processo Attivatore\n");
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
   //     printf("Errore durante la creazione del processo Inibitore\n");
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
 //   printf("Creazione processo Atomo...\n");
    pid_t atomPid = fork();
    if (atomPid == -1) //Errore di creazione della fork
    {
//        printf("Errore durante la creazione del processo Atomo\n");
        printf("MELTDOWN %i\n", masterMessageChannelId);
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
 //       printf("Processo Atomo creato correttamente\n");
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





























































































/*
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
int terminatingSharedMemoryId = -1;

//Semafori
int signalSemaphore = -1;
int statisticsSemaphore = -1;
int inhibitorSemaphore = -1;
int terminatingSemaphore = -1;

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
    printf("pidMaster secondo masterato1spazio %i\n", getpid());
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

    //Inizializzazione memoria condivisa terminazione
    terminatingSemaphore = getSemaphore(TERMINATING_SEMAPHORE);
    terminatingSharedMemoryId = getSharedMemoryId(TERMINATING_SHARED_MEMORY, sizeof(int));
    clearSharedMemory(terminatingSharedMemoryId);
    int* terminatingMemory = getSharedMemory(terminatingSharedMemoryId);
    terminatingMemory[0] = false;
    unlockSemaphore(terminatingSemaphore);

    //Inizializzazione ricezione messaggi tramite segnali
    signalSemaphore = getSemaphore(MASTER_SIGNAL_SEMAPHORE);
    setSignalAction(SIGUSR1, onReceiveMessage);
    messageReceiveChannel = getMessageId(getpid());
    unlockSemaphore(signalSemaphore);





    //Creazione processi ausiliari
    //createInhibitor(isInhibitorActive);
    createSupply();
    //createActivator();


    for (int i = 0; i < N_ATOMI_INIT; i++)
    {
        createAtom();
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

    //Inizio simulazione
    sendMessage(supplyPid, createMessage(1, "start"));
    sendMessage(activatorPid, createMessage(1, "start"));
    sendMessage(inhibitorPid, createMessage(1, "start"));

    for (int i = 0; i < 10; i++)
    {
        sleepTime.tv_sec = 10;
        printf("StartPause\n");
        nanosleep(&sleepTime, &remaining);
        printf("EndPause\n");
    }
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
    waitSemaphore(signalSemaphore);
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
    printf("Atomi creati: %i\n", statistics[CREATED_ATOMS]);

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
    waitAndLockSemaphore(terminatingSemaphore);
    int* term = getSharedMemory(terminatingSharedMemoryId);
    term[0] = true;
    unlockSemaphore(terminatingSemaphore);
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

    sendMessage(supplyPid, createMessage(1, "term"));
    sendMessage(activatorPid, createMessage(1, "term"));
    sendMessage(inhibitorPid, createMessage(1, "term"));

    sleep(5);

    //Terminazione atomi
    waitSemaphore(signalSemaphore);
    while (atoms != NULL)
    {
        int pid = atoms->value;
        sendMessage(pid, createMessage(1, "term"));
        atoms = getNextNode(atoms);
    }

    //Chiusura risorse IPC
    printf("Rimozione schifezzuole\n");
    killMessageChannel(getpid());
    deleteSharedMemory(inhibitorSharedMemoryId);
    deleteSharedMemory(statisticsSharedMemoryId);
    deleteSharedMemory(terminatingSharedMemoryId);
    deleteSemaphore(signalSemaphore);
    deleteSemaphore(statisticsSemaphore);
    deleteSemaphore(inhibitorSemaphore);
    deleteSemaphore(terminatingSemaphore);
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
        printf("MASTER RECEIVE SIGNAL\n");
        Message message = createEmptyMessage();
        while (msgrcv(messageReceiveChannel, &message, sizeof(message), 0, IPC_NOWAIT) != -1)
        {
            printf("Messaggio segnalato: %s\n", message.messageText);
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
                        killMessageChannel(processPid);
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
            message = createEmptyMessage();
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
                stringJoin("PID_INHIBITOR=", intToString(inhibitorPid)),
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
        int N_ATOMICO = getRandomIntBetween(MIN_N_ATOMICO, N_ATOM_MAX);
        char* forkArgs[] = {NULL};
        char* forkEnv[] = {
                stringJoin("ENERGY_EXPLODE_THRESHOLD=", intToString(ENERGY_EXPLODE_THRESHOLD)),
                stringJoin("MIN_N_ATOMICO=", intToString(MIN_N_ATOMICO)),
                stringJoin("N_ATOMICO=", intToString(N_ATOMICO)),
                stringJoin("PID_MASTER=",intToString(getpid())),
                stringJoin("N_ATOM_MAX=",intToString(N_ATOM_MAX)),
                stringJoin("PID_INHIBITOR=", intToString(inhibitorPid)),
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
}*/
