#!/bin/bash

# Script to take MM 191 baseline runs. Questions: henri.bachacou@cern.ch
display_usage() { 
	echo "Usage: source 191_mm_baselines.sh sector daq_sector felix"
	echo "Example: source 191_mm_baselines.sh A14 LS -> if on felix06"
	echo "Example: source 191_mm_baselines.sh A06 SS flx12 -> if on felix12"
	echo "This script will take a baseline run, produce plots, and move them to eos and to the 191 webpage."
}

if [[ ( $# == "--help") ||  $# == "-h" ]] 
then 
	display_usage
	return 0
fi 

sector=$1
daqsector=$2
felix=$3
side=${sector:0:1}
sect_number=${sector:1:2}

if [ "$side" != "A" ] && [ "$side" != "C" ] 
 then
	echo "Incorrect side (should be A or C)"
	display_usage
	return 1
 fi
 
 if [ "$sect_number" -ne "15" ] && [ "$sect_number" -ne "14" ] && [ "$sect_number" -ne "13" ]  && [ "$sect_number" -ne "12" ]  && [ "$sect_number" -ne "11" ]  && [ "$sect_number" -ne "10" ]  && [ "$sect_number" -ne "9" ]  && [ "$sect_number" -ne "08" ]  && [ "$sect_number" -ne "07" ]  && [ "$sect_number" -ne "06" ]  && [ "$sect_number" -ne "05" ]  && [ "$sect_number" -ne "04" ]  && [ "$sect_number" -ne "03" ]  && [ "$sect_number" -ne "02" ]  && [ "$sect_number" -ne "01" ]  && [ "$sect_number" -ne "16" ]  
 then
	echo "incorrect sector (should be between 01 and 16)"
	display_usage
	return 1
 fi
 
 if [ "$daqsector" != "SS" ] && [ "$daqsector" != "LS" ] 
 then
	echo "Incorrect daq sector (should be SS or LS)"
	display_usage
	return 1
 fi

if ["${felix}" -ne ""]
  then  
    echo Will run baselines on sector $side$sect_number with $daqsector configuration on $felix
else
    echo Will run baselines on sector $side$sect_number with $daqsector configuration on felix6
fi
#echo "RUNNING WITHOUT PCB 5-8 (ip&ho)"
#LAYER1="MMFE8_L1P1_IPL,MMFE8_L1P2_IPL,MMFE8_L1P3_IPL,MMFE8_L1P4_IPL,MMFE8_L1P1_HOL,MMFE8_L1P2_HOL,MMFE8_L1P3_HOL,MMFE8_L1P4_HOL,MMFE8_L1P1_IPR,MMFE8_L1P2_IPR,MMFE8_L1P3_IPR,MMFE8_L1P4_IPR,MMFE8_L1P1_HOR,MMFE8_L1P2_HOR,MMFE8_L1P3_HOR,MMFE8_L1P4_HOR"
#
#LAYER2="MMFE8_L2P1_IPL,MMFE8_L2P2_IPL,MMFE8_L2P3_IPL,MMFE8_L2P4_IPL,MMFE8_L2P1_HOL,MMFE8_L2P2_HOL,MMFE8_L2P3_HOL,MMFE8_L2P4_HOL,MMFE8_L2P1_IPR,MMFE8_L2P2_IPR,MMFE8_L2P3_IPR,MMFE8_L2P4_IPR,MMFE8_L2P1_HOR,MMFE8_L2P2_HOR,MMFE8_L2P3_HOR,MMFE8_L2P4_HOR"
#
#LAYER3="MMFE8_L3P1_IPL,MMFE8_L3P2_IPL,MMFE8_L3P3_IPL,MMFE8_L3P4_IPL,MMFE8_L3P1_HOL,MMFE8_L3P2_HOL,MMFE8_L3P3_HOL,MMFE8_L3P4_HOL,MMFE8_L3P1_IPR,MMFE8_L3P2_IPR,MMFE8_L3P3_IPR,MMFE8_L3P4_IPR,MMFE8_L3P1_HOR,MMFE8_L3P2_HOR,MMFE8_L3P3_HOR,MMFE8_L3P4_HOR"
#
#LAYER4="MMFE8_L4P1_IPL,MMFE8_L4P2_IPL,MMFE8_L4P3_IPL,MMFE8_L4P4_IPL,MMFE8_L4P1_HOL,MMFE8_L4P2_HOL,MMFE8_L4P3_HOL,MMFE8_L4P4_HOL,MMFE8_L4P1_IPR,MMFE8_L4P2_IPR,MMFE8_L4P3_IPR,MMFE8_L4P4_IPR,MMFE8_L4P1_HOR,MMFE8_L4P2_HOR,MMFE8_L4P3_HOR,MMFE8_L4P4_HOR"
#
#echo "FULL MMFE8 LIST IS COMMENTED OUT! -------------------------------------"

#Here removing only P5 R side
#LAYER1="MMFE8_L1P1_IPL,MMFE8_L1P2_IPL,MMFE8_L1P3_IPL,MMFE8_L1P4_IPL,MMFE8_L1P5_IPL,MMFE8_L1P6_IPL,MMFE8_L1P7_IPL,MMFE8_L1P8_IPL,MMFE8_L1P1_HOL,MMFE8_L1P2_HOL,MMFE8_L1P3_HOL,MMFE8_L1P4_HOL,MMFE8_L1P5_HOL,MMFE8_L1P6_HOL,MMFE8_L1P7_HOL,MMFE8_L1P8_HOL,MMFE8_L1P1_IPR,MMFE8_L1P2_IPR,MMFE8_L1P3_IPR,MMFE8_L1P4_IPR,MMFE8_L1P5_IPR,MMFE8_L1P6_IPR,MMFE8_L1P7_IPR,MMFE8_L1P8_IPR,MMFE8_L1P1_HOR,MMFE8_L1P2_HOR,MMFE8_L1P3_HOR,MMFE8_L1P4_HOR,MMFE8_L1P5_HOR,MMFE8_L1P6_HOR,MMFE8_L1P7_HOR,MMFE8_L1P8_HOR"

#LAYER2="MMFE8_L2P1_IPL,MMFE8_L2P2_IPL,MMFE8_L2P3_IPL,MMFE8_L2P4_IPL,MMFE8_L2P5_IPL,MMFE8_L2P6_IPL,MMFE8_L2P7_IPL,MMFE8_L2P8_IPL,MMFE8_L2P1_HOL,MMFE8_L2P2_HOL,MMFE8_L2P3_HOL,MMFE8_L2P4_HOL,MMFE8_L2P5_HOL,MMFE8_L2P6_HOL,MMFE8_L2P7_HOL,MMFE8_L2P8_HOL,MMFE8_L2P1_IPR,MMFE8_L2P2_IPR,MMFE8_L2P3_IPR,MMFE8_L2P4_IPR,MMFE8_L2P5_IPR,MMFE8_L2P6_IPR,MMFE8_L2P7_IPR,MMFE8_L2P8_IPR,MMFE8_L2P1_HOR,MMFE8_L2P2_HOR,MMFE8_L2P3_HOR,MMFE8_L2P4_HOR,MMFE8_L2P5_HOR,MMFE8_L2P6_HOR,MMFE8_L2P7_HOR,MMFE8_L2P8_HOR"

#LAYER3="MMFE8_L3P1_IPL,MMFE8_L3P2_IPL,MMFE8_L3P3_IPL,MMFE8_L3P4_IPL,MMFE8_L3P6_IPL,MMFE8_L3P7_IPL,MMFE8_L3P8_IPL,MMFE8_L3P1_HOL,MMFE8_L3P2_HOL,MMFE8_L3P3_HOL,MMFE8_L3P4_HOL,MMFE8_L3P5_HOL,MMFE8_L3P6_HOL,MMFE8_L3P7_HOL,MMFE8_L3P8_HOL,MMFE8_L3P1_IPR,MMFE8_L3P2_IPR,MMFE8_L3P3_IPR,MMFE8_L3P4_IPR,MMFE8_L3P6_IPR,MMFE8_L3P7_IPR,MMFE8_L3P8_IPR,MMFE8_L3P1_HOR,MMFE8_L3P2_HOR,MMFE8_L3P3_HOR,MMFE8_L3P4_HOR,MMFE8_L3P5_HOR,MMFE8_L3P6_HOR,MMFE8_L3P7_HOR,MMFE8_L3P8_HOR"

#LAYER4="MMFE8_L4P1_IPL,MMFE8_L4P2_IPL,MMFE8_L4P3_IPL,MMFE8_L4P4_IPL,MMFE8_L4P6_IPL,MMFE8_L4P7_IPL,MMFE8_L4P8_IPL,MMFE8_L4P1_HOL,MMFE8_L4P2_HOL,MMFE8_L4P3_HOL,MMFE8_L4P4_HOL,MMFE8_L4P5_HOL,MMFE8_L4P6_HOL,MMFE8_L4P7_HOL,MMFE8_L4P8_HOL,MMFE8_L4P1_IPR,MMFE8_L4P2_IPR,MMFE8_L4P3_IPR,MMFE8_L4P4_IPR,MMFE8_L4P6_IPR,MMFE8_L4P7_IPR,MMFE8_L4P8_IPR,MMFE8_L4P1_HOR,MMFE8_L4P2_HOR,MMFE8_L4P3_HOR,MMFE8_L4P4_HOR,MMFE8_L4P5_HOR,MMFE8_L4P6_HOR,MMFE8_L4P7_HOR,MMFE8_L4P8_HOR"

#Full list is here:
LAYER1="MMFE8_L1P1_IPL,MMFE8_L1P2_IPL,MMFE8_L1P3_IPL,MMFE8_L1P4_IPL,MMFE8_L1P5_IPL,MMFE8_L1P6_IPL,MMFE8_L1P7_IPL,MMFE8_L1P8_IPL,MMFE8_L1P1_HOL,MMFE8_L1P2_HOL,MMFE8_L1P3_HOL,MMFE8_L1P4_HOL,MMFE8_L1P5_HOL,MMFE8_L1P6_HOL,MMFE8_L1P7_HOL,MMFE8_L1P8_HOL,MMFE8_L1P1_IPR,MMFE8_L1P2_IPR,MMFE8_L1P3_IPR,MMFE8_L1P4_IPR,MMFE8_L1P5_IPR,MMFE8_L1P6_IPR,MMFE8_L1P7_IPR,MMFE8_L1P8_IPR,MMFE8_L1P1_HOR,MMFE8_L1P2_HOR,MMFE8_L1P3_HOR,MMFE8_L1P4_HOR,MMFE8_L1P5_HOR,MMFE8_L1P6_HOR,MMFE8_L1P7_HOR,MMFE8_L1P8_HOR"

LAYER2="MMFE8_L2P1_IPL,MMFE8_L2P2_IPL,MMFE8_L2P3_IPL,MMFE8_L2P4_IPL,MMFE8_L2P5_IPL,MMFE8_L2P6_IPL,MMFE8_L2P7_IPL,MMFE8_L2P8_IPL,MMFE8_L2P1_HOL,MMFE8_L2P2_HOL,MMFE8_L2P3_HOL,MMFE8_L2P4_HOL,MMFE8_L2P5_HOL,MMFE8_L2P6_HOL,MMFE8_L2P7_HOL,MMFE8_L2P8_HOL,MMFE8_L2P1_IPR,MMFE8_L2P2_IPR,MMFE8_L2P3_IPR,MMFE8_L2P4_IPR,MMFE8_L2P5_IPR,MMFE8_L2P6_IPR,MMFE8_L2P7_IPR,MMFE8_L2P8_IPR,MMFE8_L2P1_HOR,MMFE8_L2P2_HOR,MMFE8_L2P3_HOR,MMFE8_L2P4_HOR,MMFE8_L2P5_HOR,MMFE8_L2P6_HOR,MMFE8_L2P7_HOR,MMFE8_L2P8_HOR"

LAYER3="MMFE8_L3P1_IPL,MMFE8_L3P2_IPL,MMFE8_L3P3_IPL,MMFE8_L3P4_IPL,MMFE8_L3P5_IPL,MMFE8_L3P6_IPL,MMFE8_L3P7_IPL,MMFE8_L3P8_IPL,MMFE8_L3P1_HOL,MMFE8_L3P2_HOL,MMFE8_L3P3_HOL,MMFE8_L3P4_HOL,MMFE8_L3P5_HOL,MMFE8_L3P6_HOL,MMFE8_L3P7_HOL,MMFE8_L3P8_HOL,MMFE8_L3P1_IPR,MMFE8_L3P2_IPR,MMFE8_L3P3_IPR,MMFE8_L3P4_IPR,MMFE8_L3P5_IPR,MMFE8_L3P6_IPR,MMFE8_L3P7_IPR,MMFE8_L3P8_IPR,MMFE8_L3P1_HOR,MMFE8_L3P2_HOR,MMFE8_L3P3_HOR,MMFE8_L3P4_HOR,MMFE8_L3P5_HOR,MMFE8_L3P6_HOR,MMFE8_L3P7_HOR,MMFE8_L3P8_HOR"

LAYER4="MMFE8_L4P1_IPL,MMFE8_L4P2_IPL,MMFE8_L4P3_IPL,MMFE8_L4P4_IPL,MMFE8_L4P5_IPL,MMFE8_L4P6_IPL,MMFE8_L4P7_IPL,MMFE8_L4P8_IPL,MMFE8_L4P1_HOL,MMFE8_L4P2_HOL,MMFE8_L4P3_HOL,MMFE8_L4P4_HOL,MMFE8_L4P5_HOL,MMFE8_L4P6_HOL,MMFE8_L4P7_HOL,MMFE8_L4P8_HOL,MMFE8_L4P1_IPR,MMFE8_L4P2_IPR,MMFE8_L4P3_IPR,MMFE8_L4P4_IPR,MMFE8_L4P5_IPR,MMFE8_L4P6_IPR,MMFE8_L4P7_IPR,MMFE8_L4P8_IPR,MMFE8_L4P1_HOR,MMFE8_L4P2_HOR,MMFE8_L4P3_HOR,MMFE8_L4P4_HOR,MMFE8_L4P5_HOR,MMFE8_L4P6_HOR,MMFE8_L4P7_HOR,MMFE8_L4P8_HOR"
MMFE8S="${LAYER1},${LAYER2},${LAYER3},${LAYER4}"


NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=/afs/cern.ch/user/n/nswdaq/workspace/public/quick_and_dirty_baselines/baselines191/$NOW
WEB=/eos/atlas/atlascerngroupdisk/det-nsw/191/side${side}/Sector${sect_number}/Micromegas/baselines/${NOW}

CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/Defaults/full_sector_MM_default_191_baselines_${daqsector}.json"
if [ "$felix" == "flx12" ]
then
  CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/Defaults/full_sector_MM_default_191_baselines_${daqsector}_flx12.json"
fi

BIN="/afs/cern.ch/user/n/nswdaq/workspace/public/quick_and_dirty_baselines/x86_64-centos7-gcc8-opt/NSWConfiguration"
NSAMPS="200"

echo tmp dir: $OUT
echo web: $WEB
echo json: $CONFIG

# commands
#export not needed anymore
#export NOW
#$BIN/configure_frontend   -c $CONFIG -v -r -n $MMFE8S
# to switch to tdaq current release:
/afs/cern.ch/user/n/nswdaq/workspace/public/nswdaq/current/nswdaq/installed/x86_64-centos7-gcc8-opt/bin/configure_frontend/configure_frontend -c $CONFIG -v -r -n $MMFE8S
$BIN/read_channel_monitor -c $CONFIG -s $NSAMPS -o $OUT --baseline --dump -n $MMFE8S --threads 128
grep -h "^DATA" $OUT/baselines_MMFE8_L* > $OUT/baselines.txt
source /cvmfs/sft.cern.ch/lcg/releases/ROOT/6.16.00-fcdd1/x86_64-centos7-gcc8-opt/bin/thisroot.sh
# now -o specifies only the output directory (instead of the output file)                                                         
# now by default no MMFE8 plots. Add "-g layer" option to have MMFE8 plots grouped by layer
# now timestamp passed as argument with -t
# put ntuple in separate directory so that it is saved but not shown on webpage
mkdir $OUT/ntuple
python NSWConfiguration/dev/mm_baselines_FAST191.plot.py -t $NOW -i $OUT/baselines.txt -o $OUT -y 5 | tee $OUT/rms.txt

## copy to the web
#no need to produce .png and .pdf files -> ROOT2HTML is commented
#${ROOT2HTML} $OUT/baselines_${NOW}.root
echo "Moving to the webspace..."
mkdir $WEB
mv $OUT/baselines_plots.root $WEB/baselines_$NOW.root
mv $OUT/rms.txt $WEB
chmod -R g+wxr ${WEB}
echo "Moving plots to the webspace: done"
mv $OUT/ntuple $WEB
mv $OUT/baselines.txt $WEB
echo "Moving data to the webspace: done"
chmod -R g+wxr ${WEB}
echo $WEB
echo "https://nsw191.web.cern.ch/nsw191/"
rm -f $OUT/baselines_MMFE8_*txt
