#==============================================================================#
#	Author: E.D Choparinov, Amsterdam
#	Created On: February 13 2024
#	
#	This Makefile contains commands for building and testing the libcsdsa.
#
#	USAGE:
#	make all     | Builds into a *.a binary for linking.
#	make pkg     | Builds into a zip containing the *.a binary and *.h files.
#	make exec    | Builds exec which can be used for development and testing.
#	make test    | Runs a specific test in the test directory.
#	make tests   | Runs all tests in the test directory.
#	make env     | Build a new docker image compatible with compiling.
#	make docker  | Enters a docker environment compatible with compiling.
#   make service | Make a docker container in the background for entering.
#   make sstop   | Stop the docker contaienr in the background
#	make memtsts | Run all tests in the test directory. Activated in docker.
#				   Make sure to run 'make env' first.
#	make memtst  | Run specific test in the test directory. Actiated in docker.
#				   Make sure to run 'make env' first.
#	make massif  | Prints the massif runtime of a executable and deletes temps.
#==============================================================================#

#-------------------------------------------------------------------------------#
# VARIABLES
#-------------------------------------------------------------------------------#
# 	PROCESS:
#	  - Used in the context of make massif and make test. Example:
#		make massif PROCESS=run_demo
#		make test PROCESS=./tests/bin/vector_tests
#		make memtest PROCESS=./tests/bin/map_tests
#	BUILD_FILE_NAME:
#	  - The name of the output file.
PROCESS = $(DEMO_FILE_NAME)
BUILD_FILE_NAME = libgecs
PKG_DIR = dist
LIBS = 

#------------------------------------------------------------------------------#
# DIRECTORY PATH CONFIGURATIONS                                                #
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
INC_DIR = -I./include/ -I./src/csdsa/ -I./src/core/ -I./src/utils/
SRC_DIR = src
OBJ_DIR = obj
DEM_DIR = demo
DEM_OBJ = demo/objs
DEM_INC = -I./demo/$(BUILD_FILE_NAME)
TST_INC = -I./tests/unity -I./src/
TST_DIR = tests
UNITY_DIR = tests/unity
BUILD_DIR = .
TST_BINS_DIR = tests/bin
TST_OBJ_DIR  = tests/objs

#------------------------------------------------------------------------------#
# COMPILER CONFIGURATIONS                                                      #
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
EXT_HDR = .h
EXT_ARCHIVE = .a
CXXFLAGS = -gdwarf-4 -O3 -Wall -Werror -UDEBUG
DEMO_FLAGS = -lrt -O3 -lncurses
BENCHFLAGS = -pg
TESTFLAGS = -DUNITY_OUTPUT_COLOR
BUILDFLAGS = -D_DEBUG

#==============================================================================#
# DOCKER CONFIGURATIONS                                                        #
#==============================================================================#
#	IMG_NAME:
#	  - The name of the docker image
#	CNT_NAME:
#	  - The name of the docker container
IMG_NAME = gecs-image
CNT_NAME = gecs-container

#------------------------------------------------------------------------------#
# PROJECT CONFIGURATIONS                                                       #
#------------------------------------------------------------------------------#
#	BUILD_LIB_FILE:
#	  - The full path to where the output file will live.
#------------------------------------------------------------------------------#
BUILD_LIB_FILE  = $(BUILD_DIR)/$(BUILD_FILE_NAME)$(EXT_ARCHIVE)
PACKG_ZIP_FILE  = $(BUILD_FILE_NAME).zip
DEMO_FILE_NAME  = run_demo
UNITY_FILE_NAME = unity

