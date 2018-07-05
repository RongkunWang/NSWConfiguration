// Program to loop over phase registers in ROC configuration

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"

#include "boost/program_options.hpp"
#include "boost/property_tree/json_parser.hpp"

namespace po = boost::program_options;

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    // bool configure_vmm;
    std::string fe_name;
    std::string config_filename;
    uint32_t start_value;
    uint32_t increment;
    bool vmm_roc_phase;
    po::options_description desc("This program loops over the 40MHz and 160MHz TTC clock phases of ROC for the selected front end");
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "integration_config.json"),
        "Configuration file path")
        ("name,n", po::value<std::string>(&fe_name)->default_value("MMFE8-0001"),
        "The name of frontend to configure (must contain MMFE8, SFEB or PFEB)")
        ("vmm-roc-phase,v", po::bool_switch(&vmm_roc_phase)->default_value(false),
        "Instead of clock phase, scan the vmm-roc phase required for data alignment (Default: False)")
        ("start_value,s", po::value<uint32_t>(&start_value)->default_value(0), "The start value of phase scan (0-128)")
        ("increment,i", po::value<uint32_t>(&increment)->default_value(5),
        "Step size to increment the value at each step(0-128)");

    // Declare an options description instance which will include all the options
    po::options_description all("Allowed options");
    all.add(desc);

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, all), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << all << "\n";
        return 1;
    }

    nsw::ConfigReader reader1("json://" + config_filename);
    auto config1 = reader1.readConfig();
    // boost::property_tree::json_parser::write_json(std::cout, reader1.readConfig());
    auto feb_config_tree = reader1.readConfig(fe_name);
    nsw::FEBConfig feb(feb_config_tree);
    // feb.dump();

    auto & roc_analog = feb.getRocAnalog();

    nsw::ConfigSender cs;

    std::string input = "";
    std::cout << "\n";
    if(!vmm_roc_phase) {
        std::cout << "Looping over ROC TTC Clock phase" << std::endl;
    } else {
        std::cout << "Looping over VMM-ROC phase" << std::endl;
    }

    for (uint32_t i = start_value; i < 128; i=i+increment) {
        std::cout << " -- Press enter to go to next step -- " << std::endl;
        getline(std::cin, input);
        sleep(1);
        uint32_t phase40 = i;

        uint32_t phase160 = i % 32;
        uint32_t shift40_ps = 200 * i;  // Phase shift of 40MHz in ps
        uint32_t shift160_ps = 200 * phase160;

        std::cout << "-- step: " << i
                  <<  " ----------------> phase40: " << shift40_ps
                  << " - phase160: " << shift160_ps << std::endl;

        // std::cout << " - phase160: " << shift160_ps << std::endl;
        // continue;

        if(!vmm_roc_phase) {
            // Following are for setting the phase of TTC clock
            roc_analog.setRegisterValue("reg115", "ePllPhase160MHz_0[4]", phase160 >> 4);
            roc_analog.setRegisterValue("reg116", "ePllPhase160MHz_0[4]", phase160 >> 4);

            roc_analog.setRegisterValue("reg117", "ePllPhase160MHz_0[4]", phase160 >> 4);
            roc_analog.setRegisterValue("reg118", "ePllPhase160MHz_0[0:3]", phase160 & 15);
            roc_analog.setRegisterValue("reg118", "ePllPhase160MHz_1[0:3]", phase160 & 15);
            roc_analog.setRegisterValue("reg119", "ePllPhase160MHz_0[3:0]", phase160 & 15);

            roc_analog.setRegisterValue("reg115", "ePllPhase40MHz_0", phase40);
            roc_analog.setRegisterValue("reg116", "ePllPhase40MHz_0", phase40);
            roc_analog.setRegisterValue("reg117", "ePllPhase40MHz_0", phase40);
        } else {
            // Following are for setting the phase of data lines
            roc_analog.setRegisterValue("reg064ePllVmm0", "ePllPhase160MHz_0[4]", phase160 >> 4);
            roc_analog.setRegisterValue("reg065ePllVmm0", "ePllPhase160MHz_1[4]", phase160 >> 4);
            roc_analog.setRegisterValue("reg066ePllVmm0", "ePllPhase160MHz_2[4]", phase160 >> 4);
            roc_analog.setRegisterValue("reg067ePllVmm0", "ePllPhase160MHz_3[4]", phase160 >> 4);

            roc_analog.setRegisterValue("reg064ePllVmm0", "ePllPhase40MHz_0", phase40);
            roc_analog.setRegisterValue("reg065ePllVmm0", "ePllPhase40MHz_1", phase40);
            roc_analog.setRegisterValue("reg066ePllVmm0", "ePllPhase40MHz_2", phase40);
            roc_analog.setRegisterValue("reg067ePllVmm0", "ePllPhase40MHz_3", phase40);

            roc_analog.setRegisterValue("reg068ePllVmm0", "ePllPhase160MHz_0[3:0]", phase160 & 15);
            roc_analog.setRegisterValue("reg068ePllVmm0", "ePllPhase160MHz_1[3:0]", phase160 & 15);
            roc_analog.setRegisterValue("reg069ePllVmm0", "ePllPhase160MHz_2[3:0]", phase160 & 15);
            roc_analog.setRegisterValue("reg069ePllVmm0", "ePllPhase160MHz_3[3:0]", phase160 & 15);

            // ----
            roc_analog.setRegisterValue("reg080ePllVmm1", "ePllPhase160MHz_0[4]", phase160 >> 4);
            roc_analog.setRegisterValue("reg081ePllVmm1", "ePllPhase160MHz_1[4]", phase160 >> 4);
            roc_analog.setRegisterValue("reg082ePllVmm1", "ePllPhase160MHz_2[4]", phase160 >> 4);
            roc_analog.setRegisterValue("reg083ePllVmm1", "ePllPhase160MHz_3[4]", phase160 >> 4);

            roc_analog.setRegisterValue("reg080ePllVmm1", "ePllPhase40MHz_0", phase40);
            roc_analog.setRegisterValue("reg081ePllVmm1", "ePllPhase40MHz_1", phase40);
            roc_analog.setRegisterValue("reg082ePllVmm1", "ePllPhase40MHz_2", phase40);
            roc_analog.setRegisterValue("reg083ePllVmm1", "ePllPhase40MHz_3", phase40);

            roc_analog.setRegisterValue("reg084ePllVmm1", "ePllPhase160MHz_0[3:0]", phase160 & 15);
            roc_analog.setRegisterValue("reg084ePllVmm1", "ePllPhase160MHz_1[3:0]", phase160 & 15);
            roc_analog.setRegisterValue("reg085ePllVmm1", "ePllPhase160MHz_2[3:0]", phase160 & 15);
            roc_analog.setRegisterValue("reg085ePllVmm1", "ePllPhase160MHz_3[3:0]", phase160 & 15);

            // ----
            roc_analog.setRegisterValue("reg096ePllTdc", "ePllPhase160MHz_0[4]", phase160 >> 4);
            roc_analog.setRegisterValue("reg097ePllTdc", "ePllPhase160MHz_1[4]", phase160 >> 4);
            roc_analog.setRegisterValue("reg098ePllTdc", "ePllPhase160MHz_2[4]", phase160 >> 4);
            roc_analog.setRegisterValue("reg099ePllTdc", "ePllPhase160MHz_3[4]", phase160 >> 4);

            roc_analog.setRegisterValue("reg096ePllTdc", "ePllPhase40MHz_0", phase40);
            roc_analog.setRegisterValue("reg097ePllTdc", "ePllPhase40MHz_1", phase40);
            roc_analog.setRegisterValue("reg098ePllTdc", "ePllPhase40MHz_2", phase40);
            roc_analog.setRegisterValue("reg099ePllTdc", "ePllPhase40MHz_3", phase40);

            roc_analog.setRegisterValue("reg100ePllTdc", "ePllPhase160MHz_0[3:0]", phase160 & 15);
            roc_analog.setRegisterValue("reg100ePllTdc", "ePllPhase160MHz_1[3:0]", phase160 & 15);
            roc_analog.setRegisterValue("reg101ePllTdc", "ePllPhase160MHz_2[3:0]", phase160 & 15);
            roc_analog.setRegisterValue("reg101ePllTdc", "ePllPhase160MHz_3[3:0]", phase160 & 15);
        }

        // Send all ROC config
        cs.sendRocConfig(feb);
    }

    return 0;
}
