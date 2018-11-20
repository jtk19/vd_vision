#----------------------------------------------------------------
# Root Makefile for Veed Machine Vision source code
#----------------------------------------------------------------

.DEFAULT_GOAL := all

ifneq ($(BUILD),"RELEASE")
        export BUILD = "DEBUG"
endif


COMMON_ROOT := $(shell pwd)
COMMON_INC := ${COMMON_ROOT}/inc
COMMON_LIB := ${COMMON_ROOT}/lib
COMMON_BUILD := ${COMMON_ROOT}/build
COMMON_DIST := ${HOME}/veed
COMMON_DIST1 := ${COMMON_ROOT}/dist

curr_dir := $(shell pwd)


SUBDIRS = common orient90 orient90_procs opticalflow

EXECS = orient90 veed_orient90 opticalflow

LIBS = 

ALL_EXECS = $(addprefix $(COMMON_BUILD)/, $(EXECS) )

LIBS1 = $(addprefix $(COMMON_BUILD)/lib, $(LIBS) )
ALL_LIBS = $(addsuffix .so, $(LIBS1) ) 
ALL_LIBS += $(addsuffix .a, $(LIBS1) )


define fullBuild
	mkdir -p ${COMMON_INC} ${COMMON_LIB} ${COMMON_BUILD}  
	@for subdir in $(SUBDIRS); do \
	    echo "Making all in $(curr_dir)/$$subdir"; \
	    $(MAKE) -C $(curr_dir)/$$subdir tree;\
	done
endef


all:  
	$(call fullBuild)
	

clean:
	rm -rf $(COMMON_INC)/*.h  ${COMMON_LIB}/* $(COMMON_BUILD)/* $(COMMON_DIST)/*
	
cleanlog:
cleanlogs:
	rm -rf $(COMMON_BUILD)/*.log
	
	
install:
	rm -rf $(COMMON_DIST) $(COMMON_DIST1)
	mkdir $(COMMON_DIST) $(COMMON_DIST1)
	cp -f $(ALL_EXECS)  $(COMMON_DIST)
	##cp -f $(ALL_LIBS)  $(COMMON_DIST)
	cp -f $(COMMON_BUILD)/*.cfg $(COMMON_DIST)
	cp -f $(ALL_EXECS)  $(COMMON_DIST1)
	cp -f $(COMMON_BUILD)/*.cfg $(COMMON_DIST1)


