# TODO: sTGC 
#FEBs="PFEB_L1Q1,PFEB_L1Q2,PFEB_L1Q3,SFEB_L1Q1,SFEB_L1Q2,SFEB_L1Q3,PFEB_L2Q1,PFEB_L2Q2,PFEB_L2Q3,SFEB_L2Q1,SFEB_L2Q2,SFEB_L2Q3,PFEB_L3Q1,PFEB_L3Q2,PFEB_L3Q3,SFEB_L3Q1,SFEB_L3Q2,SFEB_L3Q3,PFEB_L4Q1,PFEB_L4Q2,PFEB_L4Q3,SFEB_L4Q1,SFEB_L4Q2,SFEB_L4Q3"\
FEBs="SFEB_L3Q3"
VMM="4"
#FEBs="PFEB_L1Q1"
#FEBs="PFEB_L2Q3"
#FEBs="SFEB_L1Q1_IPL,SFEB_L1Q2_IPL,SFEB_L1Q3_IPL,PFEB_L1Q1_IPR,PFEB_L1Q2_IPR,PFEB_L1Q3_IPR"
#FEBs="SFEB_L2Q1_IPR,SFEB_L2Q2_IPR,SFEB_L2Q3_IPR,PFEB_L2Q1_IPL,PFEB_L2Q2_IPL,PFEB_L2Q3_IPL"

PATTERN_baseline="?FEB*VMM*"
PATTERN_threshold="?FEB*vmm*"

#
# How to choose only a single board:
# FEBs="MMFE8_ML1P4_IPR"
#

#FILES="/afs/cern.ch/user/x/xuwa/public/wedgereadouttest"
FILES="/afs/cern.ch/user/x/xuwa/public"
#CONFIG="${FILES}/flx2L2Q3.json"
#CONFIG="/afs/cern.ch/user/n/nswdaq/public/config-files-sTGC/wedge1B191layer1234_L2.json"
#CONFIG="${FILES}/wedge3flx2layer1234.json"
CONFIG="${FILES}/wedge3gain3noise/baseline+30.json"

NSAMPS="10000"

NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)_DebugNoBaseline
OUT=trimmers_sTGC/$NOW
THREADS=1

WEB=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/trimmers/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/trimmers/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw-stgc/trimmers/root2html.py"

BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
CALIBR="${BIN}/calibrate_trimmers_multirms"
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
$BIN/configure_frontend -c $CONFIG -v -r -m $THREADS 

$BIN/read_channel_monitor -c $CONFIG -s $NSAMPS -o $OUT/baselines_channelMonitorOutput --baseline --threads $THREADS --dump -n $FEBs -V $VMM
grep -h "^DATA" $OUT/baselines_channelMonitorOutput/baselines_*.txt   > $OUT/baselines_channelMonitorOutput/baselines.txt    
grep -h "^SUMMARY" $OUT/baselines_channelMonitorOutput/summary_baselines_*.txt   > $OUT/baselines_channelMonitorOutput/summary_baselines.txt

## plotting script                                                                                                                                                                                      
python NSWConfiguration/dev/stgc_trimmers.plot_baselinescan.py -o $OUT/baselines_channelMonitorOutput/baselinescan_${NOW}.root \
    --bl           $OUT/baselines_channelMonitorOutput/baselines.txt \
    --bl_summ      $OUT/baselines_channelMonitorOutput/summary_baselines.txt

## copy to the web                                                                                                                                                                                      
$ROOT2HTML $OUT/baselines_channelMonitorOutput/baselinescan_${NOW}.root
mv $OUT/baselines_channelMonitorOutput/baselinescan_${NOW} ${WEB}_baselinescan
echo "<a href='${NOW}_baselinescan'> ${NOW}_baselinescan </a> :: INSERT DESCRIPTION HERE<br/>" >> $LOG
echo "Update the log, please: $LOG"
echo "www.cern.ch/stgc-trimmer/"

## Plotting standard deviation on the baseline                                                                                                                                                          

/afs/cern.ch/work/n/nswdaq/public/stgc_baselines/x86_64-centos7-gcc8-opt/stgc_baseline_noise_measurement/analyzeHit $OUT/baselines_channelMonitorOutput/summary_baselines.txt baseline_stdev_eachlayer_strip_pad.root $OUT/baselines_channelMonitorOutput/ $1 $2

## copy these stdev on baseline per layer plots for strip pad and wire to the webpage                                                                                                                  

$ROOT2HTML $OUT/baselines_channelMonitorOutput/baseline_stdev_eachlayer_strip_pad.root
mv $OUT/baselines_channelMonitorOutput/baseline_stdev_eachlayer_strip_pad ${WEB}_baselinescan_stdev_baseline_perlayerPlots
echo "<a href='${NOW}_baselinescan_stdev_baseline_perlayerPlots'> ${NOW}_baselinescan_stdev_baseline_perlayerPlots </a> :: INSERT DESCRIPTION HERE<br/>" >> $LOG
echo "Update the log, please: $LOG"
echo "www.cern.ch/stgc-trimmer/"
