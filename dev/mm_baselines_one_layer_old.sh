# edited on 21.07.2020, ntekas, added check for root2html.py before running the script

LAYER1="MMFE8_L1P1_IPL,MMFE8_L1P2_IPL,MMFE8_L1P3_IPL,MMFE8_L1P4_IPL,MMFE8_L1P5_IPL,MMFE8_L1P6_IPL,MMFE8_L1P7_IPL,MMFE8_L1P8_IPL,MMFE8_L1P1_HOL,MMFE8_L1P2_HOL,MMFE8_L1P3_HOL,MMFE8_L1P4_HOL,MMFE8_L1P5_HOL,MMFE8_L1P6_HOL,MMFE8_L1P7_HOL,MMFE8_L1P8_HOL,MMFE8_L1P1_IPR,MMFE8_L1P2_IPR,MMFE8_L1P3_IPR,MMFE8_L1P4_IPR,MMFE8_L1P5_IPR,MMFE8_L1P6_IPR,MMFE8_L1P7_IPR,MMFE8_L1P8_IPR,MMFE8_L1P1_HOR,MMFE8_L1P2_HOR,MMFE8_L1P3_HOR,MMFE8_L1P4_HOR,MMFE8_L1P5_HOR,MMFE8_L1P6_HOR,MMFE8_L1P7_HOR,MMFE8_L1P8_HOR"

LAYER2="MMFE8_L2P1_IPL,MMFE8_L2P2_IPL,MMFE8_L2P3_IPL,MMFE8_L2P4_IPL,MMFE8_L2P5_IPL,MMFE8_L2P6_IPL,MMFE8_L2P7_IPL,MMFE8_L2P8_IPL,MMFE8_L2P1_HOL,MMFE8_L2P2_HOL,MMFE8_L2P3_HOL,MMFE8_L2P4_HOL,MMFE8_L2P5_HOL,MMFE8_L2P6_HOL,MMFE8_L2P7_HOL,MMFE8_L2P8_HOL,MMFE8_L2P1_IPR,MMFE8_L2P2_IPR,MMFE8_L2P3_IPR,MMFE8_L2P4_IPR,MMFE8_L2P5_IPR,MMFE8_L2P6_IPR,MMFE8_L2P7_IPR,MMFE8_L2P8_IPR,MMFE8_L2P1_HOR,MMFE8_L2P2_HOR,MMFE8_L2P3_HOR,MMFE8_L2P4_HOR,MMFE8_L2P5_HOR,MMFE8_L2P6_HOR,MMFE8_L2P7_HOR,MMFE8_L2P8_HOR"

LAYER3="MMFE8_L3P1_IPL,MMFE8_L3P2_IPL,MMFE8_L3P3_IPL,MMFE8_L3P4_IPL,MMFE8_L3P5_IPL,MMFE8_L3P6_IPL,MMFE8_L3P7_IPL,MMFE8_L3P8_IPL,MMFE8_L3P1_HOL,MMFE8_L3P2_HOL,MMFE8_L3P3_HOL,MMFE8_L3P4_HOL,MMFE8_L3P5_HOL,MMFE8_L3P6_HOL,MMFE8_L3P7_HOL,MMFE8_L3P8_HOL,MMFE8_L3P1_IPR,MMFE8_L3P2_IPR,MMFE8_L3P3_IPR,MMFE8_L3P4_IPR,MMFE8_L3P5_IPR,MMFE8_L3P6_IPR,MMFE8_L3P7_IPR,MMFE8_L3P8_IPR,MMFE8_L3P1_HOR,MMFE8_L3P2_HOR,MMFE8_L3P3_HOR,MMFE8_L3P4_HOR,MMFE8_L3P5_HOR,MMFE8_L3P6_HOR,MMFE8_L3P7_HOR,MMFE8_L3P8_HOR"

