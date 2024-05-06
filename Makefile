#==============================================================================#
#	Author: E.D Choparinov, Amsterdam
#	Related Files: Makefile.h Makefile.c
#	Created On: February 13 2024
#	
#	This Makefile contains commands for building and testing the GECS framework.
#
#	USAGE:
#	make all    | Builds into a *.a binary for linking.
#	make test   | Runs all tests in the test directory.
#	make env	| Build a new docker image compatible with compiling.
#	make docker | Enters a docker environment compatible with compiling.
#	make memtst | Run all tests in the test directory. Activated in docker. Make
#				  sure to run 'make env' first.
#==============================================================================#

#------------------------------------------------------------------------------#
# DIRECTORY PATH CONFIGURATIONS												   #
#------------------------------------------------------------------------------#
#	Directory Configurations
#	INC_DIRS:
#     - Location where *.h files appear and must be included.
#	  - Note: May contain many.
#	SRC_DIR:
#	  - The source path containing the framework.
#	OBJ_DIR:
#	  - The directory to output intermediary *.o.
#	LIBS_DIR:
#	  - The path to all libraries necessary for compiling.
#	TST_DIR:
#	  - The test path containing individual files containing various 
#		unit testing components for the framework and structures.
#	BUILD_DIR:
#	  - The build path of the final executable.
#	TST_BINS_DIR:
#	  - The directory to output each test binary
#------------------------------------------------------------------------------#
INC_DIR = -I./libs/ -I./include/ -I./src/structures/ -I./src/core/ -I./src/utils
SRC_DIR = src
OBJ_DIR = obj
LIBS_DIR = libs
TST_DIR = tests
BUILD_DIR = .
TST_BINS_DIR = tests/bin

#------------------------------------------------------------------------------#
# COMPILER CONFIGURATIONS													   #
#------------------------------------------------------------------------------#
#	CC:
#	  - Compiler to use
#	EXT:
#	  - File extension used in source.
#	CXXFLAGS:
#	  - Flags used for compilation. This automatically includes -I directories 
# 		listed in the INC_DIR variable.
#	TESTFLAGS:
#	  - Flags only used specifically for compiling test files
#	BUILDFLAGS:
#	  - Extra flags used for interacting with the build.
#	LDFLAGS:
#	  - 
#------------------------------------------------------------------------------#
CC = clang
EXT = .c
CXXFLAGS = -gdwarf-4 -Wall -Werror -UDEBUG $(INC_DIR)
TESTFLAGS = -DUNITY_OUTPUT_COLOR
BUILDFLAGS =


#------------------------------------------------------------------------------#
# PROJECT CONFIGURATIONS													   #
#------------------------------------------------------------------------------#
#	BUILD_FILE_NAME:
#	  - The name of the output file.
#	BUILD_LIB_FILE:
#	  - The full path to where the output file will live.
#------------------------------------------------------------------------------#
BUILD_FILE_NAME = libgecs
BUILD_LIB_FILE   = $(BUILD_DIR)/$(BUILD_FILE_NAME).a

