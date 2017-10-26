#!/bin/make
# makefile for PROJECT shared library
# 2013-05-18

### BIN ###############################################

CC=g++
MKDIR=@mkdir -p
RM=@rm
CP=@cp

RELEASE_DIR=${HOME}/local
DEBUG=0#enable to provide debug infos

### FLAGS #############################################
LDFLAGS= 
LDLIBS=  -lm -lpthread -lrt
CPPFLAGS=  -I/home/mnattaf/Dev/PTC/Problem/header/ -I/home/mnattaf/Dev/PTC/Heuristics/LH/header -I/home/mnattaf/Dev/PTC/Heuristics/SCH/header -I/home/mnattaf/Dev/PTC/Heuristics/QCH/header -I/home/mnattaf/Dev/util/header/ -I./header -std=c++11 -O2 -m64 -W  -Wall -Werror -fPIC -march=native -DIL_STD 
DEBUG_FLAGS= -W  -Wall -Werror -g 

### PROPERTIES ########################################
PROJECTNAME=bin/RH
SRCS=~/Dev/util/source/utils.cpp \
	~/Dev/PTC/Problem/source/Family.cpp \
	~/Dev/PTC/Problem/source/Problem.cpp \
	~/Dev/PTC/Problem/source/Assignment.cpp \
	~/Dev/PTC/Problem/source/Solution.cpp \
	~/Dev/PTC/Heuristics/QCH/source/QualifCentric.cpp \
	./source/Recursive.cpp \
	./source/main.cpp
#	~/Dev/PTC/Heuristics/SCH/source/SchedulingCentric.cpp \
#	~/Dev/PTC/Heuristics/LH/source/ListHeuristic.cpp \


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

