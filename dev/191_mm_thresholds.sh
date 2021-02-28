#!/bin/bash

# Script to take MM 191 threshold calibration runs. Questions: henri.bachacou@cern.ch
display_usage() { 
	echo "Usage: source 191_mm_thresholds.sh sector daq_sector x_rms felix"
	echo "Example: source 191_mm_thresholds.sh A14 LS 9 -> for felix06"
	echo "Example: source 191_mm_thresholds.sh A06 SS 12 flx12 -> if running on felix12"
	echo "This script will take a thresholds calibration run, produce a json file and plots, and copy and move them to eos and to the 191 webpage."
}

if [[ ( $# == "--help") ||  $# == "-h" ]] 
then 
	display_usage
	return 0
fi 

sector=$1
daqsector=$2
RMS=$3
felix=$4
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
 
 if [ "$daqsector" != "SS" ] && [ "$daqsector" != "LS" ] 
 then
	 display_usage
	 echo "Incorrect daq sector (should be SS or LS). Exiting."
	 return 1
 fi

 if [ "$RMS" -le "20" ] && [ "$RMS" -ge "0" ]  
 then
     echo "Will run on sector $side$sect_number with $daqsector configuration with threshold = baseline + rmsx${RMS}"
 else
	 display_usage
	 echo "x_rms should be an integer between 0 and 20. Exiting."
	 return 1
 fi

LAYER1="MMFE8_L1P1_IPL,MMFE8_L1P2_IPL,MMFE8_L1P3_IPL,MMFE8_L1P4_IPL,MMFE8_L1P5_IPL,MMFE8_L1P6_IPL,MMFE8_L1P7_IPL,MMFE8_L1P8_IPL,MMFE8_L1P1_HOL,MMFE8_L1P2_HOL,MMFE8_L1P3_HOL,MMFE8_L1P4_HOL,MMFE8_L1P5_HOL,MMFE8_L1P6_HOL,MMFE8_L1P7_HOL,MMFE8_L1P8_HOL,MMFE8_L1P1_IPR,MMFE8_L1P2_IPR,MMFE8_L1P3_IPR,MMFE8_L1P4_IPR,MMFE8_L1P5_IPR,MMFE8_L1P6_IPR,MMFE8_L1P7_IPR,MMFE8_L1P8_IPR,MMFE8_L1P1_HOR,MMFE8_L1P2_HOR,MMFE8_L1P3_HOR,MMFE8_L1P4_HOR,MMFE8_L1P5_HOR,MMFE8_L1P6_HOR,MMFE8_L1P7_HOR,MMFE8_L1P8_HOR"

LAYER2="MMFE8_L2P1_IPL,MMFE8_L2P2_IPL,MMFE8_L2P3_IPL,MMFE8_L2P4_IPL,MMFE8_L2P5_IPL,MMFE8_L2P6_IPL,MMFE8_L2P7_IPL,MMFE8_L2P8_IPL,MMFE8_L2P1_HOL,MMFE8_L2P2_HOL,MMFE8_L2P3_HOL,MMFE8_L2P4_HOL,MMFE8_L2P5_HOL,MMFE8_L2P6_HOL,MMFE8_L2P7_HOL,MMFE8_L2P8_HOL,MMFE8_L2P1_IPR,MMFE8_L2P2_IPR,MMFE8_L2P3_IPR,MMFE8_L2P4_IPR,MMFE8_L2P5_IPR,MMFE8_L2P6_IPR,MMFE8_L2P7_IPR,MMFE8_L2P8_IPR,MMFE8_L2P1_HOR,MMFE8_L2P2_HOR,MMFE8_L2P3_HOR,MMFE8_L2P4_HOR,MMFE8_L2P5_HOR,MMFE8_L2P6_HOR,MMFE8_L2P7_HOR,MMFE8_L2P8_HOR"

LAYER3="MMFE8_L3P1_IPL,MMFE8_L3P2_IPL,MMFE8_L3P3_IPL,MMFE8_L3P4_IPL,MMFE8_L3P5_IPL,MMFE8_L3P6_IPL,MMFE8_L3P7_IPL,MMFE8_L3P8_IPL,MMFE8_L3P1_HOL,MMFE8_L3P2_HOL,MMFE8_L3P3_HOL,MMFE8_L3P4_HOL,MMFE8_L3P5_HOL,MMFE8_L3P6_HOL,MMFE8_L3P7_HOL,MMFE8_L3P8_HOL,MMFE8_L3P1_IPR,MMFE8_L3P2_IPR,MMFE8_L3P3_IPR,MMFE8_L3P4_IPR,MMFE8_L3P5_IPR,MMFE8_L3P6_IPR,MMFE8_L3P7_IPR,MMFE8_L3P8_IPR,MMFE8_L3P1_HOR,MMFE8_L3P2_HOR,MMFE8_L3P3_HOR,MMFE8_L3P4_HOR,MMFE8_L3P5_HOR,MMFE8_L3P6_HOR,MMFE8_L3P7_HOR,MMFE8_L3P8_HOR"

LAYER4="MMFE8_L4P1_IPL,MMFE8_L4P2_IPL,MMFE8_L4P3_IPL,MMFE8_L4P4_IPL,MMFE8_L4P5_IPL,MMFE8_L4P6_IPL,MMFE8_L4P7_IPL,MMFE8_L4P8_IPL,MMFE8_L4P1_HOL,MMFE8_L4P2_HOL,MMFE8_L4P3_HOL,MMFE8_L4P4_HOL,MMFE8_L4P5_HOL,MMFE8_L4P6_HOL,MMFE8_L4P7_HOL,MMFE8_L4P8_HOL,MMFE8_L4P1_IPR,MMFE8_L4P2_IPR,MMFE8_L4P3_IPR,MMFE8_L4P4_IPR,MMFE8_L4P5_IPR,MMFE8_L4P6_IPR,MMFE8_L4P7_IPR,MMFE8_L4P8_IPR,MMFE8_L4P1_HOR,MMFE8_L4P2_HOR,MMFE8_L4P3_HOR,MMFE8_L4P4_HOR,MMFE8_L4P5_HOR,MMFE8_L4P6_HOR,MMFE8_L4P7_HOR,MMFE8_L4P8_HOR"

MMFE8S="${LAYER1},${LAYER2},${LAYER3},${LAYER4}"

# How to choose only a single board:
# MMFE8S="MMFE8_L1P4_IPR"

CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/Defaults/full_sector_MM_default_191_baselines_${daqsector}.json"
if [ "$felix" == "flx12" ]
then
      CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/Defaults/full_sector_MM_default_191_baselines_${daqsector}_flx12.json"
fi
  
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=trimmers191/$NOW
OUTLOG=trimmers191/$NOW/log
THREADS=128

WEB=/eos/atlas/atlascerngroupdisk/det-nsw/191/side${side}/Sector${sect_number}/Micromegas/trimmers/${NOW}

BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
CALIBR="${BIN}/calibrate_trimmers_multirms_191"
INJECT="./bb5_analysis/scripts/injectTrimmerCalibration.py"

mkdir -p ${OUT}
mkdir -p ${OUTLOG}

#
# ALWAYS CONFIGURE
#
#$BIN/configure_frontend -c $CONFIG -v -r -m $THREADS -n $MMFE8S
# to switch to tdaq current release:                                                                                                                                                                      
/afs/cern.ch/user/n/nswdaq/workspace/public/nswdaq/current/nswdaq/installed/x86_64-centos7-gcc8-opt/bin/configure_frontend/configure_frontend -c $CONFIG -v -r -m $THREADS -n $MMFE8S                                               


# calibrate thresholds
#time python NSWConfiguration/app/threshold_parallel_191.py -rms $RMS -c $CONFIG -s $CALIBR -f $MMFE8S -m $THREADS -o $OUTLOG
time python NSWConfiguration/app/threshold_parallel_191.py -rms $RMS -c $CONFIG -s $CALIBR -f $MMFE8S -m $THREADS -o $OUTLOG --remove_disconnected

# put thresholds into a new json
grep -h "^DATA_x${RMS}" $OUTLOG/thresholds_MMFE8_L* > $OUT/trimmers_x${RMS}.txt

TRIMMERJSON="${OUT}/trimmers_x${RMS}.json"
if [ "$felix" == "flx12" ]
then
      TRIMMERJSON="${OUT}/trimmers_x${RMS}_flx12.json"
fi

python $INJECT -i $CONFIG -c $OUT/trimmers_x${RMS}.txt -o ${TRIMMERJSON} -d 0
echo

# baselines
grep -h "^DATA" $OUTLOG/baselines_MMFE8_L*VMM* > $OUT/baselines.txt

# below: make fancy plots

# thresholds
#CONFIG="$OUT/trimmers_x${RMS}.json"
#$BIN/read_channel_monitor -c $CONFIG -s  10 -o $OUT/${RMS}/thresholds_trimmed   --threshold --threads $THREADS --dump -n $MMFE8S
#$BIN/read_channel_monitor -c $CONFIG -s  10 -o $OUT/${RMS}/thresholds_untrimmed --threshold --threads $THREADS --dump -n $MMFE8S -T 0
###LUIGI
$BIN/read_channel_monitor -c $TRIMMERJSON -s  10 -o $OUT/${RMS}/thresholds_trimmed   --threshold --threads $THREADS --dump -n $MMFE8S
$BIN/read_channel_monitor -c $TRIMMERJSON -s  10 -o $OUT/${RMS}/thresholds_untrimmed --threshold --threads $THREADS --dump -n $MMFE8S -T 0
grep -h "^DATA" $OUT/${RMS}/thresholds_trimmed/*.txt   > $OUT/${RMS}/thresholds_trimmed.txt
grep -h "^DATA" $OUT/${RMS}/thresholds_untrimmed/*.txt > $OUT/${RMS}/thresholds_untrimmed.txt

## plotting script
python NSWConfiguration/dev/mm_trimmers.plot.191.py -o $OUT/trimmers_${NOW}_x${RMS}.root \
    --bl           $OUT/baselines.txt \
    --th_untrimmed $OUT/${RMS}/thresholds_untrimmed.txt \
    --th_trimmed   $OUT/${RMS}/thresholds_trimmed.txt

## copy to the web
mkdir ${WEB}_x${RMS}
chmod -R g+wxr                       ${WEB}_x${RMS}
mv $OUT/trimmers_${NOW}_x${RMS}.root ${WEB}_x${RMS}/
mv $OUT/baselines.txt                ${WEB}_x${RMS}/
mv $OUT/trimmers_*                   ${WEB}_x${RMS}/
mv $OUT/${RMS}/thresholds*.txt       ${WEB}_x${RMS}/
rm -rf $OUT/log
rm -rf $OUT/${RMS}/thresholds_trimmed
rm -rf $OUT/${RMS}/thresholds_untrimmed
echo $OUT
echo $WEB_x${RMS}
echo "http://nsw191.web.cern.ch/"
echo "You may want to save the produced json file in the appropriate directory:"
echo "cp $OUT/trimmers_x${RMS}.json /afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/${sector}/full_sector_${sector}_191_noise_${daqsector}_x${RMS}_${NOW}.json"
echo "cp $OUT/trimmers_x${RMS}_flx12.json /afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/${sector}/full_sector_${sector}_191_noise_${daqsector}_x${RMS}_${NOW}_flx12.json"
echo "If you want the json to be used by the 191A-MM-Noise or 191A-MM-Noise-LS partitions (beware of which DAQ sector you are (SS or LS) and edit the json accordingly!):"
echo "cp $OUT/trimmers_x${RMS}.json /afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/Defaults/full_sector_MM_191_noise.json"
echo "and:"
echo "cp $OUT/trimmers_x${RMS}.json /afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/Defaults/full_sector_MM_191_noise_LS.json"
echo "please use the right trimmer json file accortingly with the felix machine"

