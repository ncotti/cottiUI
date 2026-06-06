#------------------------------------------------------------------------------
# Makefile Initialization
#------------------------------------------------------------------------------
SHELL=/bin/bash
.DELETE_ON_ERROR:
.SILENT:
.DEFAULT_GOAL := help

#------------------------------------------------------------------------------
# User modifiable variables
#------------------------------------------------------------------------------
# E.g.: arm-none-eabi-, arm-linux-gnueabihf-, (left empty), etc
TOOLCHAIN ?=

SRC_DIRS ?= src
HEADER_DIRS ?= inc

CFLAGS ?= -Wall -g -Wpedantic
ASFLAGS ?= -g
LDFLAGS ?= -g -lncurses -lpanel -lmenu -lform -lcdk -lm
# Linker script (can be empty)
LDSCRIPT ?=

# Name of the final executable (without extension)
EXE ?= exe

# Name of the gdb script (can be empty)
GDB_SCRIPT ?=

#------------------------------------------------------------------------------
# Binutils
#------------------------------------------------------------------------------
ifeq ($(origin CC), default)
CC := $(TOOLCHAIN)gcc
else
CC ?= $(TOOLCHAIN)gcc
endif
ifeq ($(origin AS), default)
AS := $(TOOLCHAIN)as
else
AS ?= $(TOOLCHAIN)as
endif
LD 			:= $(TOOLCHAIN)gcc
OBJDUMP 	:= $(TOOLCHAIN)objdump
OBJCOPY 	:= $(TOOLCHAIN)objcopy

#------------------------------------------------------------------------------
# Miscellaneous constants
#------------------------------------------------------------------------------
PRINT_CHECKMARK 	:= printf "\033[0;32m\342\234\224\n\033[0m"
PRINT_CROSS 		:= printf "\033[0;31m\342\235\214\n\033[0m"
PRINT_WARNING		:= printf "\033[0;33m\342\232\240\n\033[0m"

#------------------------------------------------------------------------------
# File location
#------------------------------------------------------------------------------
BUILD_DIR	:= build
INFO_DIR 	:= info
ELF 		:= $(BUILD_DIR)/$(EXE).elf
BIN 		:= $(BUILD_DIR)/$(EXE).bin
MAP			:= $(BUILD_DIR)/$(INFO_DIR)/memory.map

COMPILE_COMMANDS := $(BUILD_DIR)/compile_commands.json
SCAN_BUILD_DIR := $(BUILD_DIR)/scan_build

# If you use "ld" or "gcc" as linker, the memory map option is declared different
ifneq (,$(findstring -ld, $(LD)))
LDFLAGS += -Map $(MAP)
else
LDFLAGS += -Wl,-Map=$(MAP)
endif

# If you are using a custom linker script, don't use the default crt0.S files
ifneq (,$(LDSCRIPT))
LDFLAGS += -T $(LDSCRIPT)
endif

