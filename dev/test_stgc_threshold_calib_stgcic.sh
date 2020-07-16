# TODO: sTGC 
FEBs="PFEB_L1Q1,PFEB_L1Q2,PFEB_L1Q3,SFEB8_L1Q1,SFEB6_L1Q2,SFEB6_L1Q3,PFEB_L2Q1,PFEB_L2Q2,PFEB_L2Q3,SFEB8_L2Q1,SFEB6_L2Q2,SFEB6_L2Q3,PFEB_L3Q1,PFEB_L3Q2,PFEB_L3Q3,SFEB8_L3Q1,SFEB6_L3Q2,SFEB6_L3Q3,PFEB_L4Q1,PFEB_L4Q2,PFEB_L4Q3,SFEB8_L4Q1,SFEB6_L4Q2,SFEB6_L4Q3"
#FEBs="SFEB8_L1Q1"

#FEBs="PFEB_L4Q3"
#FEBs="SFEB_L1Q1_HOL"
#FEBs="SFEB_L1Q1_IPL,SFEB_L1Q2_IPL,SFEB_L1Q3_IPL,PFEB_L1Q1_IPR,PFEB_L1Q2_IPR,PFEB_L1Q3_IPR"
#FEBs="SFEB_L2Q1_IPR,SFEB_L2Q2_IPR,SFEB_L2Q3_IPR,PFEB_L2Q1_IPL,PFEB_L2Q2_IPL,PFEB_L2Q3_IPL"

PATTERN_baseline="?FEB*VMM*"
PATTERN_threshold="?FEB*vmm*"

#
# How to choose only a single board:
# FEBs="MMFE8_ML1P4_IPR"
#

FILES="/afs/cern.ch/user/x/xuwa/public/"
#FILES="/afs/cern.ch/user/n/nswdaq/public/config-files-sTGC/191_A12"
#CONFIG="${FILES}/flx2L2Q3.json"
#CONFIG="/afs/cern.ch/user/n/nswdaq/public/config-files-sTGC/wedge1B191layer1234_L2.json"
#CONFIG="${FILES}/wedge3gain3noise/baseline+30.json"
#CONFIG="${FILES}/baseline_A12_HO.json"
#CONFIG="/afs/cern.ch/user/n/nswdaq/public/wedge3flx2layer1234_final.json"
CONFIG="/afs/cern.ch/user/s/stgcic/public/configFiles_WedgeTestsB180/Wedge5_A10_S_P/wedge5layer1234_copy.json"
#CONFIG="/afs/cern.ch/user/s/stgcic/public/configFiles_WedgeTestsB180/Wedge6_A10_S_C/wedge6layer1234.json"

#CONFIG="/afs/cern.ch/work/n/nswdaq/public/sTGC_quick_and_dirty_baselines/test_wedge4.json"
#CONFIG="/afs/cern.ch/work/n/nswdaq/public/sTGC_quick_and_dirty_baselines/test.json"
#CONFIG="/afs/cern.ch/user/n/nswdaq/public/trimmers_x30_2020_02_14_13h51m38s_test.json"
NSAMPS="100"

# NOW="2019_08_07_12h41m24s"
# RMS="12,10,8,6,4"
RMS="30"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)_DebugNoBaseline
OUT=trimmers_sTGC/$NOW
THREADS=32

WEB=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/trimmers/${NOW}
LOG=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/trimmers/index.html
ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw-stgc/baseline_on_bench/root2html.py"

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
#$BIN/configure_frontend -c $CONFIG -v -r -t -M 31 -m $THREADS #monitor channel 31 to MO output
#$BIN/configure_frontend -c $CONFIG -v -r -t #-m $THREADS 
#/afs/cern.ch/work/r/rowang/public/FELIX/NSWConfiguration_workspace/x86_64-centos7-gcc8-opt/NSWConfiguration/configure_frontend -c $CONFIG -v -r -t
#/fs/cern.ch/work/r/rowang/public/FELIX/NSWConfiguration_workspace/installed/x86_64-centos7-gcc8-opt/bin/configure_frontend -c $CONFIG -v -r -t #-m $THREADS 

# calibrate thresholds
time python NSWConfiguration/app/threshold_parallel.py -rms $RMS -c $CONFIG -s $CALIBR -f $FEBs  -m $THREADS

