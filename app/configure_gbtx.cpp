// Sample program to read configuration from json and send to any front end module

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <thread>
#include <future>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/L1DDCConfig.h"
#include "ic-over-netio/IChandler.h"

using namespace std;


int main (int argc, char** argv){

    // required settings
    string felixServerIp="none";
    string opcServerIp="none";
    string opcNodeId="none";
    string iPath="none";
    string boardType="none";
    int portToGBTx=-1;
    int portFromGBTx=-1;
    int elinkId=-1;
    int trainGBTxPhaseWaitTime=1;

    // Check if reading or writing or training
    string mode="none";
    for (int i = 0; i < argc; i++){
        if      (!strcmp(argv[i],"-t")) mode="train";
        else if (!strcmp(argv[i],"-w")) mode="write";
        else if (!strcmp(argv[i],"-h")) mode="help";
        else if (!strcmp(argv[i],"-s")) mode="simulation";
    }
    // get required settings

    for (int i = 0; i < argc; i++){
        if (!strcmp(argv[i],"--opc")) felixServerIp=argv[i+1];
        if (!strcmp(argv[i],"--node")) opcNodeId=argv[i+1];
        if (!strcmp(argv[i],"--felix")) opcServerIp=argv[i+1];
        if (!strcmp(argv[i],"--iport")) portToGBTx=atoi(argv[i+1]);
        if (!strcmp(argv[i],"--oport")) portFromGBTx=atoi(argv[i+1]);
        if (!strcmp(argv[i],"--elink")) elinkId=atoi(argv[i+1]);
        if (!strcmp(argv[i],"--sleep")) trainGBTxPhaseWaitTime=atoi(argv[i+1]);
        if (!strcmp(argv[i],"--board")) boardType=argv[i+1];
        if (!strcmp(argv[i],"--config")) iPath=argv[i+1];
    }

    // check inputs
    if (mode=="none"){
        cout<<"Please run with -w,-t,-s, or -h for help\n";
        cout<<"Example: --felix pcatlnswfelix10.cern.ch --iport 12340 --oport 12350 --elink 62 --board mmg \n";
        return 0;
    }

    if (mode=="help"){
        cout<<"NAME\n   configure_gbtx - write to and read from GBTx through netio\n";
        cout<<"SYNOPSIS\n   configure_gbtx [-w,-t,-s,-h]... --iport WWW --oport XXX --felix YYY --opc QQQ --elink ZZZ --board mmg --config CONFIG \n";
        cout<<"-t       | Write configuration and train GBTx e-links\n";
        cout<<"-w       | Just write configuration\n";
        cout<<"-s       | Simulation mode: load configuration file to check for errors\n";
        cout<<"--config | Required configuration file. Either *.json of the standard format\n";
        cout<<"         | 1) Either: *.json of the standard format\n";
        cout<<"         | 2) Or:     *.xml, consisting of line separated hex bytes (GBTx1 only)\n";
        cout<<"         |    If using the 'xml' format, the following options are required:\n";
        cout<<"         |      --board  | Required board type: mmg, stgc, or rim_stgc\n";
        cout<<"         |      --iport  | Input port for GBTx. For example, 12340\n";
        cout<<"         |      --oport  | Output port for GBTx. For example, 12350\n";
        cout<<"         |      --felix  | Address/name for FELIX machine. For example, pcatlnswfelix10.cern.ch\n";
        cout<<"         |      --opc    | Address/name for machine running OPC. For example, pcatlnswfelix10.cern.ch\n";
        cout<<"         |      --node   | OPC node ID for GBTx2\n";
        cout<<"         |      --elink  | FELIX e-link number in decimal for GBTx. For example 62 if you want to configure link 0x3E\n";
        cout<<"         |      --sleep  | Optional: number of seconds to wait for training\n";
        cout<<"\nExample uses:\n";
        cout<<"configure_gbtx --felix pcatlnswfelix10.cern.ch --iport 12340 --oport 12350 --elink 62 -c config.txt\n";
        cout<<"configure_gbtx -t --felix pcatlnswfelix10.cern.ch --iport 12340 --oport 12350 --elink 62 -c config.txt\n";
        cout<<endl;
        return 0;
    }


    cout<<"##################################################\n";
    cout<<"# Mode:          "<<mode.c_str()<<endl;
    cout<<"# felixServerIp: "<<felixServerIp<<endl;
    cout<<"# portToGBTx:    "<<portToGBTx<<endl;
    cout<<"# portFromGBTx:  "<<portFromGBTx<<endl;
    cout<<"# elinkId:       "<<elinkId<<endl;
    cout<<"# config file:   "<<iPath<<endl;
    cout<<"# board type:    "<<boardType<<endl;
    cout<<"#------------------------------------------------#\n";
    cout<<"# sleep:         "<<trainGBTxPhaseWaitTime<<endl;
    cout<<"##################################################\n";
    std::vector<nsw::L1DDCConfig> l1ddc_configs;

    // Configure with xml file
    if (iPath.find(".xml")!=string::npos){
        if (felixServerIp=="none"||portToGBTx==-1||portFromGBTx==-1||elinkId==-1||boardType=="none"||opcServerIp=="none"||opcNodeId=="none"){
            cout<<"Please set required inputs.\nExample: --felix pcatlnswfelix10.cern.ch --opc pcatlnswfelix10.cern.ch --node XXX --iport 12340 --oport 12350 --elink 62 --board mmg --config CONFIG.json\n";
            return 0;
        }

        nsw::GBTxSingleConfig config {
            .iPath = iPath,
            .opcServerIp = opcServerIp,
            .opcNodeId = opcNodeId,
            .felixServerIp = felixServerIp,
            .portToGBTx = portToGBTx,
            .portFromGBTx = portFromGBTx,
            .elinkId = elinkId,
            .boardType = boardType,
            .trainGBTxPhaseAlignment = (mode=="train"),
            .trainGBTxPhaseWaitTime = trainGBTxPhaseWaitTime
        };

        l1ddc_configs.emplace_back(config);

    }
    // Read configuration from json
    else if (iPath.find(".json")!=string::npos){
        // Check that unneeded and confusing options not provided:
        if (felixServerIp!="none"||portToGBTx!=-1||portFromGBTx!=-1||elinkId!=-1||boardType!="none"){
            cout<<"Note that the options for felix, opc, node, iport, oport, elink, and board should be provided by the json and not the command line.\nPlease remove them and re-run.\n";
            return 0;
        }
        if (mode=="train"){
            cout<<"--------------------------------------------------\n";
            cout<<"Warning: the trainGBTxPhaseAlignment setting in\n";
            cout<<"the JSON file will take precident over the -t  \n";
            cout<<"--------------------------------------------------\n";
        }
        nsw::ConfigReader reader("json://" + iPath);
        try {
            reader.readConfig();
        } catch (std::exception & e) {
            std::cout<<"ERROR: Can't read config file due to : "<<e.what()<<std::endl;
            exit(0);
        }
        // pick names for configuration
        for (auto & name : reader.getAllElementNames()) {
             std::cout<<"READING: "<<name<<std::endl;
             if (name.find("L1DDC")==std::string::npos) continue;
             std::cout<<"CONFIGURING: "<<name<<std::endl;
             boost::property_tree::ptree config = reader.readConfig(name);
             l1ddc_configs.emplace_back(config);
        }
    }
    else{
        std::cout<<"ERROR: Configuration file should be either .xml or .json\n";
        exit(0);
    }

    if (mode!="simulation"){
        // send configuration
        nsw::ConfigSender cs;
        for (std::size_t i=0; i<l1ddc_configs.size(); i++){
            std::cout << "Starting cs.sendL1DDCConfig\n";
            cs.sendL1DDCConfig(l1ddc_configs[i]);
            std::cout << "Done with configure_board\n";

            // TODO: this will need to be implemented:
            // std::vector<uint8_t> data = l1ddc_configs[i].getGbtxBytestream(0);
            // std::cout<<nsw::getPrintableGbtxConfig(data)<<std::endl;

        }
    }
    else{
        std::cout<<"Running in simulation mode. This is the point where the configuration would be sent.\n";
        return 0;
    }
}

