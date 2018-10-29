#!/bin/sh
#OAR -l nodes=1/core=8,walltime=96:00:00
#OAR --name schedAPC
#OAR --project schedAPC

module load Ilog
#OPL_OPTS="-deploy"
OPLOPTS=""
INST=`basename $2 -opl.dat`
echo "i "$INST
oplrun $OPLOPTS schedAPC.mod $1 $2

