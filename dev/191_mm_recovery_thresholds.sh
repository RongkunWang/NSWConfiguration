#!/bin/bash

# Script to take MM 191 threshold calibration runs. Questions: henri.bachacou@cern.ch
display_usage() { 
	echo "Usage: source NSWConfiguration/dev/191_mm_recovery_thresholds.sh sector time_stamp_rms"
	echo "Example: source NSWConfiguration/dev/191_mm_recovery_thresholds.sh A14 2021_01_29_12h57m33s_x9"
	echo "This script re-analyzes treshold calibration data stored in baselines.txt, thresholds_trimmed.txt, and thresholds_untrimmed.txt files on eos or locally (looks in both places) and creates a new output with the suffix _2 on eos visible on the webpage."
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
NOW=${NOWIN}_2
OUT=/afs/cern.ch/user/n/nswdaq/workspace/public/quick_and_dirty_baselines/trimmers191/$NOW
OUTLOG=/afs/cern.ch/user/n/nswdaq/workspace/public/quick_and_dirty_baselines/trimmers191/$NOW/log

WEB=/eos/atlas/atlascerngroupdisk/det-nsw/191/side${side}/Sector${sect_number}/Micromegas/trimmers/${NOW}

# eos paths
INEOS=/eos/atlas/atlascerngroupdisk/det-nsw/191/side${side}/Sector${sect_number}/Micromegas/trimmers/${NOWIN}
# local path (depending on where the baselines.txt file is)
INLOCAL=/afs/cern.ch/user/n/nswdaq/workspace/public/quick_and_dirty_baselines/trimmers191/${NOWIN}

echo "This script re-analyzes thresholds/trimmers of run ${NOWIN} and creates a new directory ${NOW}"
echo "baselines.txt, thresholds_trimmed.txt, and thresholds_untrimmed.txt will be taken (in that order) from ${INEOS} or ${INLOCAL}"
echo "New output will be stored in ${WEB}"


if [ -f "${INEOS}/baselines.txt" ]; then 
    INbaselines=$INEOS/baselines.txt
    echo "Found baselines.txt in ${INEOS}"
elif [ -f "${INLOCAL}/baselines.txt" ]; then 
    INbaselines=$INLOCAL/baselines.txt
    echo "Found baselines.txt in ${INLOCAL}"
else
	display_usage
    echo "Could not find baselines.txt : Exiting."
    return 1
fi

if [ -f "${INEOS}/thresholds_trimmed.txt" ]; then 
    INtrimmed=$INEOS/thresholds_trimmed.txt
    echo "Found thresholds_trimmed.txt in ${INEOS}"
elif [ -f "${INLOCAL}/thresholds_trimmed.txt" ]; then 
    INtrimmed=$INLOCAL/thresholds_trimmed.txt
    echo "Found thresholds_trimmed.txt in ${INLOCAL}"
else
	display_usage
    echo "Could not find thresholds_trimmed.txt : Exiting."
    return 1
fi

if [ -f "${INEOS}/thresholds_untrimmed.txt" ]; then 
    INuntrimmed=$INEOS/thresholds_untrimmed.txt
    echo "Found thresholds_untrimmed.txt in ${INEOS}"
elif [ -f "${INLOCAL}/thresholds_untrimmed.txt" ]; then 
    INuntrimmed=$INLOCAL/thresholds_untrimmed.txt
    echo "Found thresholds_untrimmed.txt in ${INLOCAL}"
else
	display_usage
    echo "Could not find thresholds_untrimmed.txt : Exiting."
    return 1
fi


mkdir -p ${OUT}
mkdir -p ${OUTLOG}


## plotting script
python NSWConfiguration/dev/mm_trimmers.plot.191.py -o $OUT/trimmers_${NOW}.root \
    --bl           $INbaselines \
    --th_untrimmed $INuntrimmed \
    --th_trimmed   $INtrimmed

## copy to the web
mkdir ${WEB}
chmod -R g+wxr                       ${WEB}
mv $OUT/trimmers_${NOW}.root ${WEB}/
mv $OUT/baselines.txt                ${WEB}/
mv $OUT/trimmers_*                   ${WEB}/
mv $OUT/thresholds*.txt       ${WEB}/
rm -rf $OUT/log
rm -rf $OUT/thresholds_trimmed
rm -rf $OUT/thresholds_untrimmed
echo $OUT
echo $WEB
echo "http://nsw191.web.cern.ch/"
echo "You may want to save the produced json file in the appropriate directory (careful you have to fill in the xRMS:"
echo "cp $OUT/trimmers_xX.json /afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/${sector}/full_sector_${sector}_191_noise_${flx}_${NOW}.json"
echo "If you want the json to be used by the 191A-MM-Noise or 191A-MM-Noise-LS partitions (beware of which DAQ sector you are (SS or LS) and edit the json accordingly!):"
echo "cp $OUT/trimmers_xX.json /afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/Defaults/full_sector_MM_191_noise.json"
echo "and:"
echo "cp $OUT/trimmers_xX.json /afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/Defaults/full_sector_MM_191_noise_LS.json"


