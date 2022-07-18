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

#include <boost/program_options.hpp>

using namespace std;
namespace po = boost::program_options;

struct Args {
    string mode="none";
    string iPath="none";
    string boardType="none";
    string name="";
    uint64_t toflx=0;
    uint64_t tohost=0;
    bool parallel=false;
    bool isJson=true;
    bool no_rim=false;
    std::size_t sleep=1;
};


Args parseCommandLineArgs(const int argc, char* argv[]) {
    Args args{};

    // If onle json file provided, then assume to configure with this
    if (argc==2){
        std::string iPath(argv[1]);
        if (iPath.find(".json")!=string::npos)
            {
                cout<<"Assuming json\n";
                args.iPath=iPath;
                args.mode="write";
                args.parallel=true;
                return args;
            }

    }

    // Parse arguements in detail
    bool fwrite      = false;
    bool ftrain      = false;
    bool fread       = false;
    bool fsimulation = false;
    bool fnorim      = false;
    bool fparallel   = false;

    po::options_description desc("Allowed options");
    desc.add_options()
      // Modes
      ("help,h", "Print this help message")
      // Modes
      ("write,w",      po::bool_switch(&fwrite),                "MODE (one allowed): Write configuration")
      ("read,r",       po::bool_switch(&fread),                 "MODE (one allowed): Read configuration")
      ("train,t",      po::bool_switch(&ftrain),                "MODE (one allowed): Write configuration and train GBTx e-links")
      ("simulation,s", po::bool_switch(&fsimulation),           "MODE (one allowed): Simulation mode: load configuration file to check for errors")
      // Config file
      ("config,c",     po::value<std::string>()->required(),    "REQUIRED configuration file. \nEither a JSON file (IC and I2C GBTx), or a line separated bitmap (IC GBTx only)")
      // Bitmap options
      ("board",        po::value<std::string>(),                "REQUIRED for bitmap: Board type: mmg, stgc, or rim_stgc")
      ("toflx",        po::value<std::string>(),                "REQUIRED for bitmap: FID e.g. 0x16b00000007e8000")
      ("tohost",       po::value<std::string>(),                "REQUIRED for bitmap: FID e.g. 0x16b00000007e0000")
      ("sleep",        po::value<int>(),                        "OPTIONAL for bitmap: Seconds to wait for training")
      // JSON options
      ("norim",        po::bool_switch(&fnorim),                "OPTIONAL for JSON: Ignore RIM L1DDCs")
      ("parallel,p",   po::bool_switch(&fparallel),             "OPTIONAL for JSON: Run configuration in parallel")
      ("name",         po::value<std::string>(),                "OPTIONAL for JSON: Comma-separated L1DDC names to configure")
      ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << "\n";
        std::exit(0);
    }

    // Check that arguements are sensible
    args.iPath = vm["config"].as<std::string>();
    if (args.iPath.find("json")!=string::npos){
        cout<<"Config json\n";
        // check that only json arguements are provided
        if (vm.count("board") || vm.count("toflx") || vm.count("tohost") || vm.count("sleep") ){
            cout<<"You provided bitmap style options for a JSON configuration file. Exiting \n";
            std::exit(0);
        }
        args.no_rim   = vm["norim"].as<bool>();
        args.parallel = vm["parallel"].as<bool>();
        if (vm.count("name")) args.name = vm["name"].as<std::string>();

    }
    else{
        // check that only bitmap arguements are provided
        cout<<"Config bitmap\n";
        if (fnorim || vm.count("name") || fparallel){
            cout<<"You provided JSON style options for a bitmap configuration file. Exiting \n";
            std::exit(0);
        }
        if (!vm.count("board") || !vm.count("toflx") || !vm.count("tohost")){
            cout<<"You are missing a required argument for a bitmap configuration file. Exiting \n";
            std::exit(0);
        }
        if (vm.count("sleep")) args.sleep  = vm["sleep"].as<int>();
        args.isJson = false;
        args.toflx  = stoull(vm["toflx"].as<std::string>(),nullptr,0);
        args.tohost = stoull(vm["tohost"].as<std::string>(),nullptr,0);
        args.boardType  = vm["board"].as<std::string>();
    }

    // check that the modes are sensible
    if (fwrite + fread + ftrain + fsimulation != 1){
        cout<<"You provided != 1 mode options. Pick one of: write, read, train, simulation. Exiting \n";
        std::exit(0);
    }

    // not a nice way to do this, but I don't want to change the options from flags
    if      (fwrite) args.mode = "write";
    else if (fread) args.mode = "read";
    else if (fsimulation) args.mode = "simulation";
    else if (ftrain) args.mode = "train";
    else {
        cout<<"Invalid option choice. Exiting\n";
        std::exit(0);
    }

    // Print out summary of parameters
    cout<<"##################################################\n";
    cout<<"# Mode:            "<<args.mode<<endl;
    cout<<"# config file:     "<<args.iPath<<endl;
    cout<<"# Ignore RIM:      "<<args.no_rim<<endl;
    cout<<"# run multithread: "<<args.parallel<<endl;
    cout<<"#------------------------------ Bitmap options --#\n";
    cout<<"# board type:      "<<args.boardType<<endl;
    cout<<"# fid_tohost:      0x"<<std::hex<<args.tohost<<std::dec<<endl;
    cout<<"# fid_toflx:       0x"<<std::hex<<args.toflx<<std::dec<<endl;
    cout<<"# name:            "<<args.name<<endl;
    cout<<"# sleep:           "<<args.sleep<<endl;
    cout<<"##################################################\n";


    return args;

}

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

    auto args = parseCommandLineArgs(argc,argv);

    std::vector<nsw::L1DDCConfig> l1ddc_configs;
    std::vector<nsw::L1DDCConfig> rimL1ddc_configs;

    // Configure with xml file
    if (!args.isJson){

        nsw::GBTxSingleConfig config {
            .iPath = args.iPath,
            .opcServerIp = "",
            .opcNodeId = "",
            .fid_toflx = args.toflx,
            .fid_tohost = args.tohost,
            .boardType = args.boardType,
            .trainGBTxPhaseAlignment = (args.mode=="train"),
            .trainGBTxPhaseWaitTime = args.sleep
        };

        l1ddc_configs.emplace_back(config);

    }
    // Read configuration from json
    else{
        // Check that unneeded and confusing options not provided:
        std::vector<const char*> boards = {"L1DDC"};
        if (!args.no_rim) boards.push_back("RimL1DDC");
        for (const auto& rimOrL1ddc: boards) {
            std::cout << "Adding board: " << rimOrL1ddc << std::endl;
            auto cfg = nsw::ConfigReader::makeObjects<nsw::L1DDCConfig>("json://" + args.iPath, rimOrL1ddc, args.name);
            std::move(std::begin(cfg), std::end(cfg), std::back_inserter(l1ddc_configs));
        }

        for (const auto& l1ddc: l1ddc_configs) {
            std::cout << "CONFIGURING: " << l1ddc.getName() << std::endl;
        }

    }

    // Mode specific behavior/override
    if (args.mode=="simulation"){
        std::cout<<"Running in simulation mode. This is the point where the configuration would be sent.\n";
        return 0;
    }
    else if (args.mode=="read"){
        std::cout<<"Changing L1DDC configuration to only read back GBTx configuration. NOTE: this will only read the IC GBTx configuration. If you want to read the other GBTx's, update the JSON file.\n";
        for (auto& l1ddc: l1ddc_configs) {
            // enable readback
            l1ddc.setReadbackGBTx(0,true);
            // disable configuration
            for (std::size_t iGbtx=0; iGbtx<l1ddc.getNumberGBTx(); iGbtx++){
                l1ddc.setConfigureGBTx(iGbtx,false);
            }
        }
    }
    else if (args.mode=="train"){
        std::cout<<"Changing L1DDC configuration to train GBTx.\n";
        for (auto& l1ddc: l1ddc_configs) {
            for (std::size_t iGbtx=0; iGbtx<l1ddc.getNumberGBTx(); iGbtx++){
                l1ddc.setTrainGBTxPhaseAlignment(true);
            }
        }
    }

    // Continue with configuration
    if (args.parallel){
        std::vector< std::future<int> > threads;
        const int max_threads = 16;
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

