//
// Program to test pulse triggerable patterns, MM edition
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
#include "ALTI/AltiModule.h"
#include "ALTI/AltiConfiguration.h"
#include "RCDVme/RCDCmemSegment.h"

namespace po = boost::program_options;

class FEBPattern {
public:
    std::string name;
    std::vector< std::pair<int, int> > vmmchs;
    FEBPattern(std::string nm): name(nm) { vmmchs.clear(); }
};

LVL1::AltiModule* prepare_alti(unsigned int slot, bool do_config, bool dry_run);
int fifo_status(LVL1::AltiModule* alti);
int oneshot(LVL1::AltiModule* alti);
int wait_until_done(std::vector< std::future<int> >* threads);
int configure_vmms(nsw::ConfigSender* cs, nsw::FEBConfig feb, FEBPattern febpatt, bool unmask, bool reset, bool dry_run);
int configure_art_input_phase(nsw::ConfigSender* cs, nsw::ADDCConfig addc, uint phase);
std::vector<nsw::FEBConfig> parse_feb_name(std::string name, std::string cfg);
std::vector<nsw::ADDCConfig> parse_addc_name(std::string name, std::string cfg);
std::vector< std::vector< FEBPattern > > patterns();
std::string strf_time();
int minutes_remaining(double time_diff, int nprocessed, int ntotal);

