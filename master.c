#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "StringUtils.h"

int main() {
    printf("Hello, World 4!\n");
    printf("key = %s\nvalue = %s", stringBefore("string=25", "="), stringAfter("string=25", "="));
    return 0;
}


void readLine(char* line)
{
    line = stringClearChar(line, ' ');
    if (stringEndsWith(line, "\0"))
    {
        line = stringCut(line, 0, stringLength(line) - 2);
    }
    if (stringEndsWith(line, "\n"))
    {
        line = stringCut(line, 0, stringLength(line) - 1);
    }
    char* key = stringBefore(line, "=");
    char* value = stringAfter(line, "=");

}

void readConfig()
{
    FILE* file = fopen("./config.txt", "r");
    if (file != NULL)
    {
        boolean hasNextLine;
        char* line = emptyString(100);
        do
        {
            fgets(line, 100, file);
            readLine(line);
            hasNextLine = !stringEndsWith(line, "\0");
        }
        while (hasNextLine);
        fclose(file);
    }
}
