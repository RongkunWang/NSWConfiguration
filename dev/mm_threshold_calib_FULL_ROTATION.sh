LAYER_1_2_IP="MMFE8_L1P1_IPL,MMFE8_L1P2_IPL,MMFE8_L1P3_IPL,MMFE8_L1P4_IPL,MMFE8_L1P5_IPL,MMFE8_L1P6_IPL,MMFE8_L1P7_IPL,MMFE8_L1P8_IPL,MMFE8_L1P1_IPR,MMFE8_L1P2_IPR,MMFE8_L1P3_IPR,MMFE8_L1P4_IPR,MMFE8_L1P5_IPR,MMFE8_L1P6_IPR,MMFE8_L1P7_IPR,MMFE8_L1P8_IPR,MMFE8_L2P1_IPL,MMFE8_L2P2_IPL,MMFE8_L2P3_IPL,MMFE8_L2P4_IPL,MMFE8_L2P5_IPL,MMFE8_L2P6_IPL,MMFE8_L2P7_IPL,MMFE8_L2P8_IPL,MMFE8_L2P1_IPR,MMFE8_L2P2_IPR,MMFE8_L2P3_IPR,MMFE8_L2P4_IPR,MMFE8_L2P5_IPR,MMFE8_L2P6_IPR,MMFE8_L2P7_IPR,MMFE8_L2P8_IPR"


LAYER_3_4_IP="MMFE8_L3P1_IPL,MMFE8_L3P2_IPL,MMFE8_L3P3_IPL,MMFE8_L3P4_IPL,MMFE8_L3P5_IPL,MMFE8_L3P6_IPL,MMFE8_L3P7_IPL,MMFE8_L3P8_IPL,MMFE8_L3P1_IPR,MMFE8_L3P2_IPR,MMFE8_L3P3_IPR,MMFE8_L3P4_IPR,MMFE8_L3P5_IPR,MMFE8_L3P6_IPR,MMFE8_L3P7_IPR,MMFE8_L3P8_IPR,MMFE8_L4P1_IPL,MMFE8_L4P2_IPL,MMFE8_L4P3_IPL,MMFE8_L4P4_IPL,MMFE8_L4P5_IPL,MMFE8_L4P6_IPL,MMFE8_L4P7_IPL,MMFE8_L4P8_IPL,MMFE8_L4P1_IPR,MMFE8_L4P2_IPR,MMFE8_L4P3_IPR,MMFE8_L4P4_IPR,MMFE8_L4P5_IPR,MMFE8_L4P6_IPR,MMFE8_L4P7_IPR,MMFE8_L4P8_IPR"


LAYER_1_2_HO="MMFE8_L1P1_HOL,MMFE8_L1P2_HOL,MMFE8_L1P3_HOL,MMFE8_L1P4_HOL,MMFE8_L1P5_HOL,MMFE8_L1P6_HOL,MMFE8_L1P7_HOL,MMFE8_L1P8_HOL,MMFE8_L1P1_HOR,MMFE8_L1P2_HOR,MMFE8_L1P3_HOR,MMFE8_L1P4_HOR,MMFE8_L1P5_HOR,MMFE8_L1P6_HOR,MMFE8_L1P7_HOR,MMFE8_L1P8_HOR,MMFE8_L2P1_HOL,MMFE8_L2P2_HOL,MMFE8_L2P3_HOL,MMFE8_L2P4_HOL,MMFE8_L2P5_HOL,MMFE8_L2P6_HOL,MMFE8_L2P7_HOL,MMFE8_L2P8_HOL,MMFE8_L2P1_HOR,MMFE8_L2P2_HOR,MMFE8_L2P3_HOR,MMFE8_L2P4_HOR,MMFE8_L2P5_HOR,MMFE8_L2P6_HOR,MMFE8_L2P7_HOR,MMFE8_L2P8_HOR"


