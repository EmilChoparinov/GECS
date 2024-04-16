#-------------------------------------------------------------------------------
# Directory Configurations
# INC_DIRS: 
# 	- Location where *.h files appear
# 	- Note: May contain many.
# 
# OBJ_DIR:
# 	- Object output director.
# 
# BUILD_DIR:
#	- The build path of the final executable.
#
# SRC_DIR:
#	- Source directory.
#-------------------------------------------------------------------------------
INC_DIR = -I./include/ -I./src/structures/ -I./src/core/
SRC_DIR = src
OBJ_DIR = obj
TST_DIR = tests
BUILD_DIR =

TST_BINS_DIR = tests/bin

#-------------------------------------------------------------------------------
# Compiler Configuations
# CC:
# 	- Compiler used.
# EXT:
# 	- File extension used in source.
# CXXFLAGS:
# 	- Flags for compliation. This includes that -I include directories.
# BUILDFLAGS:
#	- Flags used for interacting with the build.
# LDFLAGS:
# 	- Additions.
#-------------------------------------------------------------------------------
CC = clang
EXT = .c
CXXFLAGS = -gdwarf-4 -Wall -Werror -UDEBUG $(INC_DIR)
TESTFLAGS = -lcriterion
BUILDFLAGS =
LDFLAGS =

#-------------------------------------------------------------------------------
# Project Configurations
# BUILD_FILE:
# 	- The name of the executable file
#-------------------------------------------------------------------------------
BUILD_FILE = libgecs
BUILD_LIB_FILE   = $(BUILD_DIR)$(BUILD_FILE).a
OUTPUTS    = $(BUILD_DIR)/$(BUILD_LIB_FILE)

#-------------------------------------------------------------------------------
# Project Sources
# SRC_FILES is a recursive find on the paths of all c files.
# OBJ_FILES is a name map where [src].c -> [src].o
#-------------------------------------------------------------------------------
SRC_FILES = $(shell find $(SRC_DIR) -name "*$(EXT)")
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%$(EXT)=$(OBJ_DIR)/%.o)
TST_FILES = $(wildcard $(TST_DIR)/*.c)

TST_BINS=$(patsubst $(TST_DIR)/%.c, $(TST_BINS_DIR)/%, $(TST_FILES))

#-------------------------------------------------------------------------------
# Makefile Rules
# all:
# 	- Build everything.
# clean:
# 	- Remove all traces of build
#-------------------------------------------------------------------------------
all: notif $(BUILD_FILE)

.PHONY: clean

clean:
	@echo "Cleaning /obj/*, /tests/bin/* $(OUTPUTS), .."
	@rm -r -f $(OUTPUTS)
	@rm -r -f $(OBJ_DIR)
	@rm -r -f $(TST_BINS_DIR)

$(BUILD_FILE): $(OBJ_FILES)
	@echo Bundling...
	ar cr $(BUILD_LIB_FILE) $^
	@echo Done! GLHF

notif:
	@echo Building ECS to ${BUILD_DIR}/${BUILD_FILE}...
	@mkdir -p ${BUILD_DIR}

$(OBJ_DIR)/%.o: $(SRC_DIR)/%$(EXT)
	@mkdir -p $(@D)
	@echo + $< -\> $@
	@$(CC) $(BUILDFLAGS) $(CXXFLAGS) -o $@ -c $<

test: $(BUILD_FILE) $(TST_DIR) $(TST_BINS_DIR) $(TST_BINS)
	@echo Running Tests...
	@for test in $(TST_BINS) ; do ./$$test --verbose ; done

$(TST_BINS_DIR)/%: $(TST_DIR)/%.c
	@echo + $< -\> $@
	$(CC) $(BUILDFLAGS) $(CXXFLAGS) $(TESTFLAGS) $< $(BUILD_LIB_FILE) -o $@

$(TST_DIR):
	@echo in TST_DIR
	@mkdir $@

$(TST_BINS_DIR):
	@mkdir $@