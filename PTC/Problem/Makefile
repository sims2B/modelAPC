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
LDFLAGS= -L/home/mnattaf/Library/CPLEX_1271/cpoptimizer/lib/x86-64_linux/static_pic -L/home/mnattaf/Library/CPLEX_1271/cplex/lib/x86-64_linux/static_pic -L/home/mnattaf/Library/CPLEX_1271/concert/lib/x86-64_linux/static_pic
LDLIBS= -lcp -lconcert -lilocplex -lcplex  -lm -lpthread -lrt
CPPFLAGS=  -I/home/mnattaf/Library/CPLEX_1271/cplex/include -I/home/mnattaf/Library/CPLEX_1271/cpoptimizer/include -I/home/mnattaf/Library/CPLEX_1271/concert/include  -I../../util/header/ -I../CP/jobBased/header -I../MIP/IP1/header -I./header -std=c++11 -O2 -m64 -W  -Wall -Werror -fPIC -march=native -DIL_STD 
DEBUG_FLAGS= -W  -Wall -Werror -g 

### PROPERTIES ########################################
PROJECTNAME=bin/le_nom
SRCS=~/Dev/util/source/utils.cpp \
	~/Dev/PTC/Problem/source/Family.cpp \
	~/Dev/PTC/Problem/source/Problem.cpp \
	~/Dev/PTC/Problem/source/Assignment.cpp \
	~/Dev/PTC/Problem/source/Solution.cpp \
	./source/main.cpp


#	~/Dev/PTC/MIP/IP1/source/jobModel.cpp \
#	~/Dev/PTC/CP/jobBased/source/CPjobModel.cpp \
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