#------------------------------------------------------------------------------#
# PROJECT FILE COLLETION													   #
#------------------------------------------------------------------------------#
#	SRC_FILES:
#	  - Contains a list of all files in the SRC_DIR
#	LIB_FILES:
#	  - Contains a list of all files in the LIB_DIR
#	TST_FILES:
#	  - Contains a list of all files in the TST_DIR
#	OBJ_FILES:
#	  - Contains a list of all output locations for *.c files in the project.
#	  - OBJ_FILES is essentially a name map where 
#		[path_a]/[src].c -> [path_b]/[src].o
#	  - Note: Since libs must be included, the LIB_DIR files will be placed 
#		always in obj/lib/. Therefore the lib keyword is reserved.
#	TST_BINS:
#	  - Contains a list of all output locations for the testing binaries.
#------------------------------------------------------------------------------#
SRC_FILES = $(shell find $(SRC_DIR) -name "*$(EXT)")
LIB_FILES = $(wildcard $(LIBS_DIR)/*.c)
TST_FILES = $(wildcard $(TST_DIR)/*.c)
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%$(EXT)=$(OBJ_DIR)/%.o) $(LIB_FILES:$(LIBS_DIR)/%$(EXT)=$(OBJ_DIR)/libs/%.o)
TST_BINS=$(patsubst $(TST_DIR)/%.c, $(TST_BINS_DIR)/%, $(TST_FILES))

#------------------------------------------------------------------------------#
# MAKE ALL					 												   #
#------------------------------------------------------------------------------#
all: notif $(BUILD_FILE_NAME)

notif:
	@echo Building ECS to ${BUILD_LIB_FILE}...
	@mkdir -p ${BUILD_DIR}

$(BUILD_FILE_NAME): $(OBJ_FILES)
	@echo Bundling...
	ar cr $(BUILD_LIB_FILE) $^
	@echo Done! GLHF

$(OBJ_DIR)/%.o: $(SRC_DIR)/%$(EXT)
	@mkdir -p $(@D)
	@echo + $< -\> $@
	@$(CC) $(BUILDFLAGS) $(CXXFLAGS) -o $@ -c $<

$(OBJ_DIR)/libs/%.o: $(LIBS_DIR)/%$(EXT)
	@mkdir -p $(@D)
	@echo + $< -\> $@
	@$(CC) $(BUILDFLAGS) $(CXXFLAGS) -o $@ -c $<

#------------------------------------------------------------------------------#
# MAKE TEST					 												   #
#------------------------------------------------------------------------------#
test: $(BUILD_FILE_NAME) $(TST_DIR) $(TST_BINS_DIR) $(TST_BINS)
	@echo Running Tests...
	@echo
	@for test in $(TST_BINS) ; do ./$$test --verbose && echo ; done

$(TST_BINS_DIR)/%: $(TST_DIR)/%.c
	@echo + $< -\> $@
	$(CC) $(BUILDFLAGS) $(CXXFLAGS) $(TESTFLAGS) $< $(LIB_FILES) $(BUILD_LIB_FILE) -o $@

$(TST_DIR):
	@echo in TST_DIR
	@mkdir $@

$(TST_BINS_DIR):
	@mkdir $@

#------------------------------------------------------------------------------#
# MAKE MEMTST					 										   	   #
#------------------------------------------------------------------------------#
memtst:
	docker start gecs-container >/dev/null
	docker exec -it gecs-container sh -c 'make memtst_containerized'

memtst_containerized: $(BUILD_FILE_NAME) $(TST_DIR) $(TST_BINS_DIR) $(TST_BINS)
	@echo Running Valgrind with Tests...
	@for test in $(TST_BINS) ; do valgrind 									   \
		 --leak-check=full 													   \
		 --track-origins=yes 												   \
		 ./$$test --verbose 											       \
		 ; done

#------------------------------------------------------------------------------#
# MAKE DOCKER					 										   	   #
#------------------------------------------------------------------------------#
.PHONY: docker
docker:
	@echo "Entering Container"
	docker start gecs-container >/dev/null
	docker exec -it gecs-container /bin/bash
	@echo "Stopping container...please wait..."
	@docker stop gecs-container
	@echo "Done!"

#------------------------------------------------------------------------------#
# MAKE ENV						 										   	   #
#------------------------------------------------------------------------------#
.PHONY: env
env:
	@echo "Ensuring source clean before building image... doing make clean..."
	@make clean
	@echo "Done!"
	@echo "Removing container and image if they already exist..."
	@docker stop gecs-container || true
	@docker rm gecs-container || true
	@docker rmi gecs-image || true
	@echo "Done!"
	@echo "Building image..."
	@docker build -t gecs-image .
	@echo "Done!"
	@echo "Running container for the first time..."
	@docker run -it -v "$(pwd):/virtual" --name gecs-container gecs-image

#------------------------------------------------------------------------------#
# MAKE CLEAN																   #
#------------------------------------------------------------------------------#
.PHONY: clean
clean:
	@echo "Cleaning /$(OBJ_DIR)/* /$(TST_BINS_DIR)/* $(BUILD_LIB_FILE)"
	@rm -r -f $(BUILD_LIB_FILE)
	@rm -r -f $(OBJ_DIR)
	@rm -r -f $(TST_BINS_DIR)
