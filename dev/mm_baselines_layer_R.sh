LAYER1="MMFE8_L1P1_IPR,MMFE8_L1P2_IPR,MMFE8_L1P3_IPR,MMFE8_L1P4_IPR,MMFE8_L1P5_IPR,MMFE8_L1P6_IPR,MMFE8_L1P7_IPR,MMFE8_L1P8_IPR,MMFE8_L1P1_HOR,MMFE8_L1P2_HOR,MMFE8_L1P3_HOR,MMFE8_L1P4_HOR,MMFE8_L1P5_HOR,MMFE8_L1P6_HOR,MMFE8_L1P7_HOR,MMFE8_L1P8_HOR"

LAYER2="MMFE8_L2P1_IPR,MMFE8_L2P2_IPR,MMFE8_L2P3_IPR,MMFE8_L2P4_IPR,MMFE8_L2P5_IPR,MMFE8_L2P6_IPR,MMFE8_L2P7_IPR,MMFE8_L2P8_IPR,MMFE8_L2P1_HOR,MMFE8_L2P2_HOR,MMFE8_L2P3_HOR,MMFE8_L2P4_HOR,MMFE8_L2P5_HOR,MMFE8_L2P6_HOR,MMFE8_L2P7_HOR,MMFE8_L2P8_HOR"

LAYER3="MMFE8_L3P1_IPR,MMFE8_L3P2_IPR,MMFE8_L3P3_IPR,MMFE8_L3P4_IPR,MMFE8_L3P5_IPR,MMFE8_L3P6_IPR,MMFE8_L3P7_IPR,MMFE8_L3P8_IPR,MMFE8_L3P1_HOR,MMFE8_L3P2_HOR,MMFE8_L3P3_HOR,MMFE8_L3P4_HOR,MMFE8_L3P5_HOR,MMFE8_L3P6_HOR,MMFE8_L3P7_HOR,MMFE8_L3P8_HOR"

LAYER4="MMFE8_L4P1_IPR,MMFE8_L4P2_IPR,MMFE8_L4P3_IPR,MMFE8_L4P4_IPR,MMFE8_L4P5_IPR,MMFE8_L4P6_IPR,MMFE8_L4P7_IPR,MMFE8_L4P8_IPR,MMFE8_L4P1_HOR,MMFE8_L4P2_HOR,MMFE8_L4P3_HOR,MMFE8_L4P4_HOR,MMFE8_L4P5_HOR,MMFE8_L4P6_HOR,MMFE8_L4P7_HOR,MMFE8_L4P8_HOR"


MMFE8S="${LAYER1}"
#MMFE8S="${LAYER1},${LAYER2}"
#MMFE8S="${LAYER4}"
#MMFE8S="MMFE8_L2P4_HOR"


#
# How to choose only a single board
#MMFE8S="MMFE8_ML1P5_IPR"
#

# NOW="2019_07_16_11h99m99s"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=baselines/$NOW
WEB=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/baselines/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/baselines/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/root2html.py"

CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/full_small_sector_12_191_ONLY_RIGHT_di.json"
BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
NSAMPS="100"

# commands
$BIN/configure_frontend   -c $CONFIG -v -r -n $MMFE8S
$BIN/read_channel_monitor -c $CONFIG -s $NSAMPS -o $OUT --baseline --dump -n $MMFE8S --threads 16
grep -h "^DATA" $OUT/baselines_MMFE8_L* > $OUT/baselines.txt
python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}.root -y 5 | tee $OUT/rms.txt
#python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}_L1.root -y 5 -n $LAYER1
#python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}_L2.root -y 5 -n $LAYER2

# copy to the web
${ROOT2HTML} $OUT/baselines_${NOW}.root
<echo "Moving to the webspace..."
mv $OUT/baselines_${NOW}          ${WEB}/
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
#echo "www.cern.ch/BB5Cosmics/html/baselines/${NOW}/"

