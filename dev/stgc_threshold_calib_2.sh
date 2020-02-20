# TODO: sTGC 
FEBs="PFEB_L1Q1,PFEB_L1Q2,PFEB_L1Q3,SFEB_L1Q1,SFEB_L1Q2,SFEB_L1Q3,PFEB_L2Q1,PFEB_L2Q2,PFEB_L2Q3,SFEB_L2Q1,SFEB_L2Q2,SFEB_L2Q3,PFEB_L3Q1,PFEB_L3Q2,PFEB_L3Q3,SFEB_L3Q1,SFEB_L3Q2,SFEB_L3Q3,PFEB_L4Q1,PFEB_L4Q2,PFEB_L4Q3,SFEB_L4Q1,SFEB_L4Q2,SFEB_L4Q3"
#FEBs="SFEB_L3Q3"
#FEBs="SFEB_L1Q1_IPL,SFEB_L1Q2_IPL,SFEB_L1Q3_IPL,PFEB_L1Q1_IPR,PFEB_L1Q2_IPR,PFEB_L1Q3_IPR"
#FEBs="SFEB_L2Q1_IPR,SFEB_L2Q2_IPR,SFEB_L2Q3_IPR,PFEB_L2Q1_IPL,PFEB_L2Q2_IPL,PFEB_L2Q3_IPL"

PATTERN_baseline="?FEB*VMM*"
PATTERN_threshold="?FEB*vmm*"

#
# How to choose only a single board:
# FEBs="MMFE8_ML1P4_IPR"
#

#FILES="/afs/cern.ch/user/x/xuwa/public/"
FILES="/afs/cern.ch/work/n/nswdaq/public/sTGC_quick_and_dirty_baselines"
#CONFIG="${FILES}/flx2L2Q3.json"
#CONFIG="/afs/cern.ch/user/n/nswdaq/public/config-files-sTGC/wedge1B191layer1234_L2.json"
#CONFIG="${FILES}/wedge3gain3noise/baseline+30.json"
#CONFIG="${FILES}/test.json"
#CONFIG="/afs/cern.ch/user/n/nswdaq/public/wedge3flx2layer1234_final.json"
CONFIG="/afs/cern.ch/work/n/nswdaq/public/sTGC_quick_and_dirty_baselines/test_wedge4.json"

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
CALIBR="${BIN}/calibrate_trimmers_multirms_stgc"
INJECT="./bb5_analysis/scripts/injectTrimmerCalibration.py"

if [ "$#" -ne 2 ]; then
    echo "[Error] You must enter 2 arguments: #1 - 'S' or 'L' (Small or Large wedge) , #2 - 'P' or 'C' (Pivot or Confirm wedge)"
    return 1
fi

if [ "$1" != "S" -a "$1" != "L" ]; then
    echo "[Error] Argument #1 should be 'S' or 'L' (Small or Large wedge). "
    return 1
fi

if [ "$2" != "P" -a "$2" != "C" ]; then
    echo "[Error] Argument #2 should be 'P' or 'C' (Small or Large wedge). "
    return 1
fi

mkdir -p ${OUT}

#
# ALWAYS CONFIGURE
#
#$BIN/configure_frontend -c $CONFIG -v -t -r #-m $THREADS 
/afs/cern.ch/work/r/rowang/public/FELIX/NSWConfiguration_workspace/x86_64-centos7-gcc8-opt/NSWConfiguration/configure_frontend -c $CONFIG -v -r -t

# calibrate thresholds
time python NSWConfiguration/app/threshold_parallel_stgc.py -rms $RMS -c $CONFIG -s $CALIBR -f $FEBs  -m $THREADS

# copy those baseline in
for R in $(echo ${RMS} | tr "," "\n"); do
    grep -h "^DATA_x${R}" baselines_$PATTERN_baseline > $OUT/trimmers_x${R}.txt
    python $INJECT -i $CONFIG -c $OUT/trimmers_x${R}.txt -o ${OUT}/trimmers_x${R}.json -d 0
    echo
done

# baselines
grep -h "^DATA" baselines_$PATTERN_baseline > $OUT/baselines.txt
grep -h "^SUMMARY" summary_baselines_$PATTERN_baseline > $OUT/summary_baselines.txt
#grep -h "^BASELINE_OUTSIDE_150_200_MV" baseline_outside_150_200mV_$PATTERN_baseline > $OUT/baseline_outside_150_200mV.txt

# mv
mkdir -p $OUT/log/
#mv thresholds_$PATTERN_threshold    $OUT/log/
mv baselines_$PATTERN_baseline $OUT/log/
mv summary_baselines_$PATTERN_baseline $OUT/log/

for R in $(echo ${RMS} | tr "," "\n"); do
    
    ## plotting script                                                                                                                                                                                      
    python NSWConfiguration/dev/stgc_trimmers.plot_baselinescan.py -o $OUT/trimmers_${NOW}_x${R}.root \
        --bl           $OUT/baselines.txt \
        --bl_summ      $OUT/summary_baselines.txt \

    ## copy to the web                                                                                                                                                                                      
    $ROOT2HTML $OUT/trimmers_${NOW}_x${R}.root
    mv $OUT/trimmers_${NOW}_x${R} ${WEB}_x${R}
    echo "<a href='${NOW}_x${R}'> ${NOW}_x${R} </a> :: INSERT DESCRIPTION HERE<br/>" >> $LOG
    echo "Update the log, please: $LOG"
    echo "www.cern.ch/stgc-trimmer/"

    ## Plotting standard deviation on the baseline                                                                                                                       
    /afs/cern.ch/work/n/nswdaq/public/stgc_baselines/x86_64-centos7-gcc8-opt/stgc_baseline_noise_measurement/analyzeHit $OUT/summary_baselines.txt baseline_stdev_eachlayer_strip_pad.root $OUT/ $1 $2

    ## copy these stdev on baseline per layer plots for strip pad and wire to the webpage                                                                                                                   

    $ROOT2HTML $OUT/baseline_stdev_eachlayer_strip_pad.root
    mv $OUT/baseline_stdev_eachlayer_strip_pad ${WEB}_x${R}_stdev_baseline_perlayerPlots
    echo "<a href='${NOW}_x${R}_stdev_baseline_perlayerPlots'> ${NOW}_x${R}_stdev_baseline_perlayerPlots </a> :: INSERT DESCRIPTION HERE<br/>" >> $LOG
    echo "Update the log, please: $LOG"
    echo "www.cern.ch/stgc-trimmer/"

done
