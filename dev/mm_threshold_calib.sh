OLDSLICE="MMFE8_ML1P4_IPR,MMFE8_ML2P4_IPL,MMFE8_ML3P4_IPR,MMFE8_ML4P4_IPL,MMFE8_ML4P4_HOR,MMFE8_ML3P4_HOL,MMFE8_ML2P4_HOR,MMFE8_ML1P4_HOL"
NEWSLICE="MMFE8_ML1P4_IPL,MMFE8_ML2P4_IPR,MMFE8_ML3P4_IPL,MMFE8_ML4P4_IPR,MMFE8_ML4P4_HOL,MMFE8_ML3P4_HOR,MMFE8_ML2P4_HOL,MMFE8_ML1P4_HOR"
# MMFE8S="${OLDSLICE},${NEWSLICE}"
MMFE8S="${OLDSLICE}"

#
# How to choose only a single board:
# MMFE8S="MMFE8_ML1P4_IPR"
#

FILES="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files"
CONFIG="${FILES}/mmfe8_bb5_16_boards_l1matching_nobypass_712_phase1_cosmicsSlice_cosmics.json"
NSAMPS="100"

# NOW="2019_07_16_21h00m00s"
RMS="12,10,8,6,4"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=trimmers/$NOW

WEB=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/trimmers/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/trimmers/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/root2html.py"

BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
CALIBR="${BIN}/calibrate_trimmers_multirms"
INJECT="./bb5_analysis/scripts/injectTrimmerCalibration.py"

mkdir -p ${OUT}

#
# ALWAYS CONFIGURE
#
$BIN/configure_frontend -c $CONFIG -v -r

# calibrate thresholds
time python3 NSWConfiguration/app/threshold_parallel.py -rms $RMS -c $CONFIG -s $CALIBR -f $MMFE8S
for R in $(echo ${RMS} | tr "," "\n"); do
    grep -h "^DATA_x${R}" thresholds_MMFE8_ML* > $OUT/trimmers_x${R}.txt
    python $INJECT -i $CONFIG -c $OUT/trimmers_x${R}.txt -o ${OUT}/trimmers_x${R}.json -d 0
done

# baselines
grep -h "^DATA" baselines_MMFE8_ML*VMM* > $OUT/baselines.txt

# below: make fancy plots
for R in $(echo ${RMS} | tr "," "\n"); do

    # thresholds
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
    echo "<a href='${NOW}_x${R}'> ${NOW}_x${R} </a> :: INSERT DESCRIPTION HERE<br/>" >> $LOG
    echo "Update the log, please: $LOG"
    echo "www.cern.ch/BB5Cosmics/html/trimmers/"

done
