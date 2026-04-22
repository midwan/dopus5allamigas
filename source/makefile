# Main Dopus5 makefile

###########################################################
# Enter debug=no on "Make" command line to disable debug output.
export debug
###########################################################
# Export arch to select the AROS architecture
export arch
###########################################################
# Example Commands:

# make os4
# Compiles debug version of dopus5 components for OS4.

# make MOS debug=no
# Compiles normal (non-debug) version of dopus5 components for MorphOS.

# make OS3 release
# Compiles debug version of dopus5 components for OS3 & creates
# release archive.

# make i386-aros release debug=no
# Compiles normal version of dopus5 components for i386-AROS & creates
# release archive.
###########################################################
# Read platform arguments from the "Make" command line:
# Acceptable platform arguments are os3, os4, mos, i386-aros or arm-aros.
PLATFORM:= none
NAME:=
ifeq ($(findstring os3, $(MAKECMDGOALS)),os3)
	PLATFORM:= os3
	NAME:= $(PLATFORM) $(NAME)
endif
ifeq ($(findstring os4, $(MAKECMDGOALS)),os4)
	PLATFORM:= os4
	NAME:= $(PLATFORM) $(NAME)
endif
ifeq ($(findstring mos, $(MAKECMDGOALS)),mos)
	PLATFORM:= mos
	NAME:= $(PLATFORM) $(NAME)
endif
ifeq ($(findstring arm, $(MAKECMDGOALS)),arm)
	PLATFORM:= aros
	arch:= arm
	NAME:= $(arch)-$(PLATFORM) $(NAME)
endif
ifeq ($(findstring i386-aros, $(MAKECMDGOALS)),i386-aros)
	PLATFORM:= aros
	arch:= i386
	NAME:= $(arch)-$(PLATFORM) $(NAME)
endif
NAME := $(strip $(NAME))

# Generate error if no platform entered on "Make" command line.
ifeq ($(PLATFORM), none)
$(error Error: Enter a platform (os3, os4, mos, arm-aros or i386-aros).)
endif

#Generate error if multiple platforms entered on "Make" command line.
ifneq ($(words $(NAME)), 1)
$(error Error: Enter only 1 platform (os3, os4, mos, arm-aros or i386-aros).)
endif

# Compile all if only a platform is entered on "Make" command line.
ifeq ($(strip $(filter-out $(NAME), $(MAKECMDGOALS))),)
$(NAME) : all
endif
###########################################################
# Sets archive type & version.
# REVISION is derived from PROG_REVISION in Include/dopus/version.h so the
# release archive filename (Dopus5_<REV>_<platform>.lha) always matches the
# binary's reported version. Bumping the version.h value is the only step.
REVISION := $(shell awk '/^\#define[ \t]+PROG_REVISION/ {print $$3; exit}' Include/dopus/version.h)
ifeq ($(debug), no)
TYPE :=
VERSION := _$(REVISION)_
else
TYPE := _debug
VERSION := _$(REVISION)_dev_
endif
###########################################################
# Sets compile system.
HOST := $(shell uname)
ifeq ($(HOST), AmigaOS)
	SYSTEM := amiga
endif
ifeq ($(HOST), MorphOS)
	SYSTEM := amiga
endif
ifeq ($(HOST), AROS)
	SYSTEM := amiga
endif
ifeq ($(HOST), Linux)
	SYSTEM := linux
endif
###########################################################
# Sets destination directory and lha command for releases
DEST := ../releases/
ifeq ($(SYSTEM), amiga)
ARCDEST := /releases/
LHA := lha a -a -r -q
else ifeq ($(SYSTEM), linux)
ARCDEST := $(DEST)
LHA := lha a -q
else
ARCDEST := $(DEST)
LHA := lha ao5q
endif
###########################################################
# Sets base archive to use for releases
BASEARC := basedata.lha
###########################################################
# Commands
REMOVE := rm -rf
COPY := cp -p
MAKEDIR := mkdir -p
DATE := date
###########################################################
ifeq ($(PLATFORM), os4)
SHELL := sh
endif

.PHONY : all
all : program library modules commands

.PHONY : program
program:
	@echo " "
	@echo "<<<<<<<<<<<<<<<<<< program >>>>>>>>>>>>>>>>>>"
	@$(MAKE) -C Program -f makefile.$(PLATFORM)

.PHONY : library
library:
	@echo " "
	@echo "<<<<<<<<<<<<<<<<<< library >>>>>>>>>>>>>>>>>>"
	@$(MAKE) -C Library -f makefile.$(PLATFORM)

.PHONY : modules
modules:
	@echo " "
	@echo "<<<<<<<<<<<<<<<<<< modules >>>>>>>>>>>>>>>>>>"
	@$(MAKE) -C Modules $(NAME)

.PHONY : commands
commands:
	@echo " "
	@echo "<<<<<<<<<<<<<<<<<< commands >>>>>>>>>>>>>>>>>"
	@$(MAKE) -C Misc/C $(NAME)

