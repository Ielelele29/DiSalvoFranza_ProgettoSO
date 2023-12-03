#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "StringUtils.h"

int main() {
    printf("Hello, World 4!\n");
    char* newString = string("prova");
    printf("%i", stringEndsWith(newString, ""));
    return 0;
}

void readConfig()
{
    FILE* file = fopen("./config.txt", "r");
    if (file != NULL)
    {
    /*    char* line = malloc(sizeof(char[100]));
        do
        {
            fgets(line, 100, file);
        }
        while
        fclose(file);*/
    }
}

char* readLine(FILE* file)
{
    if (file != NULL)
    {

    }
}
