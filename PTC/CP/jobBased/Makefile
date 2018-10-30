#!/bin/make
# makefile for PROJECT shared library
# 2013-05-18

### BIN ###############################################

CC=g++
MKDIR=@mkdir -p
RM=@rm
CP=@cp

#CPLEXDIR = /home/mnattaf/Library/CPLEX_128/
CPLEXDIR=$(CPX_PATH)

RELEASE_DIR=${HOME}/local
DEBUG=0#enable to provide debug infos

### FLAGS #############################################
LDFLAGS= -L$(CPLEXDIR)cpoptimizer/lib/x86-64_linux/static_pic \
	 -L$(CPLEXDIR)cplex/lib/x86-64_linux/static_pic \
	 -L$(CPLEXDIR)concert/lib/x86-64_linux/static_pic
LDLIBS= -lcp  -lilocplex -lcplex -lconcert -lm -pthread -ldl
CPPFLAGS= -I./header/ \
	  -I../../Problem/header/ \
	  -I../../Heuristics/SCH/header/ \
	  -I../../Heuristics/QCH/header/ \
	  -I$(CPLEXDIR)cpoptimizer/include \
	  -I$(CPLEXDIR)concert/include \
	  -std=c++11 -O2 -m64 -fPIC -march=native -Wall  -Werror -DIL_STD -Wno-ignored-attributes

DEBUG_FLAGS= -W  -Werror -Wall -gstabs


### PROPERTIES ########################################
PROJECTNAME=bin/modelCP1
SRCS=../../Problem/source/Family.cpp \
     ../../Problem/source/Problem.cpp \
     ../../Problem/source/Assignment.cpp \
     ../../Problem/source/Solution.cpp \
     ../../Heuristics/SCH/source/SchedulingCentric.cpp \
     ../../Heuristics/QCH/source/QualifCentric.cpp \
     ./source/CPjobModel.cpp \
     ./source/main.cpp

### PREPROCESSING #####################################

ifeq (${DEBUG},1)
CPPFLAGS+=${DEBUG_FLAGS}
endif

OBJS=$(subst .cpp,.o,$(SRCS))

### TARGETS ###########################################

all: $(PROJECTNAME)

depend: .depend

.depend: $(SRCS)
	@rm -f .depend
	$(CC) $(CPPFLAGS) -MM $^>>./.depend

$(PROJECTNAME): pre $(OBJS)
	$(CC) $(LDFLAGS) -o $(PROJECTNAME) $(OBJS) $(LDLIBS)

pre:
	$(MKDIR) bin

install: $(PROJECTNAME)
	$(CP) $(PROJECTNAME) $(RELEASE_DIR)/lib/$(PROJECTNAME)
	$(CP) -Ru includes/* $(RELEASE_DIR)/lib/includes/

mrproper: clean
	$(RM) -rf bin/* $(PROJECTNAME)

clean:
	$(RM) -f $(OBJS) .depend

