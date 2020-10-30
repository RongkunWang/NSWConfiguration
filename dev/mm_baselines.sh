OLDSLICE="MMFE8_ML1P5_IPR,MMFE8_ML2P5_IPL,MMFE8_ML3P5_IPR,MMFE8_ML4P5_IPL,MMFE8_ML4P5_HOR,MMFE8_ML3P5_HOL,MMFE8_ML2P5_HOR,MMFE8_ML1P5_HOL"
#USE ME WHEN WE GET THE BOARD BACK IN
#NEWSLICE="MMFE8_ML1P5_IPL,MMFE8_ML2P5_IPR,MMFE8_ML3P5_IPL,MMFE8_ML4P5_IPR,MMFE8_ML4P5_HOL,MMFE8_ML3P5_HOR,MMFE8_ML2P5_HOL,MMFE8_ML1P5_HOR"

#USE ME NOW BECAUSE THE BOARD IS :X
NEWSLICE="MMFE8_ML1P5_IPL,MMFE8_ML2P5_IPR,MMFE8_ML4P5_IPR,MMFE8_ML4P5_HOL,MMFE8_ML3P5_HOR,MMFE8_ML2P5_HOL,MMFE8_ML1P5_HOR"
MMFE8S="${OLDSLICE},${NEWSLICE}"

#
# How to choose only a single board:
# MMFE8S="MMFE8_ML1P5_IPR"
#

# NOW="2019_07_16_11h99m99s"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=baselines/$NOW
WEB=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/baselines/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/baselines/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/root2html.py"

CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/mmfe8_bb5_16_boards_l1matching_nobypass_712_phase1_cosmicsSlice_cosmics_pcb5.json"
BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
NSAMPS="100"

# commands
$BIN/configure_frontend   -c $CONFIG -v -r
$BIN/read_channel_monitor -c $CONFIG -s $NSAMPS -o $OUT --baseline --dump -n $MMFE8S
grep -h "^DATA" $OUT/baselines_MMFE8_ML* > $OUT/baselines.txt
python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}.root -y 5 | tee $OUT/rms.txt
python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}_NewSlice.root -y 5 -n $NEWSLICE
python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}_OldSlice.root -y 5 -n $OLDSLICE

# copy to the web
${ROOT2HTML} $OUT/baselines_${NOW}.root
${ROOT2HTML} $OUT/baselines_${NOW}_NewSlice.root
${ROOT2HTML} $OUT/baselines_${NOW}_OldSlice.root
echo "Moving to the webspace..."
mv $OUT/baselines_${NOW}          ${WEB}/
mv $OUT/baselines_${NOW}_NewSlice ${WEB}_NewSlice/
mv $OUT/baselines_${NOW}_OldSlice ${WEB}_OldSlice/
cp $OUT/baselines_*.root          ${WEB}/
cp $OUT/baselines.txt             ${WEB}/
cp $OUT/rms.txt                   ${WEB}/
echo "<a href='${NOW}'>${NOW}         </a> :: INSERT DESCRIPTION HERE<br/>"            >> $LOG
echo "<a href='${NOW}_NewSlice'>${NOW}</a> :: INSERT DESCRIPTION HERE (new slice)<br/>" >> $LOG
echo "<a href='${NOW}_OldSlice'>${NOW}</a> :: INSERT DESCRIPTION HERE (old slice)<br/>" >> $LOG
echo "Update the log, please: $LOG"
echo "www.cern.ch/BB5Cosmics/html/baselines/"
# echo "www.cern.ch/BB5Cosmics/html/baselines/${NOW}/"
