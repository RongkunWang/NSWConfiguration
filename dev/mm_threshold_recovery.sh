NOW="2021_02_20_18h45m32s"
OUT=trimmers/$NOW
THREADS=128
RMS="12"

WEB=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/trimmers/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/trimmers/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/root2html.py"

BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
#CALIBR="${BIN}/calibrate_trimmers_multirms_BB5"
#CALIBR="${BIN}/calibrate_trimmers_multirms_BB5 --remove_outliers"
#INJECT="./bb5_analysis/scripts/injectTrimmerCalibration.py"

mkdir -p ${OUT}

time python NSWConfiguration/app/threshold_parallel.py -rms $RMS 

# below: make fancy plots
for R in $(echo ${RMS} | tr "," "\n"); do

    # thresholds
    CONFIG="$OUT/trimmers_x${R}.json"
    $BIN/read_channel_monitor -c $CONFIG -s $NSAMPS -o $OUT/$R/thresholds_trimmed   --threshold --threads $THREADS --dump -n $MMFE8S
    $BIN/read_channel_monitor -c $CONFIG -s $NSAMPS -o $OUT/$R/thresholds_untrimmed --threshold --threads $THREADS --dump -n $MMFE8S -T 0
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
