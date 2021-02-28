# TODO: sTGC 
#FEBs="PFEB_L1Q1,PFEB_L1Q2,PFEB_L1Q3,SFEB_L1Q1,SFEB_L1Q2,SFEB_L1Q3,PFEB_L2Q1,PFEB_L2Q2,PFEB_L2Q3,SFEB_L2Q1,SFEB_L2Q2,SFEB_L2Q3,PFEB_L3Q1,PFEB_L3Q2,PFEB_L3Q3,SFEB_L3Q1,SFEB_L3Q2,SFEB_L3Q3,PFEB_L4Q1,PFEB_L4Q2,PFEB_L4Q3,SFEB_L4Q1,SFEB_L4Q2,SFEB_L4Q3"
FEBs="SFEB_L1Q1_HOL"


PATTERN_baseline="?FEB*VMM*"
PATTERN_threshold="?FEB*vmm*"

#
# How to choose only a single board:
# FEBs="MMFE8_ML1P4_IPR"
#

FILES="/afs/cern.ch/user/x/xuwa/public/wedgereadouttest"
#CONFIG="${FILES}/flx2layer1234.json"
CONFIG="${FILES}/sFEBlink1.json"
NSAMPS="100"

# NOW="2019_08_07_12h41m24s"
# RMS="12,10,8,6,4"
RMS="30"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)_DebugNoBaseline
OUT=trimmers_sTGC/$NOW
THREADS=32

WEB=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/trimmers/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/trimmers/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw-stgc/trimmers/root2html.py"

BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
CALIBR="${BIN}/calibrate_trimmers_multirms"
INJECT="./bb5_analysis/scripts/injectTrimmerCalibration.py"

mkdir -p ${OUT}

#
# ALWAYS CONFIGURE
#
$BIN/configure_frontend -c $CONFIG -v -r -m $THREADS 

# calibrate thresholds
time python NSWConfiguration/app/threshold_parallel.py -rms $RMS -c $CONFIG -s $CALIBR -f $FEBs  -m $THREADS

# copy those baseline in
for R in $(echo ${RMS} | tr "," "\n"); do
    grep -h "^DATA_x${R}" thresholds_$PATTERN_threshold > $OUT/trimmers_x${R}.txt
    python $INJECT -i $CONFIG -c $OUT/trimmers_x${R}.txt -o ${OUT}/trimmers_x${R}.json -d 0
    echo
done

# baselines
grep -h "^DATA" baselines_$PATTERN_baseline > $OUT/baselines.txt
grep -h "^SUMMARY" summary_baselines_$PATTERN_baseline > $OUT/summary_baselines.txt

# mv
mkdir -p $OUT/log/
mv thresholds_$PATTERN_threshold    $OUT/log/
mv baselines_$PATTERN_baseline $OUT/log/
mv summary_baselines_$PATTERN_baseline $OUT/log/

# below: make fancy plots
for R in $(echo ${RMS} | tr "," "\n"); do

    # thresholds
    CONFIG="$OUT/trimmers_x${R}.json"
    $BIN/read_channel_monitor -c $CONFIG -s  10 -o $OUT/$R/thresholds_trimmed   --threshold  --threads $THREADS --dump -n $FEBs
    $BIN/read_channel_monitor -c $CONFIG -s  10 -o $OUT/$R/thresholds_untrimmed --threshold  --threads $THREADS --dump -n $FEBs -T 0
    grep -h "^DATA" $OUT/$R/thresholds_trimmed/*.txt   > $OUT/$R/thresholds_trimmed.txt
    grep -h "^DATA" $OUT/$R/thresholds_untrimmed/*.txt > $OUT/$R/thresholds_untrimmed.txt

    ## plotting script
    python NSWConfiguration/dev/stgc_trimmers.plot.py -o $OUT/trimmers_${NOW}_x${R}.root \
        --bl           $OUT/baselines.txt \
        --th_untrimmed $OUT/$R/thresholds_untrimmed.txt \
        --th_trimmed   $OUT/$R/thresholds_trimmed.txt

    ## copy to the web
    $ROOT2HTML $OUT/trimmers_${NOW}_x${R}.root
    mv $OUT/trimmers_${NOW}_x${R} ${WEB}_x${R}
    echo "<a href='${NOW}_x${R}'> ${NOW}_x${R} </a> :: INSERT DESCRIPTION HERE<br/>" >> $LOG
    echo "Update the log, please: $LOG"
    echo "www.cern.ch/stgc-trimmer/"

done
