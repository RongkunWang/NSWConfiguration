# TODO: sTGC 
#FEBs="PFEB_L1Q1,PFEB_L1Q2,PFEB_L1Q3,SFEB_L1Q1,SFEB_L1Q2,SFEB_L1Q3,PFEB_L2Q1,PFEB_L2Q2,PFEB_L2Q3,SFEB_L2Q1,SFEB_L2Q2,SFEB_L2Q3,PFEB_L3Q1,PFEB_L3Q2,PFEB_L3Q3,SFEB_L3Q1,SFEB_L3Q2,SFEB_L3Q3,PFEB_L4Q1,PFEB_L4Q2,PFEB_L4Q3,SFEB_L4Q1,SFEB_L4Q2,SFEB_L4Q3"
#FEBs="SFEB6_L0Q0"
#FEBs="SFEB8_L0Q0,SFEB6_L0Q1,SFEB6_L0Q2"
FEBs="SFEB8_L1Q1,SFEB6_L1Q2,SFEB6_L1Q3"
#export PYTHONHOME=/usr/local/lib64/python3.6
#FEBs="PFEB_L1Q1,PFEB_L1Q2,PFEB_L1Q3"

PATTERN_baseline="?FEB*VMM*"
#PATTERN_threshold="?FEB*vmm*"

FILES="/afs/cern.ch/work/n/nswdaq/public/sTGC_quick_and_dirty_baselines"
#CONFIG="/afs/cern.ch/user/s/stgcic/public/baselinetemp.json"
CONFIG="/afs/cern.ch/user/s/stgcic/public/baselineflx2bench.json"
#CONFIG="/afs/cern.ch/user/s/stgcic/public/baseline_1sFEB_6VMMs_felix2_22093.json"
#CONFIG="/afs/cern.ch/user/s/stgcic/public/baseline_1sFEB_8VMMs_felix2.json"
#CONFIG="/afs/cern.ch/user/s/stgcic/public/benchreadout/felix2/3pFEB/baseline.json"
#CONFIG="/afs/cern.ch/user/s/stgcic/public/benchtest/felix2/3sFEB6/baseline3sFEBfelix2.json"
#CONFIG="/afs/cern.ch/user/s/stgcic/public/benchreadout/felix2/baseline3sFEBbench.json"
#CONFIG="/afs/cern.ch/user/s/stgcic/public/baseline_3sFEB_8VMMs_felix2.json"

NSAMPS="10000"

# NOW="2019_08_07_12h41m24s
# RMS="12,10,8,6,4"
RMS="30"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=baselines_sTGC_onbench/$NOW
THREADS=32

#WEB=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/baseline_on_bench/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/baseline_on_bench/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw-stgc/baseline_on_bench/root2html.py"

BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
CALIBR="${BIN}/calibrate_trimmers_multirms_stgc_withSCA"
INJECT="./bb5_analysis/scripts/injectTrimmerCalibration.py"

mkdir -p ${OUT}

python NSWConfiguration/dev/benchreadout.py "$OUT/"

SCAs=$(python NSWConfiguration/dev/parseBoardnames_benchTest.py $OUT/qrcode.txt $OUT/) 

newOUT=$OUT"_$SCAs"

echo "$SCAs"

cp -r --recursive $OUT $newOUT 

rm -rf $OUT

delimiter=_
s=$SCAs$delimiter
SCA_array="";

while [[ $s ]]; do
    SCA_array=$SCA_array"${s%%"$delimiter"*},";
    s=${s#*"$delimiter"};
done;

SCA_array="${SCA_array::-1}"
echo "$SCA_array"

NOW_NEW=$NOW"_$SCAs"
WEB=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/baseline_on_bench/${NOW_NEW}

#
# ALWAYS CONFIGURE
#
#/afs/cern.ch/work/r/rowang/public/FELIX/NSWConfiguration_workspace/x86_64-centos7-gcc8-opt/NSWConfiguration/configure_frontend -c $CONFIG -v -r -t

# calibrate baselines
time python NSWConfiguration/app/threshold_parallel_stgc_withSCA.py -rms $RMS -c $CONFIG -s $CALIBR -f $FEBs  -m $THREADS -sca $SCA_array

# baselines
grep -h "^DATA" baselines_$PATTERN_baseline > $newOUT/baselines.txt
grep -h "^SUMMARY" summary_baselines_$PATTERN_baseline > $newOUT/summary_baselines.txt
grep -h "^BASELINE_OUTSIDE_150_200_MV" baselines_outside150to200mV_$PATTERN_baseline > $newOUT/baselines_outside150to200mV.txt
# mv
mkdir -p $newOUT/log/
mv baselines_$PATTERN_baseline $newOUT/log/
mv summary_baselines_$PATTERN_baseline $newOUT/log/
mv baselines_outside150to200mV_$PATTERN_baseline $newOUT/log/
    
## plotting script                                                                                                                                                                                      
python NSWConfiguration/dev/stgc_trimmers.plot_baselinescan.py -o $newOUT/baselines_${NOW_NEW}.root \
    --bl           $newOUT/baselines.txt \
    --bl_summ      $newOUT/summary_baselines.txt \
    --SCAs         $SCA_array

## copy to the web                                                                                                                                                                                      
$ROOT2HTML $newOUT/baselines_${NOW_NEW}.root
mv $newOUT/baselines_${NOW_NEW} ${WEB}
echo "<a href='${NOW_NEW}'> ${NOW_NEW} </a> :: On-bench Reception Test sFEBs repair Dec 16 <br/>" >> $LOG
echo "Update the log, please: $LOG"
echo "www.cern.ch/stgc-threshold-on-bench/"

## Plotting standard deviation on the threshold                                                                                                                       
#/afs/cern.ch/work/n/nswdaq/public/stgc_baselines/x86_64-centos7-gcc8-opt/stgc_threshold_noise_measurement/analyzeHit $newOUT/summary_baselines.txt threshold_stdev_eachlayer_strip_pad.root $newOUT/ $1 $2

## copy these stdev on threshold per layer plots for strip pad and wire to the webpage                                                                                                                   

#$ROOT2HTML $newOUT/threshold_stdev_eachlayer_strip_pad.root
#mv $newOUT/threshold_stdev_eachlayer_strip_pad ${WEB}_stdev_threshold_perlayerPlots
#echo "<a href='${NOW}_stdev_threshold_perlayerPlots'> ${NOW}_stdev_threshold_perlayerPlots </a> :: INSERT DESCRIPTION HERE<br/>" >> $LOG
#echo "Update the log, please: $LOG"
#echo "www.cern.ch/stgc-trimmer/"


