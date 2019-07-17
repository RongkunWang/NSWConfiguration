MMFE8S=""
MMFE8S="${MMFE8S},MMFE8_ML1P4_IPR"
MMFE8S="${MMFE8S},MMFE8_ML2P4_IPL"
MMFE8S="${MMFE8S},MMFE8_ML3P4_IPR"
MMFE8S="${MMFE8S},MMFE8_ML4P4_IPL"
MMFE8S="${MMFE8S},MMFE8_ML1P4_HOL"
MMFE8S="${MMFE8S},MMFE8_ML2P4_HOR"
MMFE8S="${MMFE8S},MMFE8_ML3P4_HOL"
MMFE8S="${MMFE8S},MMFE8_ML4P4_HOR"
MMFE8S="${MMFE8S},MMFE8_ML1P4_IPL"
MMFE8S="${MMFE8S},MMFE8_ML2P4_IPR"
MMFE8S="${MMFE8S},MMFE8_ML3P4_IPL"
MMFE8S="${MMFE8S},MMFE8_ML4P4_IPR"
MMFE8S="${MMFE8S},MMFE8_ML1P4_HOR"
MMFE8S="${MMFE8S},MMFE8_ML2P4_HOL"
MMFE8S="${MMFE8S},MMFE8_ML3P4_HOR"
MMFE8S="${MMFE8S},MMFE8_ML4P4_HOL"

# NOW="2019_07_07_14h00m00s"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=trimmers/$NOW
WEB=/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/trimmers/${NOW}/
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/root2html.py"

CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/mmfe8_bb5_16_boards_l1matching_nobypass_712_phase1_cosmicsSlice_cosmics.json"
BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
RMS="10"

mkdir -p $OUT

## trimmers -> json
$BIN/configure_frontend -c $CONFIG -v -r
$BIN/calibrate_trimmers -c $CONFIG -o $OUT/trimmers --rms_factor $RMS -n $MMFE8S | tee $OUT/log.txt
grep -h "^DATA" $OUT/trimmers/trimmers_MMFE8_ML* > $OUT/trimmers_x$RMS.txt
python bb5_analysis/scripts/injectTrimmerCalibration.py -c $OUT/trimmers_x$RMS.txt -i $CONFIG -o $OUT/trimmers_x$RMS.json -d 0
echo -e "\n -> $OUT/trimmers_x$RMS.json \n"
# cp trimmers_x${RMS}.txt $OUT/

## below: make fancy plots
## baselines and thresholds
CONFIG="$OUT/trimmers_x$RMS.json"
$BIN/read_channel_monitor -c $CONFIG -s 100 -o $OUT/baselines            --baseline  --dump -n $MMFE8S
$BIN/read_channel_monitor -c $CONFIG -s  10 -o $OUT/thresholds_trimmed   --threshold --dump -n $MMFE8S
$BIN/read_channel_monitor -c $CONFIG -s  10 -o $OUT/thresholds_untrimmed --threshold --dump -n $MMFE8S -T 0
grep -h "^DATA" $OUT/baselines/*.txt            > $OUT/baselines.txt
grep -h "^DATA" $OUT/thresholds_trimmed/*.txt   > $OUT/thresholds_trimmed.txt
grep -h "^DATA" $OUT/thresholds_untrimmed/*.txt > $OUT/thresholds_untrimmed.txt

## plotting script
python NSWConfiguration/dev/mm_trimmers.plot.py -o $OUT/trimmers_$NOW.root \
    --bl           $OUT/baselines.txt \
    --th_untrimmed $OUT/thresholds_untrimmed.txt \
    --th_trimmed   $OUT/thresholds_trimmed.txt

## copy to the web
$ROOT2HTML $OUT/trimmers_${NOW}.root
mv $OUT/trimmers_${NOW} ${WEB}
echo "www.cern.ch/BB5Cosmics/html/trimmers/${NOW}/"
