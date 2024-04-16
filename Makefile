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
BUILD_DIR = .

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
CC = gcc
EXT = .c
CXXFLAGS = -g -Wall -Werror -UDEBUG $(INC_DIR)
BUILDFLAGS =
LDFLAGS =

#-------------------------------------------------------------------------------
# Project Configurations
# BUILD_FILE:
# 	- The name of the executable file
#-------------------------------------------------------------------------------
BUILD_FILE = ecs
BUILD_TEST = ecs_test
OUTPUTS = $(BUILD_DIR)/$(BUILD_FILE)

#-------------------------------------------------------------------------------
# Project Sources
# SRC_FILES is a recursive find on the paths of all c files.
# OBJ_FILES is a name map where [src].c -> [src].o
#-------------------------------------------------------------------------------
SRC_FILES = $(shell find $(SRC_DIR) -name "*$(EXT)")
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%$(EXT)=$(OBJ_DIR)/%.o)

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
	@echo "Cleaning /obj/*, $(OUTPUTS) .."
	@rm -r -f $(OUTPUTS)
	@rm -r -f obj

$(BUILD_FILE): $(OBJ_FILES)
	@echo Linking...
	@$(CC) $(CXXFLAGS) -o ./${BUILD_DIR}/$@ $^ $(LDFLAGS)
	@echo Done! GLHF

notif:
	@echo Building ECS to ${BUILD_DIR}/${BUILD_FILE}...
	@mkdir -p ${BUILD_DIR}

$(OBJ_DIR)/%.o: $(SRC_DIR)/%$(EXT)
	@mkdir -p $(@D)
	@echo + $< -\> $@
	@$(CC) $(BUILDFLAGS) $(CXXFLAGS) -o $@ -c $<