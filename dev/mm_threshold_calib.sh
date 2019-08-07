OLDSLICE="MMFE8_ML1P4_IPR,MMFE8_ML2P4_IPL,MMFE8_ML3P4_IPR,MMFE8_ML4P4_IPL,MMFE8_ML4P4_HOR,MMFE8_ML3P4_HOL,MMFE8_ML2P4_HOR,MMFE8_ML1P4_HOL"
NEWSLICE="MMFE8_ML1P4_IPL,MMFE8_ML2P4_IPR,MMFE8_ML3P4_IPL,MMFE8_ML4P4_IPR,MMFE8_ML4P4_HOL,MMFE8_ML3P4_HOR,MMFE8_ML2P4_HOL,MMFE8_ML1P4_HOR"
# MMFE8S="${OLDSLICE},${NEWSLICE}"
MMFE8S="${OLDSLICE}"

#
# How to choose only a single board:
# MMFE8S="MMFE8_ML1P4_IPR"
#

CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/mmfe8_bb5_16_boards_l1matching_nobypass_712_phase1_cosmicsSlice_cosmics.json"
NSAMPS="100"

SCRIPT_CONFIG="./x86_64-centos7-gcc8-opt/NSWConfiguration/configure_frontend"
SCRIPT_MEASUR="./x86_64-centos7-gcc8-opt/NSWConfiguration/read_channel_thresholds"
SCRIPT_CALIBR="./x86_64-centos7-gcc8-opt/NSWConfiguration/calibrate_trimmers_multirms"
SCRIPT_INJECT="./bb5_analysis/scripts/injectTrimmerCalibration.py"

# NOW="2019_07_16_21h00m00s"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
RMS="12,10,8,6,4"

mkdir -p trimmers/${NOW}

#
# ALWAYS CONFIGURE
#
$SCRIPT_CONFIG -c $CONFIG -v -r

# calibrate thresholds and trimmers, 2019
time python3 NSWConfiguration/app/threshold_parallel.py -rms ${RMS} -c $CONFIG -s $SCRIPT_CALIBR -f $MMFE8S
for R in $(echo ${RMS} | tr "," "\n"); do
    grep -h "^DATA_x${R}" thresholds_MMFE8_ML* > trimmers/${NOW}/trimmers_x${R}.txt
    python $SCRIPT_INJECT -i $CONFIG -c trimmers/${NOW}/trimmers_x${R}.txt -o trimmers/${NOW}/trimmers_x${R}.json -d 0
done