SRCS := $(foreach dir, $(SRC_DIRS), $(wildcard $(dir)/*.c) $(wildcard $(dir)/*.s))

HEADERS := $(foreach dir, $(HEADER_DIRS), $(wildcard $(dir)/*.h) $(wildcard $(dir)/*.s))
HEADER_FLAGS := $(addprefix -I , $(HEADER_DIRS))

OBJS := $(addprefix $(BUILD_DIR)/, $(SRCS))
OBJS := $(patsubst %.c, %.o, $(OBJS))
OBJS := $(patsubst %.s, %.o, $(OBJS))

OBJ_HEADERS := $(patsubst %.o, %.header, $(OBJS) $(ELF))
OBJ_HEADERS := $(patsubst %.elf, %.header, $(OBJ_HEADERS))
OBJ_HEADERS := $(patsubst $(BUILD_DIR)%, $(BUILD_DIR)/$(INFO_DIR)%, $(OBJ_HEADERS))

DIS_ASM := $(patsubst %.header, %.d, $(OBJ_HEADERS))

BUILD_SRC_DIRS := $(addprefix $(BUILD_DIR)/, $(SRC_DIRS))
INFO_SRC_DIRS := $(addprefix $(BUILD_DIR)/$(INFO_DIR)/, $(SRC_DIRS))

#------------------------------------------------------------------------------
# User targets
#------------------------------------------------------------------------------

# When you call "make compile", the Makefile will be re-called but prepending
# the "scan-build bear -- make _compile"
.PHONY: compile
compile: $(BUILD_SRC_DIRS) ## Compile all source code, generate ELF file.
# if [ ! -f $(COMPILE_COMMANDS) ]; then \
# 	scan-build -o $(SCAN_BUILD_DIR) bear --output $(COMPILE_COMMANDS) -- $(MAKE) _compile; \
# else \
# 	scan-build --use-cc=$(CC) -o $(SCAN_BUILD_DIR) -V $(MAKE) _compile; \
# fi
	if [ ! -f $(COMPILE_COMMANDS) ]; then \
		bear --output $(COMPILE_COMMANDS) -- $(MAKE) _compile; \
	else \
		$(MAKE) _compile; \
	fi
	$(MAKE) tidy


# Actual compile command
.PHONY: _compile
_compile: $(ELF)

.PHONY: tidy
tidy: $(SRCS)
	clang-tidy --verify-config
	clang-tidy $^ -p $(COMPILE_COMMANDS)

.PHONY: help
help: ## Display this message.
	grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) \
	| sort \
	| awk 'BEGIN {FS = ":.*?## "}; \
	{printf "\033[36m%-10s\033[0m %s\n", $$1, $$2}'

.PHONY: binary
binary: $(BIN) ## Generate binary file, without ELF headers.

.PHONY: headers
headers: $(OBJ_HEADERS) ## Generate symbol table and section headers for all object files.

.PHONY: dasm
dasm: $(DIS_ASM) ## Generate disassemble for all object files and elf file.

.PHONY: clean
clean: ## Erase contents of build directory.
	rm -Rf $(BUILD_DIR)
	echo -n "All files successfully erased "; $(PRINT_CHECKMARK)

.PHONY: run
run: compile ## Execute compile program
	$(ELF)

.PHONY: debug
debug: compile
	gdb $(ELF)

#------------------------------------------------------------------------------
# Compilation targets
#------------------------------------------------------------------------------
# Main executable linking
$(ELF): $(OBJS)
	echo -n "Linking everything together... "
	mkdir -p $(BUILD_DIR)/$(INFO_DIR)
	$(LD) -o $@ $^ $(LDFLAGS)
	$(PRINT_CHECKMARK)
	echo "Executable file \"$@\" successfully created."

# Compiling object files from C sources
$(BUILD_DIR)/%.o: %.c $(HEADERS) Makefile $(LDSCRIPT) $(BUILD_SRC_DIRS)
	echo -n "Compiling $< --> $@... "
	$(CC) $(CFLAGS) $(HEADER_FLAGS) -o $@ -c $<
	$(PRINT_CHECKMARK)

# Compiling object files from asm sources
$(BUILD_DIR)/%.o: %.s $(HEADERS) Makefile $(LDSCRIPT) $(BUILD_SRC_DIRS)
	echo -n "Assembling $< --> $@... "
	$(AS) $(ASFLAGS) $(HEADER_FLAGS) -o $@ -c $<
	$(PRINT_CHECKMARK)

# Print object files' headers
$(BUILD_DIR)/$(INFO_DIR)/%.header: $(BUILD_DIR)/%.o $(INFO_SRC_DIRS)
	echo -n "Printing $< -> $@... "
	$(OBJDUMP) -x $< > $@
	$(PRINT_CHECKMARK)

# Print elf file's header
$(BUILD_DIR)/$(INFO_DIR)/%.header: $(BUILD_DIR)/%.elf $(INFO_SRC_DIRS)
	echo -n "Printing $< -> $@... "
	$(OBJDUMP) -x $< > $@
	$(PRINT_CHECKMARK)

# Print object files' disassembly
$(BUILD_DIR)/$(INFO_DIR)/%.d: $(BUILD_DIR)/%.o $(INFO_SRC_DIRS)
	echo -n "Disassembling $< -> $@... "
	$(OBJDUMP) -d $< > $@
	$(PRINT_CHECKMARK)

# Print elf file disassembly
$(BUILD_DIR)/$(INFO_DIR)/%.d: $(BUILD_DIR)/%.elf $(INFO_SRC_DIRS)
	echo -n "Disassembling $< -> $@... "
	$(OBJDUMP) -d $< > $@
	$(PRINT_CHECKMARK)

# Copy ELF file into BIN file
$(BIN): $(ELF)
	echo -n "Creating binary file $@... "
	$(OBJCOPY) -O binary $(ELF) $(BIN)
	$(PRINT_CHECKMARK)

# Folders
$(BUILD_SRC_DIRS) $(INFO_SRC_DIRS) $(BUILD_DIR)/$(INFO_DIR):
	mkdir -p $@
