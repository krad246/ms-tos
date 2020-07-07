################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
schedulers/vtrr/%.o: ../schedulers/vtrr/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: GNU Compiler'
	"C:/ti/msp430-gcc/bin/msp430-elf-gcc.exe" -c -mmcu=msp430fr5994 -mhwmult=f5series -DRB_UNSAFE=0 -I"C:/ti/ccsv8/ccs_base/msp430/include_gcc" -I"C:/Users/krad2/workspace_v8/rtosref333/arch" -I"C:/Users/krad2/workspace_v8/rtosref333/core" -I"C:/Users/krad2/workspace_v8/rtosref333/ds" -I"C:/Users/krad2/workspace_v8/rtosref333/ext" -I"C:/Users/krad2/workspace_v8/rtosref333/include" -I"C:/Users/krad2/workspace_v8/rtosref333/private" -I"C:/Users/krad2/workspace_v8/rtosref333/schedulers" -I"C:/Users/krad2/workspace_v8/rtosref333" -I"C:/ti/msp430-gcc/msp430-elf/include" -O0 -ffunction-sections -fdata-sections -g -gdwarf-3 -gstrict-dwarf -Wall -v -msmall -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)"  $(GEN_OPTS__FLAG) -o"$@" "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


