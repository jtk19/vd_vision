#-------------------------------------------------------------
# Makefile for managing mutiple processe
#-------------------------------------------------------------

.DEFAULT_GOAL := this

ifneq ($(BUILD),"RELEASE")
        export BUILD = "DEBUG"
endif


COMMON_BUILD := ../build
COMMON_SRC := .
COMMON_INC := ../inc
COMMON_CFG := ../cfg

curr_dir := $(shell pwd)

THIS_BUILD := $(COMMON_BUILD)/orient90_procs_build
TARGET = $(COMMON_BUILD)/veed_orient90
#CFG = $(COMMON_CFG)/orient90.cfg

BUILDDIRS = common orient90_procs


CC = g++

INCLUDES = -I$(COMMON_INC) -I/usr/local/include

LIBS = -L/usr/local/lib64 -L/usr/local/lib 
##LIBS += -lcvd -lGL -pthread -lopencv_highgui -lopencv_core -lopencv_imgproc -lopencv_objdetect -lopencv_videoio

SUBDIRS = 

EXTDIRS = ../common ../orient90

SOURCES = $(shell echo *.cc)

HEADERS = $(shell echo *.h)

OBJECTS = $(SOURCES:%.cc=$(THIS_BUILD)/%.o)


ALL_OBJECTS1 = $(addprefix $(COMMON_BUILD)/, $(BUILDDIRS) )
ALL_OBJECTS = $(addsuffix _build/*.o, $(ALL_OBJECTS1) )


CFLAGS = -Wall -g -std=c++14

LDFLAGS = 

ifeq ($(BUILD),"RELEASE")
	CFLAGS += -O2
	LDFLAGS += -s
else ## DEBUG
	CFLAGS += -g -DDEBUG
endif


$(OBJECTS): $(THIS_BUILD)/%.o : %.cc %.h
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@  
	
	
.PHONY: target_exec
target_exec: $(TARGET)
$(TARGET) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(ALL_OBJECTS) $(LDFLAGS) $(LIBS)
		
	
define fullBuild
	@echo "Building $(curr_dir) - full build "
	mkdir -p $(THIS_BUILD)
	@for subdir in $(SUBDIRS); do \
	    echo "Making all in $(curr_dir)/$$subdir"; \
	    $(MAKE) -C $(curr_dir)/$$subdir all;\
	done
	$(MAKE) target_exec
	@# post-build steps
	@#cp $(CFG) $(COMMON_BUILD)/
endef

define extbuild
	@echo "Building $(curr_dir) - full build "
	@for extdir in $(EXTDIRS); do \
	    echo "Making all in $$extdir"; \
	    $(MAKE) -C $$extdir all;\
	done
	mkdir -p $(THIS_BUILD)
	@for subdir in $(SUBDIRS); do \
	    echo "Making all in $(curr_dir)/$$subdir"; \
	    $(MAKE) -C $(curr_dir)/$$subdir all;\
	done
	$(MAKE) target_exec
	@# post-build steps
	@#cp $(CFG) $(COMMON_BUILD)/
endef

this: tree
tree:	
	$(call fullBuild)
	
	
all:
	$(call extbuild)

clean:
	rm -rf $(THIS_BUILD) $(COMMON_BUILD)/veed_orient90 
	
	
	
