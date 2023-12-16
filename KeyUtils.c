#include <sys/ipc.h>
#include <stdio.h>
#include "KeyUtils.h"

int getKey(int id)
{
    int key = ftok("./Master", id);
    if (key == -1)
    {
        perror("Key generation error");
    }
    return key+id;
}
