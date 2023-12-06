#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "StringUtils.h"
#include "MessageUtils.h"
#include "SemaphoreUtils.h"
#include "AtomUtils.h"

int ENERGY_DEMAND = -1;
int ENERGY_EXPLODE_THRESHOLD = -1;
int N_NUOVI_ATOMI = -1;
int N_ATOMI_INIT = -1;
int N_ATOMI_MAX = -1;
int MIN_N_ATOMICO = -1;
int STEP = -1;
int SIM_DURATION = -1;

pid_t supplyPid = -1;
pid_t activatorPid = -1;

void readConfig();
void tick();

int main() {
    printf("Hello, World 4!\n");
    readConfig();
    printf("ENERGY_DEMAND = %i\n", ENERGY_DEMAND);
    printf("ENERGY_EXPLODE_THRESHOLD = %i\n", ENERGY_EXPLODE_THRESHOLD);
    printf("N_NUOVI_ATOMI = %i\n", N_NUOVI_ATOMI);
    printf("N_ATOMI_INIT = %i\n", N_ATOMI_INIT);
    printf("N_ATOMI_MAX = %i\n", N_ATOMI_MAX);
    printf("MIN_N_ATOMICO = %i\n", MIN_N_ATOMICO);
    printf("STEP = %i\n", STEP);
    printf("SIM_DURATION = %i\n", SIM_DURATION);


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

    int atomMemoryId = createAtomMemory(N_ATOMI_MAX);
    int* atomMemory = getAtomMemory(atomMemoryId);
    int atomSemaphoreId = getAtomSemaphore();
    for (int i = 0; i < N_ATOMI_INIT; i++)
    {
        printf("Creazione processo Atomo...\n");
        int sem = waitAndLockSemaphore(atomSemaphoreId);
        pid_t atomPid = fork();
        if (atomPid == -1)
        {
            printf("Errore durante la creazione del processo Atomo\n");
            return 0;
        }
        else if (atomPid == 0)
        {
            int slot = getFirstFreeAtomSlot(atomMemoryId, N_ATOMI_MAX);
            if (slot != -1 && sem != -1)
            {
                atomMemory[slot] = atomPid;
                unlockSemaphore(atomSemaphoreId);
                char* forkArgs[] = {NULL};
                char* forkEnv[] = {NULL};
                printf("Processo Atomo creato correttamente\n");
                execve("./Atom", forkArgs, forkEnv);
            }
            else
            {
                //TODO TERMINAZIONE PER IMPOSSIBILITA' CREAZIONE ATOMO
            }
            printf("Errore Processo Atomo\n");
            return 0;
        }
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
    return 0;
}

void tick()
{
    printf("Master Tick\n");
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
    else if (stringEquals(key, "N_ATOMI_MAX"))
    {
        N_ATOMI_MAX = atoi(value);
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
        fclose(file);
    }
}
