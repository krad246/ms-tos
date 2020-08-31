#-------------------------------------------------------------------------------
# Build output parameters
#-------------------------------------------------------------------------------
MCU			 	:= msp430f5529
BINARY 			:= example

#-------------------------------------------------------------------------------
# Toolchain definitions
#-------------------------------------------------------------------------------
CC				:= msp430-elf-gcc
CXX				:= msp430-elf-g++	
AR				:= msp430-elf-ar
NM				:= msp430-elf-nm
OBJCOPY			:= msp430-elf-objcopy
OBJDUMP			:= msp430-elf-objdump
RANLIB			:= msp430-elf-ranlib
STRIP			:= msp430-elf-strip
SIZE			:= msp430-elf-size
READELF			:= msp430-elf-readelf

#-------------------------------------------------------------------------------
# Build structure defines
#-------------------------------------------------------------------------------
SRCDIR		  	:= src
BINDIR			:= bin
OBJDIR		  	:= obj
INCDIR 			:= include
APP  			:= $(BINDIR)/$(BINARY)

#-------------------------------------------------------------------------------
# Build file extensions
#-------------------------------------------------------------------------------
C_EXTS   		:= .c .cpp .cc
ASM_EXTS		:= .S

#-------------------------------------------------------------------------------
# Cross compiler flags
#-------------------------------------------------------------------------------
OPT_FLAGS		= -Og -g
WARN_FLAGS		= -Wall -Werror -Wunused -fverbose-asm
ERRATA_FLAGS	= cpu4,cpu8,cpu11,cpu12,cpu13,cpu19
SIZE_FLAGS		= -ffreestanding -nostartfiles -mtiny-printf
STYLE_FLAGS		= -masm-hex

CFLAGS			:= -mmcu=$(MCU) $(OPT_FLAGS) $(WARN_FLAGS) \
					-msilicon-errata=$(ERRATA_FLAGS) \
					-msilicon-errata-warn=$(ERRATA_FLAGS) \
					$(SIZE_FLAGS) $(STYLE_FLAGS) \
					-I$(INCDIR) -MMD -MP 

ASFLAGS			:= $(CFLAGS) -x assembler-with-cpp
LDFLAGS			:= -flto -Wl,-Map=$(APP).map -Wl,--gc-sections

#-------------------------------------------------------------------------------
# The magic
#-------------------------------------------------------------------------------
DIRS 			:= $(shell find $(SRCDIR) -type d -print)

C_SRC 			:= $(foreach DIR, $(DIRS), \
						$(foreach EXT, $(C_EXTS), \
							$(wildcard $(DIR)/*$(EXT))))

ASM_SRC			:= $(foreach DIR, $(DIRS), \
						$(foreach EXT, $(ASM_EXTS), \
							$(wildcard $(DIR)/*$(EXT))))	
			
C_OBJ 			:= $(addsuffix .o, $(addprefix $(OBJDIR)/, $(C_SRC)))
ASM_OBJ			:= $(addsuffix .o, $(addprefix $(OBJDIR)/, $(ASM_SRC)))
OBJS			:= $(C_OBJ) $(ASM_OBJ)

DEPS 			:= $(patsubst %.o, %.d, $(OBJS))

#-------------------------------------------------------------------------------
# Targets
#-------------------------------------------------------------------------------
.PHONY: clean all

all: $(APP).elf $(APP).hex 

strip: $(APP).elf
	@$(STRIP) -d $< -o $(APP)-strip.elf

sterilize: $(APP).elf
	@$(STRIP) -s $< -o $(APP)-sterile.elf

syms: $(APP).elf
	@$(OBJDUMP) -d $< > $(APP).asm

$(APP).elf: $(OBJS)
	@echo [LD] $@
	@$(CXX) $(CFLAGS) $(LDFLAGS) $^ -o $@
	@$(SIZE) $(APP).elf

$(APP).hex: $(APP).elf
	@$(OBJCOPY) -S -O ihex $< $@
	@$(OBJCOPY) -O ihex $< $@

#-------------------------------------------------------------------------------
# The implementation
#-------------------------------------------------------------------------------
$(OBJDIR)%/:
	@mkdir -p $@

$(OBJDIR)/%.c.o: %.c
	@echo [CC] $<
	@$(CC) $(CFLAGS) -DMODULE_NAME=\
	$(shell echo \
	$(shell basename $(dir $<)) | \
	tr '[:lower:]' '[:upper:]') \
	-c $< -o $@ 

$(OBJDIR)/%.cc.o: %.cc
	@echo [CXX] $<
	@$(CXX) $(CFLAGS) -DMODULE_NAME=\
	$(shell echo \
	$(shell basename $(dir $<)) | \
	tr '[:lower:]' '[:upper:]') \
	-c $< -o $@ 

$(OBJDIR)/%.cpp.o: %.cpp
	@echo [CXX] $<
	@$(CXX) $(CFLAGS) -DMODULE_NAME=\
	$(shell echo \
	$(shell basename $(dir $<)) | \
	tr '[:lower:]' '[:upper:]') \
	-c $< -o $@ 

$(OBJDIR)/%.S.o: %.S
	@echo [AS] $<
	@$(CC) $(ASFLAGS) -DMODULE_NAME=\
	$(shell echo \
	$(shell basename $(dir $<)) | \
	tr '[:lower:]' '[:upper:]') \
	-c $< -o $@ 

$(foreach OBJ, $(OBJS), $(eval $(OBJ): | $(dir $(OBJ))))

clean:
	@rm -f $(OBJS) $(DEPS) $(BINDIR)/*

ifneq ($(MAKECMDGOALS), clean)
-include $(DEPS)
endif