#------------------------------------------------------------------------------#
# PROJECT FILE COLLECTION                                                      #
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
ARC_FILES = $(shell find $(SRC_DIR) -name "*$(EXT_ARCHIVE)")
HDR_FILES = $(shell find $(SRC_DIR) -name "*$(EXT_HDR)")
INC_FILES = $(wildcard include/*.h)
TST_FILES = $(wildcard $(TST_DIR)/*.c)
UNITY_FILES = $(wildcard $(UNITY_DIR)/*.c)
TST_OBJ_FILES =  $(TST_FILES:$(TST_DIR)/%$(EXT)=$(TST_BINS_DIR)/%.o)
DEM_FILES = $(wildcard $(DEM_DIR)/*.c)
OBJ_FILES = $(SRC_FILES:$(SRC_DIR)/%$(EXT)=$(OBJ_DIR)/%.o) 
TST_BINS  = $(patsubst $(TST_DIR)/%.c, $(TST_BINS_DIR)/%, $(TST_FILES))

#------------------------------------------------------------------------------#
# MAKE ALL                                                                     #
#------------------------------------------------------------------------------#
all: notif $(BUILD_FILE_NAME)

notif:
	@echo Building to ${BUILD_LIB_FILE}...
	@mkdir -p ${BUILD_DIR}

$(BUILD_FILE_NAME): $(OBJ_FILES)
	@echo Bundling...
	@echo $(ARC_FILES)
	@for libfile in $(ARC_FILES) ; do ar x $$libfile ; done
	ar cr $(BUILD_LIB_FILE) *.o $^
	rm *.o
	@echo Done! GLHF

$(OBJ_DIR)/%.o: $(SRC_DIR)/%$(EXT)
	@mkdir -p $(@D)
	@echo + $< -\> $@
	$(CC) $(BUILDFLAGS) $(CXXFLAGS) $(INC_DIR) -o $@ -c $<

$(OBJ_DIR)/tests/libs/%.o: $(LIBS_DIR)/%$(EXT)
	@mkdir -p $(@D)
	@echo + $< -\> $@
	$(CC) $(BUILDFLAGS) $(CXXFLAGS) $(INC_DIR) -o $@ -c $<

#------------------------------------------------------------------------------#
# MAKE PKG                                                                     #
#------------------------------------------------------------------------------#
pkg: all 
	@echo "Packaging..."
	@mkdir -p $(PKG_DIR)
	@for header in $(INC_FILES); do cp $$header $(PKG_DIR); done
	@cp $(BUILD_LIB_FILE) $(PKG_DIR)
	@echo "Done!"

#------------------------------------------------------------------------------#
# MAKE TEST                                                                    #
#------------------------------------------------------------------------------#
.PHONY: tests
tests: $(BUILD_FILE_NAME) build_unity $(TST_BINS_DIR) $(TST_BINS) $(TST_OBJ_DIR)
	@echo Running Tests...
	@mkdir -p $(TST_DIR)
	@echo
	@for test in $(TST_BINS) ; do ./$$test --verbose && echo ; done

test: $(BUILD_FILE_NAME) build_unity $(TST_BINS_DIR) $(TST_BINS) $(TST_OBJ_DIR)
	@mkdir $(TST_DIR) || true
	@./$(PROCESS) --verbose && echo

build_unity:
	@$(CC) $(BUILDFLAGS) $(CXXFLAGS) $(INC_DIR) -o $(UNITY_FILE_NAME).o -c $(UNITY_FILES)

$(TST_OBJ_DIR)/%.o: $(TST_DIR)%.c
	@mkdir -p $(@D)
	@echo + $< -\> $@
	$(CC) $(BUILDFLAGS) $(CXXFLAGS) $(INC_DIR) -o $@ $< $(UNITY_FILE_NAME).o $(BUILD_LIB_FILE)

$(TST_BINS_DIR)/%: $(TST_DIR)/%.c
	@echo + $< -\> $@
	$(CC) $(BUILDFLAGS) $(CXXFLAGS) $(INC_DIR) $(TST_INC) $(TESTFLAGS) $< $(UNITY_FILE_NAME).o $(LIB_TST_FILES) $(BUILD_LIB_FILE) -o $@

$(TST_BINS_DIR):
	@mkdir $@

$(TST_OBJ_DIR):
	@mkdir $@

#------------------------------------------------------------------------------#
# MAKE EXEC                                                                    #
#------------------------------------------------------------------------------#
exec: pkg $(DEM_DIR)
	$(CC) $(BUILDFLAGS) $(CXXFLAGS) -lncurses $(DEMO_FLAGS) $(DEM_INC) -I./$(PKG_DIR)/ $(DEM_FILES) $(wildcard $(PKG_DIR)/*.a) -o $(DEMO_FILE_NAME)

$(DEM_DIR):
	@echo in DEM_DIR
	@mkdir $@
#------------------------------------------------------------------------------#
# MAKE MEMTSTS                                                                 #
#------------------------------------------------------------------------------#
memtsts:
	@docker run --rm -it                                                       \
		-v ".:/virtual"                                                        \
		--name $(CNT_NAME)                                                     \
		csdsa-image                                                            \
		/bin/bash -c 'make memtsts_containerized clean'

memtsts_containerized: $(BUILD_FILE_NAME) build_unity $(TST_BINS_DIR) $(TST_BINS) $(TST_OBJ_DIR)
	@echo Running Valgrind with Tests...
	@for test in $(TST_BINS) ; do echo "***\nTEST: ./$$test\n***"; valgrind                                     \
		 --show-leak-kinds=all                                                 \
		 --leak-check=full                                                     \
		 --track-origins=yes                                                   \
		 --tool=memcheck \
		 ./$$test --verbose                                                    \
		 ; done

#------------------------------------------------------------------------------#
# MAKE MEMTST                                                                  #
#------------------------------------------------------------------------------#
memtst:
	@docker run --rm -it                                                       \
		-v ".:/virtual"                                                        \
		--name $(CNT_NAME)                                                     \
		csdsa-image                                                            \
		/bin/bash -c 'make memtst_containerized clean PROCESS=$(PROCESS)'

memtst_containerized:  $(BUILD_FILE_NAME) build_unity $(TST_BINS_DIR) $(TST_BINS) $(TST_OBJ_DIR)
	  @echo "Running with args 'PROCESS=$(PROCESS)'"
	  @valgrind                                                                \
		 --show-leak-kinds=all                                                 \
		 --leak-check=full                                                     \
		 --track-origins=yes                                                   \
		 --tool=memcheck                                                       \
		 $(PROCESS) --verbose 

#------------------------------------------------------------------------------#
# MAKE SERVICE                                                                 #
#------------------------------------------------------------------------------#
.PHONY: service
service:
	@echo "Running container in background"
	@docker run                                                                \
		-v "$$(pwd):/virtual"                                                  \
		--name $(CNT_NAME)                                                     \
		csdsa-image                                                            \
		tail -f /dev/null
	@echo "Done!"

.PHONY: sservice
sservice: 
	@echo "Stoping background container..."
	@docker stop $(CNT_NAME) || true
	@docker rm $(CNT_NAME) || true

#------------------------------------------------------------------------------#
# MAKE DOCKER                                                                  #
#------------------------------------------------------------------------------#
.PHONY: docker
docker:
	@echo "Entering Container"
	@docker run --rm -it                                                       \
		-v "$$(pwd):/virtual"                                                  \
		--name $(CNT_NAME)                                                     \
		csdsa-image                                                            \
		/bin/bash
	@echo "Done!"

#------------------------------------------------------------------------------#
# MAKE ENV                                                                     #
#------------------------------------------------------------------------------#
.PHONY: env
env: sservice
	@echo "Calling 'make clean' to ensure source is clean before building!"
	@make clean
	@echo "Done!"
	@docker rmi $(IMG_NAME) || true
	@echo "Done!"
	@echo "Building image..."
	@docker build -t $(IMG_NAME) .
	@echo "Done!"

#------------------------------------------------------------------------------#
# MAKE MASSIF
#------------------------------------------------------------------------------#
.PHONY: massif
massif:
	@echo "Calling massif gen tool"
	valgrind --tool=massif --time-unit=B $(PROCESS)	
	ms_print massif.out.*
	rm massif.out.* gmon.out


#------------------------------------------------------------------------------#
# MAKE CLEAN                                                                   #
#------------------------------------------------------------------------------#
.PHONY: clean
clean:
	@echo "Cleaning generated files..."
	rm -rf $(BUILD_LIB_FILE) $(OBJ_DIR) $(TST_BINS_DIR) $(PACKG_ZIP_FILE) || true
	rm -rf $(DEMO_FILE_NAME) $(UNITY_FILE_NAME).o || true
	rm -rf $(DEMO_FILE_NAME).dSYM || true
	rm -rf $(DEM_DIR)/**/*.h $(DEM_DIR)/*.zip  $(DEM_DIR)/**/*.a
	find $(DEM_DIR) -type d -empty -delete
	rm -rf "__.SYMDEF SORTED"
	rm -rf massif.out.* gmon.out || true
