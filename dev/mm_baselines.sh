MMFE8S=""
MMFE8S="${MMFE8S},ML1P4_IPR"
MMFE8S="${MMFE8S},ML2P4_IPL"
MMFE8S="${MMFE8S},ML3P4_IPR"
MMFE8S="${MMFE8S},ML4P4_IPL"
MMFE8S="${MMFE8S},ML1P4_HOL"
MMFE8S="${MMFE8S},ML2P4_HOR"
MMFE8S="${MMFE8S},ML3P4_HOL"
MMFE8S="${MMFE8S},ML4P4_HOR"
CONFIG="/eos/atlas/atlascerngroupdisk/det-nsw/bb5/configs_from_poly/mmfe8_bb5_CosmicSlice.json"
CFGFEB="./x86_64-centos7-gcc8-opt/NSWConfiguration/configure_frontend"
SCRIPT="./x86_64-centos7-gcc8-opt/NSWConfiguration/read_channel_thresholds"
NSAMPS="100"

# commands
$CFGFEB -c $CONFIG -v -r
python NSWConfiguration/dev/mm_baselines.py -b -x $SCRIPT -c $CONFIG -s $NSAMPS -f $MMFE8S
grep -h "^DATA" baselines_ML* > baselines.txt
python NSWConfiguration/dev/mm_baselines.plot.py -i baselines.txt -f $MMFE8S

# copy to the web
NOW=$(date +%Y-%m-%d-%Hh%Mm%Ss)
mkdir ~/www/baselines_${NOW}/
cp baseline_*.pdf ~/www/baselines_${NOW}/
echo "www.cern.ch/tuna/baselines_${NOW}/"
