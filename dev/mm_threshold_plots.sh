OLDSLICE="MMFE8_ML1P4_IPR,MMFE8_ML2P4_IPL,MMFE8_ML3P4_IPR,MMFE8_ML4P4_IPL,MMFE8_ML4P4_HOR,MMFE8_ML3P4_HOL,MMFE8_ML2P4_HOR,MMFE8_ML1P4_HOL"
NEWSLICE="MMFE8_ML1P4_IPL,MMFE8_ML2P4_IPR,MMFE8_ML3P4_IPL,MMFE8_ML4P4_IPR,MMFE8_ML4P4_HOL,MMFE8_ML3P4_HOR,MMFE8_ML2P4_HOL,MMFE8_ML1P4_HOR"
# MMFE8S="${OLDSLICE},${NEWSLICE}"
MMFE8S="${OLDSLICE}"

NOW="2019_07_18_18h45m55s"
OUT=trimmers/$NOW
WEB=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/trimmers/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/trimmers/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/root2html.py"

CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/mmfe8_bb5_16_boards_l1matching_nobypass_712_phase1_cosmicsSlice_cosmics.json"
BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
RMS="12,10,8,6,4"

mkdir -p $OUT
$BIN/read_channel_monitor -c $CONFIG -s 50 -o $OUT/baselines --baseline --dump -n $MMFE8S
grep -h "^DATA" $OUT/baselines/*.txt > $OUT/baselines.txt

## below: make fancy plots
## baselines and thresholds
for R in $(echo ${RMS} | tr "," "\n"); do

    CONFIG="$OUT/trimmers_x${R}.json"
    $BIN/read_channel_monitor -c $CONFIG -s  10 -o $OUT/$R/thresholds_trimmed   --threshold --dump -n $MMFE8S
    $BIN/read_channel_monitor -c $CONFIG -s  10 -o $OUT/$R/thresholds_untrimmed --threshold --dump -n $MMFE8S -T 0
    grep -h "^DATA" $OUT/$R/thresholds_trimmed/*.txt   > $OUT/$R/thresholds_trimmed.txt
    grep -h "^DATA" $OUT/$R/thresholds_untrimmed/*.txt > $OUT/$R/thresholds_untrimmed.txt

    ## plotting script
    python NSWConfiguration/dev/mm_trimmers.plot.py -o $OUT/trimmers_${NOW}_x${R}.root \
        --bl           $OUT/baselines.txt \
        --th_untrimmed $OUT/$R/thresholds_untrimmed.txt \
        --th_trimmed   $OUT/$R/thresholds_trimmed.txt

    ## copy to the web
    $ROOT2HTML $OUT/trimmers_${NOW}_x${R}.root
    mv $OUT/trimmers_${NOW}_x${R} ${WEB}_x${R}
    # echo "www.cern.ch/BB5Cosmics/html/trimmers/${NOW}_x${R}/"
    echo "<a href='${NOW}_x${R}'> ${NOW}_x${R} </a> :: INSERT DESCRIPTION HERE<br/>" >> $LOG
    echo "Update the log, please: $LOG"
    echo "www.cern.ch/BB5Cosmics/html/trimmers/"

done
