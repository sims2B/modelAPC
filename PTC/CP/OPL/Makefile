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
LDFLAGS=-L$(CPLEXDIR)opl/lib/x86-64_linux/static_pic \
	-L$(CPLEXDIR)opl/bin/x86-64_linux/ 
LDLIBS=-lopl -liljs -loplnl1 -lilocplex  -lcp -lconcert -lcplex1280 -licuuc -licui18n -licuio -licudata -lpthread -ldl  
CPPFLAGS=-I./header/ \
	 -I../../../util/header/ \
	 -I../../Problem/header/ \
	 -I../../Heuristics/SCH/header/ \
	 -I../../Heuristics/QCH/header/ \
	 -I$(CPLEXDIR)opl/include \
	 -std=c++11 -m64 -fPIC -DIL_STD -O1 -DNDEBUG -march=native -Wall -Werror -DILOUSEMT -D_REENTRANT -Wno-ignored-attributes

DEBUG_FLAGS= -W  -Werror -Wall -gstabs


### PROPERTIES ########################################
PROJECTNAME=bin/schedAPC
SRCS=../../../util/source/utils.cpp \
../../Problem/source/Assignment.cpp \
../../Problem/source/Family.cpp \
../../Problem/source/Problem.cpp \
../../Problem/source/Solution.cpp \
../../Heuristics/SCH/source/SchedulingCentric.cpp \
../../Heuristics/QCH/source/QualifCentric.cpp \
./source/schedAPC.cpp \
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