LAYER_3_4_HO="MMFE8_L3P1_HOL,MMFE8_L3P2_HOL,MMFE8_L3P3_HOL,MMFE8_L3P4_HOL,MMFE8_L3P5_HOL,MMFE8_L3P6_HOL,MMFE8_L3P7_HOL,MMFE8_L3P8_HOL,MMFE8_L3P1_HOR,MMFE8_L3P2_HOR,MMFE8_L3P3_HOR,MMFE8_L3P4_HOR,MMFE8_L3P5_HOR,MMFE8_L3P6_HOR,MMFE8_L3P7_HOR,MMFE8_L3P8_HOR,MMFE8_L4P1_HOL,MMFE8_L4P2_HOL,MMFE8_L4P3_HOL,MMFE8_L4P4_HOL,MMFE8_L4P5_HOL,MMFE8_L4P6_HOL,MMFE8_L4P7_HOL,MMFE8_L4P8_HOL,MMFE8_L4P1_HOR,MMFE8_L4P2_HOR,MMFE8_L4P3_HOR,MMFE8_L4P4_HOR,MMFE8_L4P5_HOR,MMFE8_L4P6_HOR,MMFE8_L4P7_HOR,MMFE8_L4P8_HOR"



MMFE8S="${LAYER_1_2_IP},${LAYER_1_2_HO},${LAYER_3_4_IP},${LAYER_3_4_HO}"
#MMFE8S="MMFE8_L3P7_HOL"


#
# How to choose only a single board:
#  MMFE8S="MMFE8_L1P7_HOL,MMFE8_L2P7_HOL,MMFE8_L3P7_HOL,MMFE8_L4P7_HOL,MMFE8_L2P8_HOL,MMFE8_L3P6_HOL"
#

FILES="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files"
# CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/BB5/A12.1/full_small_sector_a12_bb5_cosmics.json"
CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/BB5/A15/full_large_sector_a15_bb5_baselines.json"
#CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/BB5/Tests/rotation_baselines_DCS.json"
# CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/BB5/A06/full_small_sector_a06_bb5_cosmics_pt100.json"

NSAMPS="10"

# NOW="2019_08_07_12h41m24s"
# RMS="6,9,12,4,5,7,8,10,11"
# RMS="5,6,7,8,9"
RMS="9"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=trimmers/$NOW
# THREADS=$(nproc)
THREADS=128

WEB=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/trimmers/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/trimmers/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/root2html.py"

BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
CALIBR="${BIN}/calibrate_trimmers_multirms"
#CALIBR="${BIN}/calibrate_trimmers_multirms_BB5 --remove_outliers"
INJECT="./bb5_analysis/scripts/injectTrimmerCalibration.py"


if [ -f "$ROOT2HTML" ]; then
    echo "$ROOT2HTML exists."
else
    echo "$ROOT2HTML does not exist, please do kinit first and source setup.sh"
    return 1
fi

mkdir -p ${OUT}

#
# ALWAYS CONFIGURE
#
$BIN/configure_frontend -c $CONFIG -v -r -m $THREADS -n $MMFE8S --resetvmm

# calibrate thresholds
time python NSWConfiguration/app/threshold_parallel.py -rms $RMS -c $CONFIG -s $CALIBR -f $MMFE8S -m $THREADS --remove_disconnected

# put thresholds into a new json
for R in $(echo ${RMS} | tr "," "\n"); do
    grep -h "^DATA_x${R}" thresholds_MMFE8_L* > $OUT/trimmers_x${R}.txt
    python $INJECT -i $CONFIG -c $OUT/trimmers_x${R}.txt -o ${OUT}/trimmers_x${R}.json -d 0
    echo
done

# baselines
grep -h "^DATA" baselines_MMFE8_L*VMM* > $OUT/baselines.txt

# mv
mkdir -p $OUT/log/
mv thresholds_MMFE8_L*    $OUT/log/
mv baselines_MMFE8_L*VMM* $OUT/log/

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
