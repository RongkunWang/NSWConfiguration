
#
# How to choose only a single board
FEBs="PFEB_L1Q1_IPR"
#

# NOW="2019_07_16_11h99m99s"
NOW=$(date +%Y_%m_%d_%Hh%Mm%Ss)
OUT=baselines191/$NOW
# careful: side and sector are hard-coded
#WEB=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/b191/A14/baselines/${NOW}
#LOG=/eos/atlas/atlascerngroupdisk/det-nsw-stgc/b191/A14/baselines/index.html
#ROOT2HTML="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/cosmics/html/root2html.py"

CONFIG="/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/191/A14/STGC_191_A14_HOIP_BASELINE.json"
BIN="./x86_64-centos7-gcc8-opt/NSWConfiguration"
NSAMPS="100"


# commands
$BIN/configure_frontend   -c $CONFIG -v -r -n $FEBs
$BIN/read_channel_monitor -c $CONFIG -s $NSAMPS -o $OUT --baseline --dump -n $FEBs --threads 64
#$BIN/read_channel_monitor -c $CONFIG -s $NSAMPS -o $OUT --baseline --dump --threads 32 # remove me and uncomment the line above to choose MMFE8s
grep -h "^DATA" $OUT/baselines_* > $OUT/baselines.txt
python NSWConfiguration/dev/mm_baselines.plot.py -i $OUT/baselines.txt -o $OUT/baselines_${NOW}.root -y 5 | tee $OUT/rms.txt


# copy to the web
#${ROOT2HTML} $OUT/baselines_${NOW}.root
#echo "Moving to the webspace..."
#mv $OUT/baselines_${NOW}  ${WEB}/
#chmod -R g+wxr ${WEB}
#cp $OUT/baselines_*.root          ${WEB}/
#cp $OUT/baselines.txt             ${WEB}/
#cp $OUT/rms.txt                   ${WEB}/
#echo "https://nsw191.web.cern.ch/nsw191/"

