#include <sys/ipc.h>
#include "KeyUtils.h"

int getKey(int id)
{
    return ftok("./Master", id);
}