int main(int argc, const char *argv[]) {
    std::string config_filename;
    std::string addc_filename;
    std::string board_name_mmfe8;
    std::string board_name_addc;
    int max_threads;
    unsigned int alti_slot;
    bool dry_run;
    bool reset_vmm;

    // CL options
    po::options_description desc(std::string("ADDC configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file,c", po::value<std::string>(&config_filename)->
         default_value("/afs/cern.ch/user/n/nswdaq/public/sw/config-ttc/config-files/full_small_sector_12.json"),
         "Configuration file path")
        ("addc_file,a", po::value<std::string>(&addc_filename)->
         default_value(""), "ADDC config file path")
        ("dry_run", po::bool_switch()->
         default_value(false), "Option to NOT send configurations")
        ("reset_vmm", po::bool_switch()->
         default_value(false), "Option to reset VMM at every configuration")
        ("max_threads,m", po::value<int>(&max_threads)->
         default_value(-1), "Maximum number of threads to run")
        ("alti", po::value<unsigned int>(&alti_slot)->
         default_value(11), "ALTI slot")
        ("mmfe8", po::value<std::string>(&board_name_mmfe8)->
         default_value(""), "The name of frontend to configure (should start with MMFE8_).")
        ("addc", po::value<std::string>(&board_name_addc)->
         default_value(""), "The name of ADDC to configure (should start with ADDC_).")
        ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    dry_run   = vm["dry_run"  ].as<bool>();
    reset_vmm = vm["reset_vmm"].as<bool>();
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    // alti
    auto alti = prepare_alti(alti_slot, false, dry_run);
    if (!dry_run)
        fifo_status(alti);

    // announce
    std::cout << "Dry run:          " << dry_run     << std::endl;
    std::cout << "VMM hard resets:  " << reset_vmm   << std::endl;
    std::cout << "ALTI slot:        " << alti_slot   << std::endl;

    // the febs
    auto febs = parse_feb_name(board_name_mmfe8, config_filename);
    std::cout << "Number of FEBs found: " << febs.size() << std::endl;

    // the addcs
    std::vector<nsw::ADDCConfig> addcs = {};
    if (addc_filename.length() > 0)
        addcs = parse_addc_name(board_name_addc, addc_filename);
    std::cout << "Number of ADDCs found: " << addcs.size() << std::endl;

    // the configuration threads
    std::cout << "Creating vector of threads..." << std::endl;
    auto threads = new std::vector< std::future<int> >();

    // limit 1 sender per board
    std::cout << "Creating map of ConfigSender*..." << std::endl;
    auto senders = new std::map< std::string, nsw::ConfigSender* >();
    for (auto & feb : febs)
        senders->insert( {feb.getAddress(), new nsw::ConfigSender()} );
    for (auto & addc : addcs)
        senders->insert( {addc.getAddress(), new nsw::ConfigSender()} );

    // keep time
    std::chrono::time_point<std::chrono::system_clock> time_start;
    std::chrono::duration<double> elapsed_seconds;
    time_start = std::chrono::system_clock::now();

    // loop over patterns
    int ipatt = 0;
    auto patts = patterns();
    for (auto pattern : patts) {

        ipatt++;
        std::cout << "> " << ipatt << " / " << patts.size() << " :: ";

        // enable test pulse
        // dont limit threads - only 8 MMFE8s, typically
        bool anything = 0;
        std::cout << "Enabling " << pattern.size() << " MMFE8s...";
        for (auto febpatt: pattern) {
            for (auto & feb : febs) {
                if (febpatt.name != feb.getAddress())
                    continue;
                anything = 1;
                threads->push_back(std::async(std::launch::async, configure_vmms,
                                              senders->at(feb.getAddress()), feb,
                                              febpatt, true, reset_vmm, dry_run));
                break;
            }
        }
        std::cout << " done. ";
        wait_until_done(threads);
        if (!anything) {
            std::cout << " Skipping.";
            std::cout << std::flush << std::endl;
            continue;
        }

        // run the TTC commands
        if (addcs.size() == 0) {
            if (!dry_run)
                oneshot(alti);
        } else {
            // or, loop through ADDC input phases and run TTC
            std::cout << "Setting input phase of " << addcs.size() << " ADDCs to be: ";
            uint nphase = 16;
            for (uint phase = 0; phase < nphase; phase++) {
                std::cout << std::hex << phase << std::dec;
                for (auto & addc : addcs)
                    if (!dry_run)
                        threads->push_back(std::async(std::launch::async,
                                                      configure_art_input_phase,
                                                      senders->at(addc.getAddress()), addc, phase));
                wait_until_done(threads);
                if (!dry_run)
                    oneshot(alti);
            }
            std::cout << " done. ";
        }

        // disable test pulse
        std::cout << "Disabling " << pattern.size() << " MMFE8s...";
        for (auto febpatt: pattern) {
            for (auto & feb : febs) {
                if (febpatt.name != feb.getAddress())
                    continue;
                threads->push_back(std::async(std::launch::async, configure_vmms,
                                              senders->at(feb.getAddress()), feb,
                                              febpatt, false, reset_vmm, dry_run));
                break;
            }
        }
        std::cout << " done. ";
        wait_until_done(threads);

        // wrap up
        elapsed_seconds = (std::chrono::system_clock::now() - time_start);
        std::cout << "~" << minutes_remaining(elapsed_seconds.count(), ipatt, patts.size()) << " min left.";
        std::cout << std::flush;
        std::cout << std::endl;
    }

    // wrap up
    std::cout << std::endl;
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

std::vector< std::vector< FEBPattern > > patterns() {
    //
    // patterns = [ [ (MMFE8_L1P1_HOR, ((0, 0), (4, 0))), (MMFE8_L2P1_HOL, ((0, 0), (4, 0))), ... ], # pattern 0
    //              [ (MMFE8_L1P1_HOR, ((0, 1), (4, 1))), (MMFE8_L2P1_HOL, ((0, 1), (4, 1))), ... ], # pattern 1
    //              [ (MMFE8_L1P1_HOR, ((0, 2), (4, 2))), (MMFE8_L2P1_HOL, ((0, 2), (4, 2))), ... ], # pattern 2
    //
    // In words:
    //   Return a vector of patterns.
    //   Each pattern contains a vector of FEBPattern objects,
    //   and each FEBPattern has a vector of (VMM, CH) pairs.
    //
    std::vector< std::vector< FEBPattern > > patts = {};

    //
    // connectivity max-parallel loop
    //
    bool even;
    int nvmm = 8;
    int nchan = 64;
    int pcb = 0;
    for (int pos = 0; pos < 16; pos++) {
        even = pos % 2 == 0;
        pcb  = pos / 2 + 1;
        auto pcbstr = std::to_string(pcb);
        for (int chan = 0; chan < nchan; chan++) {
            if (chan % 10 != 0)
                continue;
            std::vector< FEBPattern > patt = {};
            for (auto name: {"MMFE8_L1P" + pcbstr + "_HO" + (even ? "R" : "L"),
                             "MMFE8_L2P" + pcbstr + "_HO" + (even ? "L" : "R"),
                             "MMFE8_L3P" + pcbstr + "_HO" + (even ? "R" : "L"),
                             "MMFE8_L4P" + pcbstr + "_HO" + (even ? "L" : "R"),
                             "MMFE8_L4P" + pcbstr + "_IP" + (even ? "R" : "L"),
                             "MMFE8_L3P" + pcbstr + "_IP" + (even ? "L" : "R"),
                             "MMFE8_L2P" + pcbstr + "_IP" + (even ? "R" : "L"),
                             "MMFE8_L1P" + pcbstr + "_IP" + (even ? "L" : "R")}) {
                patt.push_back( FEBPattern(name) );
                for (int vmmid = 0; vmmid < nvmm; vmmid++)
                    patt.back().vmmchs.push_back(std::make_pair(vmmid, chan));
            }
            patts.push_back(patt);
        }
    }
    return patts;
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
    for (auto & nm : names) {
        try {
            if (nsw::getElementType(nm) == "MMFE8") {
                feb_configs.emplace_back(reader1.readConfig(nm));
                std::cout << "Adding: " << nm << std::endl;
            } else {
                std::cout << "Skipping: " << nm
                          << " because its a " << nsw::getElementType(nm)
                          << std::endl;
            }
        }
        catch (std::exception & e) {
            std::cout << nm << " - ERROR: Skipping this FE!"
                      << " - Problem constructing configuration due to : " << e.what() << std::endl;
        }
    }
    return feb_configs;
}

std::vector<nsw::ADDCConfig> parse_addc_name(std::string name, std::string cfg) {
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

    // make ADDC objects
    std::vector<nsw::ADDCConfig> addc_configs;
    for (auto & nm : names) {
        try {
            if (nsw::getElementType(nm) == "ADDC") {
                addc_configs.emplace_back(reader1.readConfig(nm));
                std::cout << "Adding: " << nm << std::endl;
            } else {
                std::cout << "Skipping: " << nm
                          << " because its a " << nsw::getElementType(nm)
                          << std::endl;
            }
        }
        catch (std::exception & e) {
            std::cout << nm << " - ERROR: Skipping this FE!"
                      << " - Problem constructing configuration due to : " << e.what() << std::endl;
        }
    }
    return addc_configs;
}

int configure_vmms(nsw::ConfigSender* cs, nsw::FEBConfig feb, FEBPattern febpatt, bool unmask, bool reset, bool dry_run) {
    int vmmid, ch;
    std::set<int> vmmids = {};
    for (auto kv: febpatt.vmmchs) {
        std::tie(vmmid, ch) = kv;
        feb.getVmm(vmmid).setChannelRegisterOneChannel("channel_st", unmask ? 1 : 0, ch);
        feb.getVmm(vmmid).setChannelRegisterOneChannel("channel_sm", unmask ? 0 : 1, ch);
        vmmids.emplace(vmmid);
    }
    if (!dry_run) {
        if (reset) {
            for (auto vmmid : vmmids) {
                auto & vmm = feb.getVmm(vmmid);
                auto orig = vmm.getGlobalRegister("reset");
                vmm.setGlobalRegister("reset", 3);
                cs->sendVmmConfigSingle(feb, vmmid);
                vmm.setGlobalRegister("reset", orig);
            }
        }
        if (vmmids.size() == 8)
            cs->sendVmmConfig(feb);
        else
            for (auto vmmid : vmmids)
                cs->sendVmmConfigSingle(feb, vmmid);
    }
    return 0;
}

int configure_art_input_phase(nsw::ConfigSender* cs, nsw::ADDCConfig addc, uint phase) {
    if (phase > std::pow(2, 4))
        throw std::runtime_error("Gave bad phase to configure_art_input_phase: " + std::to_string(phase));
    size_t art_size = 2;
    uint8_t art_data[] = {0x0, 0x0};
    auto opc_ip   = addc.getOpcServerIp();
    auto sca_addr = addc.getAddress();
    uint8_t this_phase = phase + (phase << 4);
    // std::cout << "Setting input phase of " << sca_addr << " to be 0x"
    //           << std::hex << (uint)(this_phase) << std::dec << std::endl;
    for (auto art : addc.getARTs()) {
        auto name = sca_addr + "." + art.getName() + "Ps" + "." + art.getName() + "Ps";
        for (auto reg : { 6,  7,  8,  9,
                         21, 22, 23, 24,
                         36, 37, 38, 39,
                         51, 52, 53, 54,
                    }) {
            art_data[0] = (uint8_t)(reg);
            art_data[1] = this_phase;
            cs->sendI2cRaw(opc_ip, name, art_data, art_size);
        }
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
