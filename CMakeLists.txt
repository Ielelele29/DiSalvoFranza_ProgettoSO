cmake_minimum_required(VERSION 3.16)
project(DiSalvoFranza_ProgettoSO C)

set(CMAKE_C_STANDARD 99)


add_executable(Master master.c StringUtils.c MessageUtils.c)
add_executable(Atom atom.c)
add_executable(Activator activator.c StringUtils.c MessageUtils.c)
add_executable(Supply supply.c)
add_executable(Inhibitor inhibitor.c)