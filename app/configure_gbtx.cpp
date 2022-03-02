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

using namespace std;


int main (int argc, char** argv){

    // required settings
    string opcServerIp="none";
    string opcNodeId="none";
    string iPath="none";
    string boardType="none";
    string name="";
    uint64_t fid_toflx=-1;
    uint64_t fid_tohost=-1;
    std::size_t trainGBTxPhaseWaitTime=1;

    // Check if reading or writing or training
    string mode="none";
    for (std::size_t i = 0; i < argc; i++){
        if      (!strcmp(argv[i],"-t")) mode="train";
        else if (!strcmp(argv[i],"-w")) mode="write";
        else if (!strcmp(argv[i],"-h")) mode="help";
        else if (!strcmp(argv[i],"-s")) mode="simulation";
    }
    // get required settings

    for (std::size_t i = 0; i < argc; i++){
        if (!strcmp(argv[i],"--node")) opcNodeId=argv[i+1];
        if (!strcmp(argv[i],"--opc")) opcServerIp=argv[i+1];
        if (!strcmp(argv[i],"--toflx")) fid_toflx=stoull(argv[i+1],nullptr,0);
        if (!strcmp(argv[i],"--tohost")) fid_tohost=stoull(argv[i+1],nullptr,0);
        if (!strcmp(argv[i],"--sleep")) trainGBTxPhaseWaitTime=atoi(argv[i+1]);
        if (!strcmp(argv[i],"--board")) boardType=argv[i+1];
        if (!strcmp(argv[i],"--config")) iPath=argv[i+1];
        if (!strcmp(argv[i],"--name")) name=argv[i+1];
    }

    // check inputs
    if (mode=="none"){
        cout<<"Please run with -w,-t,-s, or -h for help\n";
        cout<<"Example: configure_gbtx -w  --toflx WWW --tohost XXX --felix YYY --opc QQQ --board mmg --config CONFIG \n";
        return 0;
    }

    if (mode=="help"){
        cout<<"NAME\n   configure_gbtx - write to and read from GBTx through netio\n";
        cout<<"SYNOPSIS\n   configure_gbtx [-w,-t,-s,-h]... --toflx WWW --tohost XXX --felix YYY --opc QQQ --board mmg --config CONFIG \n";
        cout<<"-t       | Write configuration and train GBTx e-links\n";
        cout<<"-w       | Just write configuration\n";
        cout<<"-s       | Simulation mode: load configuration file to check for errors\n";
        cout<<"--config | Required configuration file. Either *.json of the standard format\n";
        cout<<"         | 1) Either: *.json of the standard format\n";
        cout<<"         | 2) Or:     *.xml, consisting of line separated hex bytes (GBTx1 only)\n";
        cout<<"         |    If using the 'xml' format, the following options are required:\n";
        cout<<"         |      --board  | Required board type: mmg, stgc, or rim_stgc\n";
        cout<<"         |      --toflx  | FID, used to send data to FELIX, for example 0x16b00000007e0000\n";
        cout<<"         |      --tohost | FID, used to receive data from FELIX, for example 0x16b00000007e8000\n";
        cout<<"         |      --felix  | IP address or network for FELIX communication. For example, vlan413\n";
        cout<<"         |      --opc    | Address/name for machine running OPC. For example, pcatlnswfelix10.cern.ch\n";
        cout<<"         |      --node   | OPC node ID for GBTx2\n";
        cout<<"         |      --sleep  | Optional: number of seconds to wait for training\n";
        cout<<"         |      --name   | Optional: comma-separated L1DDC names to consider (for json, not xml)\n";
        cout<<endl;
        return 0;
    }


    cout<<"##################################################\n";
    cout<<"# Mode:          "<<mode.c_str()<<endl;
    cout<<"# fid_toflx:    "<<fid_toflx<<endl;
    cout<<"# fid_tohost:  "<<fid_tohost<<endl;
    cout<<"# config file:   "<<iPath<<endl;
    cout<<"# board type:    "<<boardType<<endl;
    cout<<"#------------------------------------------------#\n";
    cout<<"# sleep:         "<<trainGBTxPhaseWaitTime<<endl;
    cout<<"# name:          "<<name<<endl;
    cout<<"##################################################\n";
    std::vector<nsw::L1DDCConfig> l1ddc_configs;

    // Configure with xml file
    if (iPath.find(".xml")!=string::npos){
        if (fid_toflx==-1||fid_tohost==-1||boardType=="none"||opcServerIp=="none"||opcNodeId=="none"){
            cout<<"Please set required inputs.\nExample: --felix vlan413 --opc pcatlnswfelix10.cern.ch --node XXX --toflx 0x16b00000007e0000 --tohost 0x16b00000007e8000 --board mmg --config CONFIG.json\n";
            return 0;
        }

        nsw::GBTxSingleConfig config {
            .iPath = iPath,
            .opcServerIp = opcServerIp,
            .opcNodeId = opcNodeId,
            .fid_toflx = fid_toflx,
            .fid_tohost = fid_tohost,
            .boardType = boardType,
            .trainGBTxPhaseAlignment = (mode=="train"),
            .trainGBTxPhaseWaitTime = trainGBTxPhaseWaitTime
        };

        l1ddc_configs.emplace_back(config);

    }
    // Read configuration from json
    else if (iPath.find(".json")!=string::npos){
        // Check that unneeded and confusing options not provided:
        if (fid_toflx!=-1||fid_tohost!=-1||boardType!="none"){
            cout<<"Note that the options for felix, opc, network or IP, toflx, tohost, and board should be provided by the json and not the command line.\nPlease remove them and re-run.\n";
            return 0;
        }
        if (mode=="train"){
            cout<<"--------------------------------------------------\n";
            cout<<"Warning: the trainGBTxPhaseAlignment setting in\n";
            cout<<"the JSON file will take precident over the -t  \n";
            cout<<"--------------------------------------------------\n";
        }
        l1ddc_configs = nsw::ConfigReader::makeObjects<nsw::L1DDCConfig>("json://" + iPath, "L1DDC", name);
        for (const auto& l1ddc: l1ddc_configs) {
            std::cout << "CONFIGURING: " << l1ddc.getName() << std::endl;
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

