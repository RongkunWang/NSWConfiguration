NOW="2020_11_04_15h14m11s"
#NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=baselines/$NOW
#OUT2=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/baselines/${NOW}
WEB=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/baselines/${NOW}
#WEB=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/web/SDW/A14_sector/Micromegas/baselines/${NOW} 
LOG=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/baselines/index.html
#DIR=/afs/cern.ch/work/n/nswdaq/public/quick_and_dirty_baselines/nn
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/root2html.py"

# commands
export NOW 
python NSWConfiguration/dev/mm_baselines_NEW_LM.plot.py -t LM -i $OUT/baselines.txt -o $OUT/baselines_${NOW}.root -y 5 | tee $OUT/rms.txt

# copy to the web
${ROOT2HTML} $OUT/baselines_${NOW}.root

echo "Moving to the webspace..."

mv $OUT/baselines_${NOW}          ${WEB}/
cp $OUT/baselines_*.root          ${WEB}/
cp $OUT/baselines.txt             ${WEB}/
cp $OUT/rms.txt                   ${WEB}/
mv -f ${WEB}/baselines_${NOW}/*   ${WEB}/
rm -rf ${WEB}/baselines_${NOW}/
echo "<a href='${NOW}'>${NOW}         </a> :: INSERT DESCRIPTION HERE <br/>"            >> $LOG
echo "Update the log, please: $LOG"
echo "www.cern.ch/BB5Cosmics/html/baselines/"
# echo "www.cern.ch/BB5Cosmics/html/baselines/${NOW}/"
##rm -f $OUT/baselines_${NOW}.root
#rm -f Baselines.root
#rm -f elx_report.pdf
#rm -f elx_report.txt
