#!/bin/sh
#OAR -l core=8,walltime=96:00:00
#OAR --name ptc
#OAR --project ptc

## module add java/1.8.0_u11 
export LD_LIBRARY_PATH=/opt/ibm/ILOG/CPLEX_Studio128/opl/bin/x86-64_linux/:$LIBRARY_PATH_LD

$1 $2 
exit $? 
