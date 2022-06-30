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

int active_threads(std::vector< std::future<int> >& threads){
  int nfinished = 0;
  for (auto& thread: threads)
    if (thread.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
      nfinished++;
  return static_cast<int>(threads.size()) - nfinished;
}

int configure_board(nsw::L1DDCConfig l1ddc) {

    std::cout << "New thread in configure_board (l1ddc) \n";

    nsw::ConfigSender cs;

    std::cout << "Starting cs.sendL1DDCConfig\n";
    cs.sendL1DDCConfig(l1ddc);
    std::cout << fmt::format("Done with configure_board for {}\n",l1ddc.getName());

    return 0;

}

int main (int argc, char** argv){

    // required settings
    string opcServerIp="none";
    string opcNodeId="none";
    string iPath="none";
    string boardType="none";
    string name="";
    uint64_t fid_toflx=-1;
    uint64_t fid_tohost=-1;
    bool parallel=false;
    bool no_rim=false;
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
        if (!strcmp(argv[i],"-p")) parallel=true;
        if (!strcmp(argv[i],"--norim")) no_rim=true;
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
        cout<<"-w       | Write configuration\n";
        cout<<"-t       | Write configuration and train GBTx e-links (applies to *.xml bitmap input file, not to JSON input file)\n";
        cout<<"-s       | Simulation mode: load configuration file to check for errors\n";
        cout<<"-p       | Run configuration in parallel. Default is to run in serial.\n";
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
        cout<<"         |      --norim  | Optional: Ignore RIM L1DDCs\n";
        cout<<endl;
        return 0;
    }

    cout<<"##################################################\n";
    cout<<"# Mode:            "<<mode.c_str()<<endl;
    cout<<"# fid_toflx:       "<<fid_toflx<<endl;
    cout<<"# fid_tohost:      "<<fid_tohost<<endl;
    cout<<"# config file:     "<<iPath<<endl;
    cout<<"# board type:      "<<boardType<<endl;
    cout<<"#------------------------------------------------#\n";
    cout<<"# sleep:           "<<trainGBTxPhaseWaitTime<<endl;
    cout<<"# name:            "<<name<<endl;
    cout<<"# run multithread: "<<parallel<<endl;
    cout<<"# Ignore RIM:      "<<no_rim<<endl;
    cout<<"##################################################\n";
    std::vector<nsw::L1DDCConfig> l1ddc_configs;
    std::vector<nsw::L1DDCConfig> rimL1ddc_configs;

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

        std::vector<const char*> boards = {"L1DDC"};
        if (!no_rim) boards.push_back("RimL1DDC");
        for (const auto& board_type: boards) {
            std::cout << "Adding board: " << board_type << std::endl;
            auto cfg = nsw::ConfigReader::makeObjects<nsw::L1DDCConfig>("json://" + iPath, board_type, name);
            std::move(std::begin(cfg), std::end(cfg), std::back_inserter(l1ddc_configs));
        }

        for (const auto& l1ddc: l1ddc_configs) {
            std::cout << "CONFIGURING: " << l1ddc.getName() << std::endl;
        }

    }
    else{
        std::cout<<"ERROR: Configuration file should be either .xml or .json\n";
        exit(0);
    }

    if (mode=="simulation"){
        std::cout<<"Running in simulation mode. This is the point where the configuration would be sent.\n";
        return 0;
    }

    // Continue with configuration
    if (parallel){
        std::vector< std::future<int> > threads;
        const int max_threads = 14;
        std::cout << "Thread info: starting threads\n";

        // Start threads for all l1ddc's
        for (const auto& l1ddc: l1ddc_configs) {
            std::cout << "Thread info: configuring l1ddc\n";
            // wait for threads to free before launching new thread
            if (max_threads > 0) {
                int n_active = active_threads(threads);
                while(n_active >= max_threads) {
                    std::cout << "Thread info: waiting to configure\n";
                    std::cout << "Too many active threads (" << n_active << "), waiting for fewer than " << max_threads << std::endl;
                    sleep(2);
                    n_active = active_threads(threads);
              }
          }

          // start thread for this L1DDC
          threads.push_back( std::async(std::launch::async, configure_board, l1ddc) );
        }

        // Wait for all threads to finish
        for (auto& thread: threads) {
            try {
                std::cout << "Thread info: getting thread\n";
                thread.get();
            } catch (ers::Issue & ex) {
                ERS_LOG("Configuration failed in app due to ers::Issue: " << ex.what());
            } catch (std::exception & ex) {
                ERS_LOG("Configuration failed in app due to std::exception: " << ex.what());
            }
        }
        std::cout << "Done with configure_gbtx\n";
    }

    // #############################################################################
    else{
        nsw::ConfigSender cs;
        for (std::size_t i=0; i<l1ddc_configs.size(); i++){
            std::cout << fmt::format("Starting L1DDC configuration {}/{}\n",i+1,l1ddc_configs.size());
            cs.sendL1DDCConfig(l1ddc_configs.at(i));
            std::cout << fmt::format("Done with configure_board for {}\n",l1ddc_configs.at(i).getName());
        }
        std::cout << "Done with configure_gbtx\n";
    }

    return 0;
}

