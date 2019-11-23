################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Add inputs and outputs from these tool invocations to the build variables 
CMD_SRCS += \
../lnk_msp430f5529.cmd 

ASM_SRCS += \
../krnl_swtch.asm 

C_SRCS += \
../barrier.c \
../bsem_pool.c \
../main.c \
../os.c \
../semaphore.c \
../task_table.c \
../thread.c 

C_DEPS += \
./barrier.d \
./bsem_pool.d \
./main.d \
./os.d \
./semaphore.d \
./task_table.d \
./thread.d 

OBJS += \
./barrier.obj \
./bsem_pool.obj \
./krnl_swtch.obj \
./main.obj \
./os.obj \
./semaphore.obj \
./task_table.obj \
./thread.obj 

ASM_DEPS += \
./krnl_swtch.d 

OBJS__QUOTED += \
"barrier.obj" \
"bsem_pool.obj" \
"krnl_swtch.obj" \
"main.obj" \
"os.obj" \
"semaphore.obj" \
"task_table.obj" \
"thread.obj" 

C_DEPS__QUOTED += \
"barrier.d" \
"bsem_pool.d" \
"main.d" \
"os.d" \
"semaphore.d" \
"task_table.d" \
"thread.d" 

ASM_DEPS__QUOTED += \
"krnl_swtch.d" 

C_SRCS__QUOTED += \
"../barrier.c" \
"../bsem_pool.c" \
"../main.c" \
"../os.c" \
"../semaphore.c" \
"../task_table.c" \
"../thread.c" 

ASM_SRCS__QUOTED += \
"../krnl_swtch.asm" 


