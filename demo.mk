#==============================================================================#
#	Author: E.D Choparinov, Amsterdam
#	Related Files: Makefile.h Makefile.c
#	Created On: February 13 2024
#	
#	This Makefile contains commands for building and testing the GECS framework.
#
#	USAGE:
#	make demo   | Builds the demo which can be run by the user.
#==============================================================================#
DEMO_GECS_DIR = ./demo/gecs/
DEMO_INC_DIR = -I$(DEMO_GECS_DIR)
DEMO_CXXFLAGS = -gdwarf-4 -Wall -Werror -UDEBUG $(DEMO_INC_DIR)
DEMO_SRC_DIR = demo
DEMO_OBJ_DIR = demo_obj
DEMO_BUILD_DIR = .

DEMO_BUILD_FILE_NAME = run_demo
DEMO_BUILD_LIB_FILE = $(DEMO_BUILD_DIR)/$(DEMO_BUILD_FILE_NAME)

DEMO_SRC_FILES = $(shell find $(DEMO_SRC_DIR) -name "*$(EXT)") $(shell find $(DEMO_SRC_DIR) -name "*$(EXT_ARCHIVE)")
DEMO_OBJ_FILES = $(DEMO_SRC_FILES:$(DEMO_SRC_DIR)/%$(EXT)=$(DEMO_OBJ_DIR)/%.o)

#------------------------------------------------------------------------------#
# MAKE DEMO					 												   #
#------------------------------------------------------------------------------#
demo: pkg demo_pre $(DEMO_BUILD_FILE_NAME)_BUILD_PROC


$(DEMO_BUILD_FILE_NAME)_BUILD_PROC: $(DEMO_OBJ_FILES)
	@echo Bundling...
	$(CC) $(DEMO_CXXFLAGS) -o $(DEMO_BUILD_FILE_NAME) $^
	@echo Done! GLHF

demo_pre:
	@echo Building demo into ${DEMO_BUILD_LIB_FILE}...
	@mkdir -p ${DEMO_BUILD_DIR}
	@rm -rf $(DEMO_GECS_DIR)
	@unzip $(PACKG_ZIP_FILE) -d $(DEMO_GECS_DIR)

$(DEMO_OBJ_DIR)/%.o: $(DEMO_SRC_DIR)/%$(EXT)
	@mkdir -p $(@D)
	@echo + $< -\> $@
	@$(CC) $(BUILDFLAGS) $(DEMO_CXXFLAGS) -o $@ -c $<
