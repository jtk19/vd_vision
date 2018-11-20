#----------------------------------------------------------------
# Root Makefile for  common code
#----------------------------------------------------------------

.DEFAULT_GOAL := this

ifneq ($(BUILD),"RELEASE")
        export BUILD = "DEBUG"
endif


COMMON_BUILD := ../build
COMMON_SRC := .
COMMON_INC := ../inc
COMMON_CFG := ../cfg

curr_dir := $(shell pwd)

THIS_BUILD := $(COMMON_BUILD)/common_build

CFG = $(COMMON_CFG)/veed.cfg


CC = g++

INCLUDES = -I$(COMMON_INC) -I/usr/local/include 

LIBS = -L/usr/local/lib 


SUBDIRS = 

SOURCES = $(shell echo *.cc)

HEADERS = $(shell echo *.h)

OBJECTS = $(SOURCES:%.cc=$(THIS_BUILD)/%.o)

ALL_OBJECTS = $(shell echo $(THIS_BUILD)/*.o)

##TARGET = $(COMMON_BUILD)/... ## No targets, just objects


CFLAGS = -Wall -g -std=c++14

LDFLAGS = 

ifeq ($(BUILD),"DEBUG")
	CFLAGS += -DDEBUG
endif


$(OBJECTS): $(THIS_BUILD)/%.o : %.cc %.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@  
	
	
.PHONY: target_lib
target_lib: $(OBJECTS)
	@##$(CC) $(CFLAGS) -o $@ $(ALL_OBJECTS) $(UTIL_OBJECTS) $(LDFLAGS) $(LIBS)
	
## No external dependencies at the moment.	
##define extbuild
##	@for extdir in $(EXTDIRS); do \
##	    echo "Making all in $$extdir"; \
##	    $(MAKE) -C $$extdir all;\
##	done
##endef

	
define fullBuild
	@echo "Building $(curr_dir) - full build "
	mkdir -p $(THIS_BUILD)
	@for subdir in $(SUBDIRS); do \
	    echo "Making all in $(curr_dir)/$$subdir"; \
	    $(MAKE) -C $(curr_dir)/$$subdir all;\
	done
	$(MAKE) target_lib 
	# post-build steps
	mkdir -p $(COMMON_INC)
	cp -f $(COMMON_SRC)/*.h $(COMMON_INC)
	cp $(CFG) $(COMMON_BUILD)
endef


all:	
	$(call fullBuild)	
	
this: tree
tree:
	$(call fullBuild)

clean:
	rm -rf $(THIS_BUILD) $(COMMON_INC)/*
	
	
	
