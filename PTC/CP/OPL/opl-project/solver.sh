#!/bin/sh
#OAR -l nodes=1/core=8,walltime=96:00:00
#OAR --name schedAPC
#OAR --project schedAPC

module load Ilog
#OPL_OPTS="-deploy"
OPLOPTS=""
INST=`basename $2 -opl.dat`
echo "i "$INST
oplrun $OPLOPTS $1 $2

# config=`mktemp --tmpdir torpedo_XXXX.dat`
# echo "configPath=\"$1\";" > $config
# echo "instancePath=\"$2\";" >> $config
# oplrun torpedo-2-stages-scheduling.mod $config
# rm $config
