# TODO: sTGC 
#FEBs="PFEB_L1Q1,PFEB_L1Q2,PFEB_L1Q3,SFEB_L1Q1,SFEB_L1Q2,SFEB_L1Q3,PFEB_L2Q1,PFEB_L2Q2,PFEB_L2Q3,SFEB_L2Q1,SFEB_L2Q2,SFEB_L2Q3,PFEB_L3Q1,PFEB_L3Q2,PFEB_L3Q3,SFEB_L3Q1,SFEB_L3Q2,SFEB_L3Q3,PFEB_L4Q1,PFEB_L4Q2,PFEB_L4Q3,SFEB_L4Q1,SFEB_L4Q2,SFEB_L4Q3"

FEBs="SFEB_L1Q1,SFEB_L1Q2,SFEB_L1Q3"

PATTERN_baseline="?FEB*VMM*"
PATTERN_threshold="?FEB*vmm*"

FILES="/afs/cern.ch/work/n/nswdaq/public/sTGC_quick_and_dirty_baselines"
CONFIG="/afs/cern.ch/user/s/stgcic/public/baselinetemp.json"

NSAMPS="10000"

# NOW="2019_08_07_12h41m24s"
# RMS="12,10,8,6,4"
RMS="30"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)_testOnBench_8vmmsSFEB_test4_baseline_flx2
OUT=baselines_sTGC_onbench/$NOW
THREADS=32

WEB=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/baseline_on_bench/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/baseline_on_bench/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw-stgc/baseline_on_bench/root2html.py"

BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
CALIBR="${BIN}/calibrate_trimmers_multirms_stgc"
INJECT="./bb5_analysis/scripts/injectTrimmerCalibration.py"

mkdir -p ${OUT}



#
# ALWAYS CONFIGURE
#
#/afs/cern.ch/work/r/rowang/public/FELIX/NSWConfiguration_workspace/x86_64-centos7-gcc8-opt/NSWConfiguration/configure_frontend -c $CONFIG -v -r -t

# calibrate baselines
time python NSWConfiguration/app/threshold_parallel_stgc.py -rms $RMS -c $CONFIG -s $CALIBR -f $FEBs  -m $THREADS

# baselines
grep -h "^DATA" baselines_$PATTERN_threshold > $OUT/baselines.txt
grep -h "^SUMMARY" summary_baselines_$PATTERN_threshold > $OUT/summary_baselines.txt

# mv
mkdir -p $OUT/log/
mv baselines_$PATTERN_threshold $OUT/log/
mv summary_baselines_$PATTERN_threshold $OUT/log/


    
## plotting script                                                                                                                                                                                      
python NSWConfiguration/dev/stgc_trimmers.plot_baselinescan.py -o $OUT/baselines_${NOW}.root \
    --bl           $OUT/baselines.txt \
    --bl_summ      $OUT/summary_baselines.txt \
    
## copy to the web                                                                                                                                                                                      
$ROOT2HTML $OUT/baselines_${NOW}.root
mv $OUT/baselines_${NOW} ${WEB}
echo "<a href='${NOW}'> ${NOW} </a> :: INSERT DESCRIPTION HERE<br/>" >> $LOG
echo "Update the log, please: $LOG"
echo "www.cern.ch/stgc-threshold-on-bench/"

## Plotting standard deviation on the threshold                                                                                                                       
#/afs/cern.ch/work/n/nswdaq/public/stgc_baselines/x86_64-centos7-gcc8-opt/stgc_threshold_noise_measurement/analyzeHit $OUT/summary_baselines.txt threshold_stdev_eachlayer_strip_pad.root $OUT/ $1 $2

## copy these stdev on threshold per layer plots for strip pad and wire to the webpage                                                                                                                   

#$ROOT2HTML $OUT/threshold_stdev_eachlayer_strip_pad.root
#mv $OUT/threshold_stdev_eachlayer_strip_pad ${WEB}_stdev_threshold_perlayerPlots
#echo "<a href='${NOW}_stdev_threshold_perlayerPlots'> ${NOW}_stdev_threshold_perlayerPlots </a> :: INSERT DESCRIPTION HERE<br/>" >> $LOG
#echo "Update the log, please: $LOG"
#echo "www.cern.ch/stgc-trimmer/"