###########################################################

.PHONY : clean
clean : clean-program clean-library clean-modules clean-commands

.PHONY : cleanall
cleanall : clean
	@echo " "
	@echo "++++++++++++++ clean binaries ++++++++++++++"
	@$(REMOVE) bin.$(NAME)

.PHONY : clean-program
clean-program:
	@echo " "
	@echo "++++++++++++++ clean program +++++++++++++++"
	@$(MAKE) -C Program -s -f makefile.$(PLATFORM) clean

.PHONY : clean-library
clean-library:
	@echo " "
	@echo "++++++++++++++ clean library +++++++++++++++"
	@$(MAKE) -C Library -s -f makefile.$(PLATFORM) clean

.PHONY : clean-modules
clean-modules:
	@echo " "
	@echo "++++++++++++++ clean modules +++++++++++++++"
	@$(MAKE) -C Modules -s $(NAME) clean

.PHONY : clean-commands
clean-commands:
	@echo " "
	@echo "++++++++++++++ clean commands ++++++++++++++"
	@$(MAKE) -C Misc/C -s $(NAME) clean

###########################################################

.PHONY : release
release: all
	@echo " "
	@echo ">>>>>Creating release archive: Dopus5$(VERSION)$(NAME)$(TYPE).lha"
	@$(MAKEDIR) Dopus5/Libs
	@$(MAKEDIR) Dopus5/Modules
	@$(MAKEDIR) Dopus5/C
	@$(MAKEDIR) Dopus5/Help
	@$(MAKEDIR) Dopus5/Documents
	@$(MAKEDIR) $(DEST)
ifeq ($(PLATFORM), os4)
	@$(MAKEDIR) Dopus5/WBStartup
	@$(COPY) ../archive/DoWBStartup Dopus5/WBStartup
	@$(COPY) ../archive/DoWBStartup.info  Dopus5/WBStartup
endif
	@$(COPY) bin.$(NAME)/dopus5.library Dopus5/Libs
	@$(COPY) bin.$(NAME)/*.module Dopus5/Modules
	@$(COPY) bin.$(NAME)/DirectoryOpus Dopus5
	@$(COPY) bin.$(NAME)/DOpusRT5 Dopus5/C
	@$(COPY) bin.$(NAME)/LoadDB Dopus5/C
	@$(COPY) bin.$(NAME)/viewfont Dopus5/C
	@$(COPY) ../documents/*.guide Dopus5/Help
	@$(COPY) ../documents/*.pdf Dopus5/Documents
ifeq ($(log), yes)
	@$(COPY) ../ChangeLog Dopus5
endif
ifeq ($(SYSTEM), linux)
# Ensure Amiga executables carry the Unix +x bit so lha records it
# (Amiga LHA maps it back to the Amiga `e` protection flag on extract).
	@chmod +x Dopus5/DirectoryOpus Dopus5/C/DOpusRT5 Dopus5/C/LoadDB Dopus5/C/viewfont
endif
	@$(COPY) ../archive/$(BASEARC) $(DEST)Dopus5$(VERSION)$(NAME)$(TYPE).lha
	@$(LHA) $(ARCDEST)Dopus5$(VERSION)$(NAME)$(TYPE).lha Dopus5
	@$(REMOVE) Dopus5
	@echo ">>>>>Release date:" $(shell $(DATE) "+%m/%d/%Y")

###########################################################

.PHONY : clean-release
clean-release: cleanall
	@echo " "
	@echo "++++++++++++++ clean release +++++++++++++++"
	@$(REMOVE) $(DEST)
	@$(REMOVE) bin.*

###########################################################

# Dummy targets to quietly dispose of platform
# commands when entered as a goal for "make"
# on the command line (like: make os3).

.PHONY : os3 os4 mos aros i386-aros arm-aros
i386-aros: mos
arm-aros: mos
os3: mos
os4: mos
aros: mos
mos:
	-@sh -c "echo -n"

.PHONY: Include/inline/dopus5.h

Include/inline/dopus5.h: Include/fd/dopus_lib.fd Include/clib/dopus5_protos.h
	fd2sfd Include/fd/dopus_lib.fd Include/clib/dopus5_protos.h Include/fd/dopus_lib.sfd
	sfdc --output=Include/inline/dopus5.h --target=m68k-amigaos --mode=macros Include/fd/dopus_lib.sfd

.PHONY: Include/inline/configopus.h

Include/inline/configopus.h: Include/fd/configopus.fd Include/clib/configopus_protos.h
		fd2sfd Include/fd/configopus.fd Include/clib/configopus_protos.h Include/fd/configopus.sfd
		sfdc --output=Include/inline/configopus.h --target=m68k-amigaos --mode=macros Include/fd/configopus.sfd

headers: Include/inline/dopus5.h Include/inline/configopus.h

