################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

# Each subdirectory must supply rules for building sources it contributes
ds/%.obj: ../ds/%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: MSP430 Compiler'
	"C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_19.6.0.STS/bin/cl430" -vmspx --data_model=restricted --use_hw_mpy=F5 --include_path="C:/Users/krad2/workspace_v8/rtosref333/private" --include_path="C:/ti/ccsv8/ccs_base/msp430/include" --include_path="C:/Users/krad2/workspace_v8/rtosref333/arch" --include_path="C:/Users/krad2/workspace_v8/rtosref333/core" --include_path="C:/Users/krad2/workspace_v8/rtosref333/ds" --include_path="C:/Users/krad2/workspace_v8/rtosref333/ext" --include_path="C:/Users/krad2/workspace_v8/rtosref333/include" --include_path="C:/Users/krad2/workspace_v8/rtosref333/schedulers" --include_path="C:/Users/krad2/workspace_v8/rtosref333/arch" --include_path="C:/Users/krad2/workspace_v8/rtosref333" --include_path="C:/ti/ccsv8/tools/compiler/ti-cgt-msp430_19.6.0.STS/include" --advice:power=all --define=__MSP430F5529__ -g --printf_support=minimal --diag_warning=225 --diag_wrap=off --display_error_number --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --preproc_with_compile --preproc_dependency="ds/$(basename $(<F)).d_raw" --obj_directory="ds" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


