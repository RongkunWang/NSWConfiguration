 NOW="2019_11_28_20h18m53s"
# NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=baselines/$NOW
WEB=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/baselines/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/baselines/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/root2html.py"

BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"

mkdir $OUT

# commands
python NSWConfiguration/dev/mm_baselines.plot.py -i /afs/cern.ch/work/n/nswdaq/public/quick_and_dirty_baselines/trimmers/2019_11_28_20h18m53s/baselines.txt -o $OUT/baselines_${NOW}.root -y 5 | tee $OUT/rms.txt
#python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}_L1.root -y 5 -n $LAYER1
#python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}_L2.root -y 5 -n $LAYER2


# copy to the web
${ROOT2HTML} $OUT/baselines_${NOW}.root
#${ROOT2HTML} $OUT/baselines_p
#${ROOT2HTML} $OUT/baselines_${NOW}_L1.root#
#${ROOT2HTML} $OUT/baselines_${NOW}_L2.root
<echo "Moving to the webspace..."
mv $OUT/baselines_${NOW}  ${WEB}/
#mv $OUT/baselines_${NOW}_NewSlice ${WEB}_L1/
#mv $OUT/baselines_${NOW}_OldSlice ${WEB}_L2/
cp $OUT/baselines_*.root          ${WEB}/
cp /afs/cern.ch/work/n/nswdaq/public/quick_and_dirty_baselines/trimmers/2019_11_28_20h18m53s/baselines.txt             ${WEB}/
cp $OUT/rms.txt                   ${WEB}/
echo "<a href='${NOW}'>${NOW}         </a> :: INSERT DESCRIPTION HERE <br/>"            >> $LOG
#echo "<a href='${NOW}_L1'>${NOW}</a> :: INSERT DESCRIPTION HERE (L1)<br/>" >> $LOG
#echo "<a href='${NOW}_L2'>${NOW}</a> :: INSERT DESCRIPTION HERE (L2)<br/>" >> $LOG
echo "Update the log, please: $LOG"
echo "www.cern.ch/BB5Cosmics/html/baselines/"
# echo "www.cern.ch/BB5Cosmics/html/baselines/${NOW}/"

