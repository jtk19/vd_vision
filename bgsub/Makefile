#-------------------------------------------------------------
# Makefile for 90-degree orientation program
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

THIS_BUILD := $(COMMON_BUILD)/bgsub_build
TARGET = $(COMMON_BUILD)/bgsub
#CFG = $(COMMON_CFG)/bgsub.cfg

BUILDDIRS = common bgsub


CC = g++

INCLUDES = -I$(COMMON_INC) -I/usr/local/include

LIBS = -L/usr/local/lib -pthread -lopencv_highgui -lopencv_core -lopencv_videoio -lopencv_video -lopencv_imgproc -lopencv_imgcodecs

SUBDIRS = 

EXTDIRS = ../common 

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
	@# build
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
	@# build
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
	rm -rf $(THIS_BUILD) $(COMMON_BUILD)/bgsub 
	@#rm -f $(COMMON_BUILD)/bgsub.cfg
	
	
	
