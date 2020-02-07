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
../main.c \
../mutex.c \
../os.c \
../semaphore.c \
../thread.c 

C_DEPS += \
./barrier.d \
./main.d \
./mutex.d \
./os.d \
./semaphore.d \
./thread.d 

OBJS += \
./barrier.obj \
./krnl_swtch.obj \
./main.obj \
./mutex.obj \
./os.obj \
./semaphore.obj \
./thread.obj 

ASM_DEPS += \
./krnl_swtch.d 

OBJS__QUOTED += \
"barrier.obj" \
"krnl_swtch.obj" \
"main.obj" \
"mutex.obj" \
"os.obj" \
"semaphore.obj" \
"thread.obj" 

C_DEPS__QUOTED += \
"barrier.d" \
"main.d" \
"mutex.d" \
"os.d" \
"semaphore.d" \
"thread.d" 

ASM_DEPS__QUOTED += \
"krnl_swtch.d" 

C_SRCS__QUOTED += \
"../barrier.c" \
"../main.c" \
"../mutex.c" \
"../os.c" \
"../semaphore.c" \
"../thread.c" 

ASM_SRCS__QUOTED += \
"../krnl_swtch.asm" 


