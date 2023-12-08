cmake_minimum_required(VERSION 3.16)
project(DiSalvoFranza_ProgettoSO C)

set(CMAKE_C_STANDARD 99)


add_executable(Master master.c StringUtils.c MessageUtils.c SignalUtils.c SemaphoreUtils.c SharedMemoryUtils.c NodeUtils.c KeyUtils.c NumberUtils.c)
add_executable(Atom atom.c SemaphoreUtils.c SignalUtils.c SharedMemoryUtils.c NodeUtils.c KeyUtils.c NumberUtils.c)
add_executable(Activator activator.c StringUtils.c MessageUtils.c SemaphoreUtils.c NodeUtils.c KeyUtils.c SignalUtils.c)
add_executable(Supply supply.c StringUtils.c MessageUtils.c SemaphoreUtils.c KeyUtils.c SignalUtils.c NumberUtils.c)
add_executable(Inhibitor inhibitor.c KeyUtils.c NumberUtils.c)