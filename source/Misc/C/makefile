# Dopus5 commands makefile

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

# make x86_64-aros release debug=no
# Compiles normal version of dopus5 components for x86_64-AROS & creates
# release archive.
###########################################################
# Read platform arguments from the "Make" command line:
# Acceptable platform arguments are os3, os4, mos, i386-aros or x86_64-aros.
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
ifeq ($(findstring i386-aros, $(MAKECMDGOALS)),i386-aros)
	PLATFORM:= aros
	arch:= i386
	NAME:= $(arch)-$(PLATFORM) $(NAME)
endif
ifeq ($(findstring x86_64-aros, $(MAKECMDGOALS)),x86_64-aros)
	PLATFORM:= aros
	arch:= x86_64
	NAME:= $(arch)-$(PLATFORM) $(NAME)
endif
NAME := $(strip $(NAME))

# Generate error if no platform entered on "Make" command line.
ifeq ($(PLATFORM), none)
$(error Error: Enter a platform (os3, os4, mos, i386-aros or x86_64-aros).)
endif

#Generate error if multiple platforms entered on "Make" command line.
ifneq ($(words $(NAME)), 1)
$(error Error: Enter only 1 platform (os3, os4, mos, i386-aros or x86_64-aros).)
endif

# Compile all if only a platform is entered on "Make" command line.
ifeq ($(strip $(filter-out $(NAME), $(MAKECMDGOALS))),)
$(NAME) : all
endif
###########################################################


.PHONY : all
all: dopusrt loaddb viewfont
	@echo " "

.PHONY : dopusrt
dopusrt:
	@echo " "
	@echo "================== dopusrt ================="
	@$(MAKE) --no-print-directory -C dopusrt -f makefile.$(PLATFORM)

.PHONY : loaddb
loaddb:
	@echo " "
	@echo "================== loaddb =================="
	@$(MAKE) --no-print-directory -C loadwb -f makefile.$(PLATFORM)

.PHONY : viewfont
viewfont:
	@echo " "
	@echo "================= viewfont ================="
	@$(MAKE) --no-print-directory -C ViewFont -f makefile.$(PLATFORM)


###########################################################

.PHONY : clean
clean: clean-dopusrt clean-loaddb clean-viewfont
	@echo " "

.PHONY : clean-dopusrt
clean-dopusrt:
	@echo " "
	@echo "--------------- clean dopusrt --------------"
	@$(MAKE) --no-print-directory -C dopusrt -f makefile.$(PLATFORM) clean
	
.PHONY : clean-loaddb
clean-loaddb:
	@echo " "
	@echo "--------------- clean loaddb ---------------"
	@$(MAKE) --no-print-directory -C loadwb -f makefile.$(PLATFORM) clean

.PHONY : clean-viewfont
clean-viewfont:
	@echo " "
	@echo "-------------- clean viewfont --------------"
	@$(MAKE) --no-print-directory -C ViewFont -f makefile.$(PLATFORM) clean


###########################################################

# Dummy targets to quietly dispose of platform
# commands when entered as a goal for "make"
# on the command line (like: make os3).

.PHONY : os3 os4 mos aros i386-aros x86_64-aros
i386-aros: mos
x86_64-aros: mos
os3: mos
os4: mos
aros: mos
mos:
	-@sh -c "echo -n"

