
import os, sys

sys.path.append("/afs/cern.ch/user/s/stgcic/public/benchtest/tools")
import felix_control
import felix_config


#FELIX_COMMAND = "felixcore -t 1 --data-interface=eno1 --elinks=0-2000 -d 1 "
FELIX_COMMAND = "openFELIX_wedge"
# FELIX_COMMAND = "openFELIX_bt"
#OPC_PATH = "~/public/Opcxml/1sFEB_felix2.xml"
#OPC_PATH = "/afs/cern.ch/user/s/stgcic/public/configFiles_WedgeTestsB180/"
OPC_PATH = "/afs/cern.ch/user/s/stgcic/public/Opcxml/flx1wedge_SFEB8_6.xml"

def auto_test_template():
    # Open felix
    felix_control.kill_process("all")
    felix_control.open_function(keys=FELIX_COMMAND)
    felix_control.open_function(path=OPC_PATH)

    # Config FEB
    #json_file = "/afs/cern.ch/user/s/stgcic/public/benchtest/felix2/3pFEB/baseline.json"
   # json_file = "/afs/cern.ch/user/s/stgcic/public/configFiles_WedgeTestsB180/Wedge5_A10_S_P/wedge5layer1234_SFEB8_6.json"
    #json_file = "/afs/cern.ch/work/s/stgcic/public/sTGC_quick_and_dirty_baselines/trimmers_sTGC/2020_03_12_14h14m04s_DebugNoBaseline/trimmers_x30.json"
    json_file = "/afs/cern.ch/user/s/stgcic/public/configFiles_WedgeTestsB180/Wedge7_20MNIWSAP00008_A10_S_P/wedge7layer1234_SFEB8_6.json"
    config_cmd = "json:{}".format(json_file)
    config_fail=felix_config.config(config_cmd,felix=1)


    



if __name__ == "__main__":
    auto_test_template()