LAYER4="MMFE8_L4P1_IPL,MMFE8_L4P2_IPL,MMFE8_L4P3_IPL,MMFE8_L4P4_IPL,MMFE8_L4P5_IPL,MMFE8_L4P6_IPL,MMFE8_L4P7_IPL,MMFE8_L4P8_IPL,MMFE8_L4P1_HOL,MMFE8_L4P2_HOL,MMFE8_L4P3_HOL,MMFE8_L4P4_HOL,MMFE8_L4P5_HOL,MMFE8_L4P6_HOL,MMFE8_L4P7_HOL,MMFE8_L4P8_HOL,MMFE8_L4P1_IPR,MMFE8_L4P2_IPR,MMFE8_L4P3_IPR,MMFE8_L4P4_IPR,MMFE8_L4P5_IPR,MMFE8_L4P6_IPR,MMFE8_L4P7_IPR,MMFE8_L4P8_IPR,MMFE8_L4P1_HOR,MMFE8_L4P2_HOR,MMFE8_L4P3_HOR,MMFE8_L4P4_HOR,MMFE8_L4P5_HOR,MMFE8_L4P6_HOR,MMFE8_L4P7_HOR,MMFE8_L4P8_HOR"

##############MMFE8S="${LAYER1},${LAYER2},${LAYER3},${LAYER4}"

#
# How to choose only a single board
#################MMFE8S="${LAYER1}"
MMFE8S="MMFE8_L4P7_HOR"
#MMFE8S="MMFE8_L3P8_IPR"

# NOW="2019_07_16_11h99m99s"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=baselines/$NOW
WEB=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/baselines/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/baselines/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/root2html.py"

CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/BB5/A13/full_large_sector_a13_bb5_baselines.json"
BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
NSAMPS="100"

if [ -f "$ROOT2HTML" ]; then
    echo "$ROOT2HTML exists."
else
    echo "$ROOT2HTML does not exist, please do kinit first and source setup.sh"
    return 1
fi

# commands
#$BIN/configure_frontend   -c $CONFIG -v -r
#$BIN/read_channel_monitor -c $CONFIG -s $NSAMPS -o $OUT --baseline --dump --threads 128
$BIN/read_channel_monitor -c $CONFIG -s $NSAMPS -o $OUT --baseline --dump -n $MMFE8S --threads 32
grep -h "^DATA" $OUT/baselines_MMFE8_L* > $OUT/baselines.txt
export NOW
python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}.root -y 5 | tee $OUT/rms.txt
#python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}_L1.root -y 5 -n $LAYER1
#python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}_L2.root -y 5 -n $LAYER2


# copy to the web
${ROOT2HTML} $OUT/baselines_${NOW}.root
#${ROOT2HTML} $OUT/baselines_p
#${ROOT2HTML} $OUT/baselines_${NOW}_L1.root#
#${ROOT2HTML} $OUT/baselines_${NOW}_L2.root
echo "Moving to the webspace..."
mv $OUT/baselines_${NOW}  ${WEB}/
#mv $OUT/baselines_${NOW}_NewSlice ${WEB}_L1/
#mv $OUT/baselines_${NOW}_OldSlice ${WEB}_L2/
cp $OUT/baselines_*.root          ${WEB}/
cp $OUT/baselines.txt             ${WEB}/
cp $OUT/rms.txt                   ${WEB}/
echo "<a href='${NOW}'>${NOW}         </a> :: INSERT DESCRIPTION HERE <br/>"            >> $LOG
#echo "<a href='${NOW}_L1'>${NOW}</a> :: INSERT DESCRIPTION HERE (L1)<br/>" >> $LOG
#echo "<a href='${NOW}_L2'>${NOW}</a> :: INSERT DESCRIPTION HERE (L2)<br/>" >> $LOG
echo "Update the log, please: $LOG"
echo "www.cern.ch/BB5Cosmics/html/baselines/"
# echo "www.cern.ch/BB5Cosmics/html/baselines/${NOW}/"
rm -rf $OUT/baselines_${NOW}                                                                                                                                                            
rm -f Baselines.root
rm -f elx_report.pdf
rm -f elx_report.txt

