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
    }
    // get required settings

    for (int i = 0; i < argc; i++){
        if (!strcmp(argv[i],"--ip")) felixServerIp=argv[i+1];
        if (!strcmp(argv[i],"--iport")) portToGBTx=atoi(argv[i+1]);
        if (!strcmp(argv[i],"--oport")) portFromGBTx=atoi(argv[i+1]);
        if (!strcmp(argv[i],"--elink")) elinkId=atoi(argv[i+1]);
        if (!strcmp(argv[i],"--sleep")) trainGBTxPhaseWaitTime=atoi(argv[i+1]);
        if (!strcmp(argv[i],"-t")) boardType=argv[i+1];
        if (!strcmp(argv[i],"-c")) iPath=argv[i+1];
    }

    // check inputs
    if (mode=="none"){
        cout<<"Please run with -w,-t, or -h for help\n";
        cout<<"Example: --ip pcatlnswfelix10.cern.ch --iport 12340 --oport 12350 --elink 62\n";
        return 0;
    }

    if (mode=="help"){
        cout<<"NAME\n   configure_gbtx - write to and read from GBTx through netio\n";
        cout<<"SYNOPSIS\n   configure_gbtx [-w,-t,-h]... --iport WWW --oport XXX --ip YYY --elink ZZZ -c CONFIG \n";
        cout<<"-t      | Write configuration and train GBTx e-links\n";
        cout<<"-w      | Just write configuration\n";
        cout<<"-c      | Required configuration file consisting of line separated hex bytes\n";
        cout<<"--iport | Input port for GBTx. For example, 12340\n";
        cout<<"--oport | Output port for GBTx. For example, 12350\n";
        cout<<"--ip    | Address/name for FELIX machine running ip. For example, pcatlnswfelix10.cern.ch\n";
        cout<<"--elink | FELIX e-link number in decimal for GBTx. For example 62 if you want to configure link 0x3E\n";
        cout<<"--sleep | Optional: number of seconds to wait for training\n";
        cout<<"\nExample uses:\n";
        cout<<"configure_gbtx --ip pcatlnswfelix10.cern.ch --iport 12340 --oport 12350 --elink 62 -c config.txt\n";
        cout<<"configure_gbtx -t --ip pcatlnswfelix10.cern.ch --iport 12340 --oport 12350 --elink 62 -c config.txt\n";
        cout<<endl;
        return 0;
    }

    if (felixServerIp=="none"||portToGBTx==-1||portFromGBTx==-1||elinkId==-1){
        cout<<"Please set required inputs.\nExample: --ip pcatlnswfelix10.cern.ch --iport 12340 --oport 12350 --elink 62\n";
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


        nsw::GBTxSingleConfig config {
            .iPath = iPath,
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
             boost::property_tree::ptree config = reader.readConfig(name);
             l1ddc_configs.emplace_back(config);
        }
    }
    else{
        std::cout<<"ERROR: Configuration file should be either .xml or .json\n";
        exit(0);
    }

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

