//
// Created by lelelele29 on 07/12/23.
//

#include <sys/ipc.h>
#include "KeyUtils.h"

int getKey(int id)
{
    return ftok("./Master", id);
}
