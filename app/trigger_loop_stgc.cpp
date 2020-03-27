//
// Program to test pulse triggerable patterns, sTGC edition :)
// 

#include <sstream>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <future>
#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/Utility.h"
#include "boost/program_options.hpp"
#include "boost/property_tree/ptree.hpp"
#include "ALTI/AltiModule.h"
#include "ALTI/AltiConfiguration.h"
#include "RCDVme/RCDCmemSegment.h"
#include <glob.h>

namespace po = boost::program_options;
namespace pt = boost::property_tree;

LVL1::AltiModule* prepare_alti(unsigned int slot, bool do_config, bool dry_run);
int fifo_status(LVL1::AltiModule* alti);
int oneshot(LVL1::AltiModule* alti);
int wait_until_done(std::vector< std::future<int> >* threads);
int configure_vmms(nsw::ConfigSender* cs, nsw::FEBConfig feb, pt::ptree febpatt, bool unmask, bool reset, bool dry_run);
std::vector<nsw::FEBConfig> parse_feb_name(std::string name, std::string cfg);
std::string rename(std::string name);
pt::ptree patterns();
std::string strf_time();
int minutes_remaining(double time_diff, int nprocessed, int ntotal);
std::atomic<bool> watch;

int main(int argc, const char *argv[]) {
    std::string config_filename;
    std::string board_name_pfeb;
    std::string fname = "trigger_loop_stgc_" + strf_time() + ".json";
    int max_threads;
    int pulsevmm;
    unsigned int alti_slot;
    bool dry_run;
    bool reset_vmm;
   
    // CL options
    po::options_description desc(std::string("Olga's configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,c", po::value<std::string>(&config_filename)->
         default_value("/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/config_json/VS/wedge1VS_A12_OlgaEdition.json"),
         "Configuration file path")
        ("dry_run", po::bool_switch()->
         default_value(false), "Option to NOT send configurations")
        ("reset_vmm", po::bool_switch()->
         default_value(false), "Option to reset VMM at every configuration")
        ("max_threads,m", po::value<int>(&max_threads)->
         default_value(-1), "Maximum number of threads to run")
        ("pulsevmm", po::value<int>(&pulsevmm)->
         default_value(-1), "Pulse a particular VMM only (good for pulsing tracks)")
        ("alti", po::value<unsigned int>(&alti_slot)->
         default_value(11), "ALTI slot")
        ("pfeb", po::value<std::string>(&board_name_pfeb)->
         default_value(""), "The name of frontend to configure (should start with PFEB_).")
         ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    dry_run    = vm["dry_run"   ].as<bool>();
    reset_vmm  = vm["reset_vmm" ].as<bool>();
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    // announce
    std::cout << "Dry run:          " << dry_run     << std::endl;
    std::cout << "VMM hard resets:  " << reset_vmm   << std::endl;
    std::cout << "ALTI slot:        " << alti_slot   << std::endl;
    std::cout << "Pulse VMM:        " << pulsevmm    << std::endl;
    std::cout << "Output json:      " << fname       << std::endl;

    // alti
    auto alti = prepare_alti(alti_slot, false, dry_run);
    if (!dry_run)
        fifo_status(alti);

    // the febs
    auto febs = parse_feb_name(board_name_pfeb, config_filename);
    std::cout << "Number of FEBs found: " << febs.size() << std::endl;

    // the configuration threads
    std::cout << "Creating vector of threads..." << std::endl;
    auto threads = new std::vector< std::future<int> >();

    // limit 1 sender per board
    std::cout << "Creating map of ConfigSender*..." << std::endl;
    auto senders = new std::map< std::string, nsw::ConfigSender* >();
    for (auto & feb : febs)
        senders->insert( {feb.getAddress(), new nsw::ConfigSender()} );   
    // keep time
    std::chrono::time_point<std::chrono::system_clock> time_start;
    std::chrono::duration<double> elapsed_seconds;
    time_start = std::chrono::system_clock::now();

    // loop over patterns
    int ipatt = 0;
    int ishot = 0;
    std::cout << "about to get patterns" << std::endl;
    auto patts = patterns();
    std::cout << "finished..." << std::endl;
    for (auto pattkv : patts) {

        std::cout << "> " << ipatt+1 << " / " << patts.size() << std::endl;

        // announce
        std::cout << " Pattern:" << std::endl;
        for (auto febkv : pattkv.second) {
	  auto febkv_rename =  rename (febkv.first);
	  std::cout << febkv_rename << std::endl;
      	  //std::cout << "  " << febkv.first;
            for (auto vmmkv : febkv.second) {
                std::cout << " " << vmmkv.first;
                for (auto chkv : vmmkv.second)
                    std::cout << "/" << chkv.second.get<unsigned>("");
            }
            std::cout << std::endl;
        }

        // enable test pulse
        bool anything = 0;
        std::cout << " Configure PFEBs (enable)..." << std::flush;
        for (auto febkv : pattkv.second) {
	  auto febkv_rename =  rename (febkv.first);
	  for (auto & feb : febs) {
	      std::cout << "feb.get      " << feb.getAddress() << std::endl;
	      std::cout << "febkv_rename " << febkv_rename << std::endl;
                if (febkv_rename != feb.getAddress())
                    continue;
                anything = 1;
                threads->push_back(std::async(std::launch::async, configure_vmms,
                                              senders->at(feb.getAddress()), feb,
                                              febkv.second, true, reset_vmm, dry_run));
                break;
            }
        }
        wait_until_done(threads);
        std::cout << " done. " << std::endl;
        if (!anything) {
            std::cout << " Skipping.";
            std::cout << std::flush << std::endl;
            continue;
        }

        // run the TTC commands
        std::cout << " Run TTC..." << std::flush;
         if (!dry_run)
            oneshot(alti);
         ishot++;
         std::cout << " done. " << std::endl;
       
        // disable test pulse
        std::cout << " Configure PFEBs (disable)..." << std::flush;
        for (auto febkv : pattkv.second) {
            for (auto & feb : febs) {
                if (febkv.first != feb.getAddress())
		 continue;
                anything = 1;
                threads->push_back(std::async(std::launch::async, configure_vmms,
                                              senders->at(feb.getAddress()), feb,
                                              febkv.second, false, reset_vmm, dry_run));
                break;
            }
        }
        wait_until_done(threads);
        std::cout << " done. " << std::endl;

        // wrap up
        ipatt++;
        elapsed_seconds = (std::chrono::system_clock::now() - time_start);
        std::cout << " ~" << minutes_remaining(elapsed_seconds.count(), ipatt, patts.size()) << " min left.";
        std::cout << std::flush;
        std::cout << std::endl;
    }
   
    // wrap up
    std::cout << std::endl;
    std::cout << "Closing " << fname << std::endl;
    if (!dry_run)
        fifo_status(alti);
    elapsed_seconds = (std::chrono::system_clock::now() - time_start);
    std::cout << "Elapsed time: ~" << std::lround(elapsed_seconds.count()/60.0) << " minutes" << std::endl;
    return 0;
}

int wait_until_done(std::vector< std::future<int> >* threads) {
    for (auto& thread : *threads)
        thread.get();
    threads->clear();
    return 0;
}

std::string strf_time() {
    std::stringstream ss;
    std::string out;
    std::time_t result = std::time(nullptr);
    std::tm tm = *std::localtime(&result);
    ss << std::put_time(&tm, "%Y_%m_%d_%Hh%Mm%Ss");
    ss >> out;
    return out;
}

int minutes_remaining(double time_diff, int nprocessed, int ntotal) {
    double rate = (double)(nprocessed+1)/time_diff;
    return std::lround((double)(ntotal-nprocessed)/(rate*60));
}

std::vector<std::string> glob_nice(std::string glob_path) {
  auto result = std::vector<std::string>();
  glob_t glob_result;
  glob(glob_path.c_str(), GLOB_TILDE, NULL, &glob_result);
  for(unsigned int i = 0; i < glob_result.gl_pathc; ++i)
    result.push_back( glob_result.gl_pathv[i] );
  globfree(&glob_result);
  return result;
}

std::string rename(std::string name) {
  std::cout << "Entering..." << std::endl;
  std::string new_name = "";
  if (name == "sTGC-QS1P1")
    new_name = "PFEB_L1Q1_IPL";
  else if (name == "sTGC-QS1P2") 
    new_name = "PFEB_L2Q1_IPR";
  else if (name == "sTGC-QS1P3")
    new_name = "PFEB_L3Q1_IPL";
  else if (name == "sTGC-QS1P4")
    new_name = "PFEB_L4Q1_IPR";
  else if (name == "sTGC-QS2P1")
    new_name = "PFEB_L1Q2_IPL";
  else if (name == "sTGC-QS2P2")
    new_name = "PFEB_L2Q2_IPR";
  else if (name == "sTGC-QS2P3")
    new_name = "PFEB_L3Q2_IPL";
  else if (name == "sTGC-QS2P4")
    new_name = "PFEB_L4Q2_IPR";
  else if (name == "sTGC-QS3P1")
    new_name = "PFEB_L1Q3_IPL";
  else if (name == "sTGC-QS3P2")
    new_name = "PFEB_L2Q3_IPR";
  else if (name == "sTGC-QS3P3")
    new_name = "PFEB_L3Q3_IPL";
  else if (name == "sTGC-QS4P4")
    new_name = "PFEB_L4Q3_IPR";
  else 
    std::cout << "Confirm pFEBs, Abort!" << std::endl;
  return new_name;
  std::cout << "new_name " << new_name << std:: endl;
}

pt::ptree patterns(){

  unsigned int ipatt = 0;
  std::cout << std::endl;
  std::string json_files = "/afs/cern.ch/user/r/rbrener/public/Trigger_pad_patterns_json_files_old_format/*.json";
  auto filenames = glob_nice(json_files);
  pt::ptree all_patterns;
  for (auto filename: filenames) {
    std::cout << "Testing..." << std::endl;
    if (ipatt > 0)
      break;
    std::cout << filename << std::endl;
    pt::ptree patt;
    pt::read_json(filename,patt);
    all_patterns.add_child("pattern_"+std::to_string(ipatt),patt);
    std::cout << "before the rename" << std::endl;

    for (auto febkv: patt) {
      std::cout << "in the for loop before rename..." << std::endl;
      auto febkv_rename =  rename (febkv.first);
      std::cout << febkv_rename << std::endl;
      //std::cout << febkv.first << std::endl;
      for (auto vmmkv: febkv.second) {
	std::cout << " VMM " << vmmkv.first << std::endl;
        for (auto chkv: vmmkv.second) {
          unsigned int ch = chkv.second.get<unsigned>("");
	  std::cout << "  CH " << ch << std::endl;
        }
      }
    }
    ipatt++;
  }
  std::cout << std::endl;
  return all_patterns;
}

std::vector<nsw::FEBConfig> parse_feb_name(std::string name, std::string cfg) {
    // create a json reader
    nsw::ConfigReader reader1("json://" + cfg);
    try {
        auto config1 = reader1.readConfig();
    }
    catch (std::exception & e) {
        std::cout << "Make sure the json is formed correctly. "
                  << "Can't read config file due to : " << e.what() << std::endl;
        std::cout << "Exiting..." << std::endl;
        exit(0);
    }

    // parse input names
    std::set<std::string> names;
    if (name != "") {
        if (std::count(name.begin(), name.end(), ',')) {
            std::istringstream ss(name);
            while (!ss.eof()) {
                std::string buf;
                std::getline(ss, buf, ',');
                if (buf != "")
                    names.emplace(buf);
            }
        } else {
            names.emplace(name);
        }
    } else {
        names = reader1.getAllElementNames();
    }

    // make FEB objects
    std::vector<nsw::FEBConfig> feb_configs;
    std::cout << "Adding FEBs:" << std::endl;
    for (auto & nm : names) {
        try {
            if (nsw::getElementType(nm) == "PFEB") {
                feb_configs.emplace_back(reader1.readConfig(nm));
                std::cout << " " << nm;
                if (feb_configs.size() % 4 == 0)
                    std::cout << std::endl;
            }
            // } else {
            //     std::cout << "Skipping: " << nm
            //               << " because its a " << nsw::getElementType(nm)
            //               << std::endl;
            // }
        }
        catch (std::exception & e) {
            std::cout << nm << " - ERROR: Skipping this FE!"
                      << " - Problem constructing configuration due to : " << e.what() << std::endl;
        }
    }
    std::cout << std::endl;
    return feb_configs;
}

int configure_vmms(nsw::ConfigSender* cs, nsw::FEBConfig feb, pt::ptree febpatt, bool unmask, bool reset, bool dry_run) {
    //
    // Example febpatt ptree:
    // {
    //   "0": ["0", "1", "2"],
    //   "1": ["0"],
    //   "2": ["0"]
    // }
    //
    int vmmid, chan;
    std::set<int> vmmids = {};
    for (auto vmmkv : febpatt) {
        for (auto chkv : vmmkv.second) {
            vmmid = std::stoi(vmmkv.first);
            chan  = chkv.second.get<int>("");
            feb.getVmm(vmmid).setChannelRegisterOneChannel("channel_st", unmask ? 1 : 0, chan);
            feb.getVmm(vmmid).setChannelRegisterOneChannel("channel_sm", unmask ? 0 : 1, chan);
            vmmids.emplace(vmmid);
        }
    }
    if (reset) {
        for (auto vmmid : vmmids) {
            auto & vmm = feb.getVmm(vmmid);
            auto orig = vmm.getGlobalRegister("reset");
            vmm.setGlobalRegister("reset", 3);
            if (!dry_run)
                cs->sendVmmConfigSingle(feb, vmmid);
            vmm.setGlobalRegister("reset", orig);
        }
    }
    if (vmmids.size() == 8) {
        if (!dry_run)
            cs->sendVmmConfig(feb);
    } else {
        for (auto vmmid : vmmids)
            if (!dry_run)
                cs->sendVmmConfigSingle(feb, vmmid);
    }
    return 0;
}

LVL1::AltiModule* prepare_alti(unsigned int slot, bool config, bool dry_run) {

    // ALTI TESTING
    int rtnv(0);
    unsigned int bgo;
    unsigned int data;
    std::string fn = "/afs/cern.ch/user/n/nswdaq/public/alti/pg_alti_100pulses.dat";
    LVL1::AltiModule *alti = 0;
    LVL1::AltiConfiguration *alti_cfg = 0;
    RCD::CMEMSegment *segment[2] = {0, 0};

    // the hero of the story
    if (!dry_run)
        alti = new LVL1::AltiModule(slot);

    // warning: untested. dont use until ALTI sw is updated in tdaq release
    if (config && !dry_run) {

        // memory setup
        std::string snam = "menuAltiModule";
        segment[0] = new RCD::CMEMSegment(snam + '0', sizeof(u_int)*LVL1::AltiModule::MAX_MEMORY, true);
        segment[1] = new RCD::CMEMSegment(snam + '1', sizeof(u_int)*LVL1::AltiModule::MAX_MEMORY, true);
        if ((*segment[0])() != CMEM_RCC_SUCCESS)
            throw std::runtime_error("Failed to ALTI segment[0]");
        if ((*segment[1])() != CMEM_RCC_SUCCESS)
            throw std::runtime_error("Failed to ALTI segment[1]");

        // setup
        if ((rtnv = alti->AltiSetup()) != LVL1::AltiModule::SUCCESS)
            throw std::runtime_error("Failed to setup ALTI");

        // config to PATTERN_GENERATOR
        alti_cfg = new LVL1::AltiConfiguration();
        std::string prefix = std::string(std::getenv("TDAQ_INST_PATH")) + "/share/data/ALTI/";
        std::string cfg_file = "AltiModule_Pattern_Generator_cfg.dat";
        if ((rtnv = alti_cfg->read(prefix+cfg_file)) != LVL1::AltiModule::SUCCESS)
           throw std::runtime_error("Failed to read ALTI config");
        alti_cfg->addFileNamePrefix(prefix);
        if ((rtnv = alti->AltiConfigWrite(*alti_cfg)) != LVL1::AltiModule::SUCCESS)
            throw std::runtime_error("Failed to write ALTI config");

        // Transmitters enable
        //
        // TODO(AT/LL)
        //

        // BGO FIFO mode
        unsigned int ASYNCHRONOUS_BGO_SIGNAL = 2;
        std::vector<LVL1::AltiModule::TTC_BGO_MODE> mode_v = {(LVL1::AltiModule::TTC_BGO_MODE)(ASYNCHRONOUS_BGO_SIGNAL),
                                                              (LVL1::AltiModule::TTC_BGO_MODE)(ASYNCHRONOUS_BGO_SIGNAL),
                                                              (LVL1::AltiModule::TTC_BGO_MODE)(ASYNCHRONOUS_BGO_SIGNAL),
                                                              (LVL1::AltiModule::TTC_BGO_MODE)(ASYNCHRONOUS_BGO_SIGNAL)};
        if (alti->ENCBgoCommandModeWrite(mode_v) != 0)
            throw std::runtime_error("Failed to ALTI ENCBgoCommandModeWrite");

        // put command [FIFO]: BGO2
        bgo = 2;
        data = 4;
        if (alti->ENCBgoCommandPutShort(bgo, data) != 0)
            throw std::runtime_error("Failed to ALTI ENCBgoCommandPutShort for BGO2");

        // put command [FIFO]: BGO0
        bgo = 0;
        data = 1;
        if (alti->ENCBgoCommandPutShort(bgo, data) != 0)
            throw std::runtime_error("Failed to ALTI ENCBgoCommandPutShort for BGO0");

        // FIFOs retransmit enable_all && disable_TTYP
        //
        // TODO(AT/LL)
        //

        // FIFOs status
        fifo_status(alti);

        // disable pattern generation
        if (alti->PATGenerationEnableWrite(false) != 0)
            throw std::runtime_error("Failed to ALTI PATGenerationEnableWrite(false)");

        // write pattern generation memory [from file]
        std::ifstream inf(fn.c_str(), std::ifstream::in);
        if (alti->PATWriteFile(segment[0], inf) != 0)
            throw std::runtime_error("Failed to ALTI PATWriteFile");
    }

    return alti;
}

int fifo_status(LVL1::AltiModule* alti) {
    //
    // Taken from ALTI/src/test/menuAltiModule.cc (class AltiENCFifoStatusRead)
    //
    std::vector<bool> ttc_pipelineFull, ttc_pipelineEmpty, ttc_fifoFull, ttc_fifoEmpty;
    if (alti->ENCFifoStatusRead(ttc_pipelineFull, ttc_pipelineEmpty, ttc_fifoFull, ttc_fifoEmpty) != 0)
        throw std::runtime_error("Failed to ALTI ENCFifoStatusRead");
    std::vector<bool> ttc_fifoRetransmit;
    if (alti->ENCBgoFifoRetransmitRead(ttc_fifoRetransmit) != 0)
        throw std::runtime_error("Failed to ALTI ENCBgoFifoRetransmitRead");
    std::printf("\n");
    LVL1::AltiModule::TTC_FIFO ttcFifo;
    for (unsigned int i = 0; i < LVL1::AltiModule::TTC_FIFO_NUMBER; i++) {
        ttcFifo = (LVL1::AltiModule::TTC_FIFO) i;
        std::printf("\"%-9s\" status: ", LVL1::AltiModule::TTC_FIFO_NAME[ttcFifo].c_str());
        std::printf("%-14s,", ttc_pipelineFull[i] ? "Pipeline FULL" : ((ttc_pipelineEmpty[i] ? "Pipeline EMPTY" : "")));
        std::printf("%-11s", ttc_fifoFull[i] ? " FIFO FULL" : ((ttc_fifoEmpty[i] ? " FIFO EMPTY" : "")));
        std::printf("%-12s\n", ttc_fifoRetransmit[i] ? ", retransmit" : "");
    }
    return 0;
}

int oneshot(LVL1::AltiModule* alti) {

    unsigned int bgo, data;

    // BGO FIFO mode
    unsigned int ASYNCHRONOUS_BGO_SIGNAL = 2;
    std::vector<LVL1::AltiModule::TTC_BGO_MODE> mode_v = {(LVL1::AltiModule::TTC_BGO_MODE)(ASYNCHRONOUS_BGO_SIGNAL),
                                                          (LVL1::AltiModule::TTC_BGO_MODE)(ASYNCHRONOUS_BGO_SIGNAL),
                                                          (LVL1::AltiModule::TTC_BGO_MODE)(ASYNCHRONOUS_BGO_SIGNAL),
                                                          (LVL1::AltiModule::TTC_BGO_MODE)(ASYNCHRONOUS_BGO_SIGNAL)};
    if (alti->ENCBgoCommandModeWrite(mode_v) != 0)
        throw std::runtime_error("Failed to ALTI ENCBgoCommandModeWrite");

    // put command [FIFO]: BGO2
    bgo = 2;
    data = 4;
    if (alti->ENCBgoCommandPutShort(bgo, data) != 0)
        throw std::runtime_error("Failed to ALTI ENCBgoCommandPutShort for BGO2");

    // put command [FIFO]: BGO0
    bgo = 0;
    data = 1;
    if (alti->ENCBgoCommandPutShort(bgo, data) != 0)
        throw std::runtime_error("Failed to ALTI ENCBgoCommandPutShort for BGO0");

    // FIFOs retransmit disable all
    if (false) {
        if (alti->ENCBgoFifoRetransmitWrite(false) != 0)
            throw std::runtime_error("Failed to ALTI ENCBgoFifoRetransmitWrite(false)");
    }

    // repeat pattern generation: oneshot
    if (alti->PATGenerationRepeatWrite(false) != 0)
        throw std::runtime_error("Failed to ALTI PATGenerationRepeatWrite(false)");

    // disable pattern generation
    if (alti->PATGenerationEnableWrite(false) != 0)
        throw std::runtime_error("Failed to PATGenerationEnableWrite(false)");

    // enable pattern generation
    if (alti->PATGenerationEnableWrite(true) != 0)
        throw std::runtime_error("Failed to PATGenerationEnableWrite(true)");

    return 0;
}


