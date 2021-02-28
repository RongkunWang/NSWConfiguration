#!/bin/bash

# Script to take MM 191 baseline runs. Questions: henri.bachacou@cern.ch
display_usage() { 
	echo "Usage: source 191_mm_recovery_baselines.sh sector time_stamp"
	echo "Example: source 191_mm_recovery_baselines.sh A06 2020_12_17_16h50m57s"
	echo "This script re-analyzes baselines data stored in a baselines.txt file on eos or locally (looks in both places) and creates a new output with the suffix _2 on eos visible on the webpage."
}

if [[ ( $# == "--help") ||  $# == "-h" ]] 
then 
	display_usage
	return 0
fi 

sector=$1
timestamp=$2
side=${sector:0:1}
sect_number=${sector:1:2}

if [ $timestamp == "" ] 
 then
	display_usage
	echo "No time-stamp was provided. Exiting."
	return 1
 fi

if [ "$side" != "A" ] && [ "$side" != "C" ] 
 then
	display_usage
	echo "Incorrect side (should be A or C). Exiting."
	return 1
 fi
 
 if [ "$sect_number" -ne "15" ] && [ "$sect_number" -ne "14" ] && [ "$sect_number" -ne "13" ]  && [ "$sect_number" -ne "12" ]  && [ "$sect_number" -ne "11" ]  && [ "$sect_number" -ne "10" ]  && [ "$sect_number" -ne "9" ]  && [ "$sect_number" -ne "08" ]  && [ "$sect_number" -ne "07" ]  && [ "$sect_number" -ne "06" ]  && [ "$sect_number" -ne "05" ]  && [ "$sect_number" -ne "04" ]  && [ "$sect_number" -ne "03" ]  && [ "$sect_number" -ne "02" ]  && [ "$sect_number" -ne "01" ]  && [ "$sect_number" -ne "16" ]  
 then
	display_usage
	echo "Incorrect sector (should be between 01 and 16). Exiting."
	return 1
 fi

NOWIN=$timestamp
NOW="${NOWIN}_2"
OUT=/afs/cern.ch/user/n/nswdaq/workspace/public/quick_and_dirty_baselines/baselines191/$NOW
# eos paths
INEOS=/eos/atlas/atlascerngroupdisk/det-nsw/191/side${side}/Sector${sect_number}/Micromegas/baselines/${NOWIN}
# local path (depending on where the baselines.txt file is)
INLOCAL=/afs/cern.ch/user/n/nswdaq/workspace/public/quick_and_dirty_baselines/baselines191/${NOWIN}
WEB=/eos/atlas/atlascerngroupdisk/det-nsw/191/side${side}/Sector${sect_number}/Micromegas/baselines/${NOW}


echo "This script re-analyzes baselines of run ${NOWIN} and creates a new directory ${NOW}"
echo "baselines.txt will be taken  (in that order) from ${INEOS} or ${INLOCAL}"
echo "New output will be stored in ${WEB}"


if [ -f "${INEOS}/baselines.txt" ]; then 
    IN=$INEOS
    echo "Found baselines.txt in ${INEOS}"
elif [ -f "${INLOCAL}/baselines.txt" ]; then 
    IN=$INLOCAL
    echo "Found baselines.txt in ${INLOCAL}"
else
	display_usage
    echo "Could not find baselines.txt : Exiting."
    return 1
fi

#rootme is needed on swrod02:
#rootme
#just need to setup root
source $ROOTSYS/bin/thisroot.sh
#export not needed anymore
#export NOW
rm -rf $OUT
mkdir $OUT
mkdir $OUT/ntuple

# now -o specifies only the output directory (instead of the output file)
# now by default no MMFE8 plots. Add "-g layer" option to have MMFE8 plots grouped by layer.
# now timestamp passed as argument with -t
python /afs/cern.ch/user/n/nswdaq/workspace/public/quick_and_dirty_baselines/NSWConfiguration/dev/mm_baselines_FAST191.plot.py -t $NOW -i $IN/baselines.txt -o $OUT -y 5 | tee $OUT/rms.txt

## copy to the web
echo "Moving to the webspace..."
rm -rf $WEB
mkdir $WEB
mv $OUT/baselines_plots.root $WEB/baselines_$NOW.root
mv $OUT/rms.txt $WEB
chmod -R g+wxr ${WEB}
echo "Moving plots to the webspace: done"
mv $OUT/ntuple $WEB
cp $IN/baselines.txt $WEB
echo "Moving data to the webspace: done"
chmod -R g+wxr ${WEB}
echo $OUT
echo "https://nsw191.web.cern.ch/nsw191/"

