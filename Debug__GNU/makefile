################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := C:/ti/msp430-gcc

GEN_OPTS__FLAG := 
GEN_CMDS__FLAG := 

ORDERED_OBJS += \
"./main.o" \
"./arch/al.o" \
"./core/irq.o" \
"./core/panic.o" \
"./core/sched.o" \
"./core/thread.o" \
"./ds/rbtree.o" \
"./ds/rbtree_lcached.o" \
"./ds/rbtree_lrcached.o" \
"./ds/rbtree_rcached.o" \
"./private/sched_impl.o" \
"./private/sleep_queue.o" \
"./private/thread_impl.o" \
"./schedulers/lottery/lottery.o" \
"./schedulers/vtrr/vtrr.o" \
-T"../msp430fr5994.ld" \
$(GEN_CMDS__FLAG) \
-Wl,--start-group \
-lgcc \
-lc \
-Wl,--end-group \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include arch/subdir_vars.mk
-include core/subdir_vars.mk
-include ds/subdir_vars.mk
-include private/subdir_vars.mk
-include schedulers/lottery/subdir_vars.mk
-include schedulers/vtrr/subdir_vars.mk
-include subdir_rules.mk
-include arch/subdir_rules.mk
-include core/subdir_rules.mk
-include ds/subdir_rules.mk
-include private/subdir_rules.mk
-include schedulers/lottery/subdir_rules.mk
-include schedulers/vtrr/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(INO_DEPS)),)
-include $(INO_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(SV7A_DEPS)),)
-include $(SV7A_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(PDE_DEPS)),)
-include $(PDE_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
rtosref333.out \

EXE_OUTPUTS__QUOTED += \
"rtosref333.out" \

BIN_OUTPUTS += \
rtosref333.hex \

BIN_OUTPUTS__QUOTED += \
"rtosref333.hex" \


# All Target
all: $(OBJS) $(CMD_SRCS) $(LD_SRCS) $(GEN_CMDS)
	@$(MAKE) --no-print-directory -Onone "rtosref333.out"

# Tool invocations
rtosref333.out: $(OBJS) $(CMD_SRCS) $(LD_SRCS) $(GEN_CMDS)
	@echo 'Building target: "$@"'
	@echo 'Invoking: GNU Linker'
	"C:/ti/msp430-gcc/bin/msp430-elf-gcc.exe" -mhwmult=f5series -DRB_UNSAFE=0 -O0 -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -v -msmall -mmcu=msp430fr5994 -Wl,-Map,"rtosref333.map" -Wl,--gc-sections -L"C:/ti/ccsv8/ccs_base/msp430/include_gcc" -o"rtosref333.out" $(ORDERED_OBJS)
	@echo 'Finished building target: "$@"'
	@echo ' '

rtosref333.hex: $(EXE_OUTPUTS)
	@echo 'Building files: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo 'Invoking: GNU Objcopy Utility'
	"C:/ti/msp430-gcc/bin/msp430-elf-objcopy.exe"  $(EXE_OUTPUTS__QUOTED) "rtosref333.hex"
	@echo 'Finished building: $(strip $(EXE_OUTPUTS__QUOTED))'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(BIN_OUTPUTS__QUOTED)$(EXE_OUTPUTS__QUOTED)
	-$(RM) "main.o" "arch\al.o" "core\irq.o" "core\panic.o" "core\sched.o" "core\thread.o" "ds\rbtree.o" "ds\rbtree_lcached.o" "ds\rbtree_lrcached.o" "ds\rbtree_rcached.o" "private\sched_impl.o" "private\sleep_queue.o" "private\thread_impl.o" "schedulers\lottery\lottery.o" "schedulers\vtrr\vtrr.o" 
	-$(RM) "main.d" "arch\al.d" "core\irq.d" "core\panic.d" "core\sched.d" "core\thread.d" "ds\rbtree.d" "ds\rbtree_lcached.d" "ds\rbtree_lrcached.d" "ds\rbtree_rcached.d" "private\sched_impl.d" "private\sleep_queue.d" "private\thread_impl.d" "schedulers\lottery\lottery.d" "schedulers\vtrr\vtrr.d" 
	-@echo 'Finished clean'
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

