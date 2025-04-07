#cmake_minimum_required(VERSION 3.29)
#project(csce_2303_s25_project_1_group_9 C)
#
#set(CMAKE_C_STANDARD 11)
#
#add_executable(csce_2303_s25_project_1_group_9
#    z16sim.c
#        z16asm.c)

cmake_minimum_required(VERSION 3.29)
project(z16_simulator C)

set(CMAKE_C_STANDARD 11)

# Simulator executable
add_executable(z16_sim z16sim.c)

# Assembler executable
add_executable(z16_asm z16asm.c)