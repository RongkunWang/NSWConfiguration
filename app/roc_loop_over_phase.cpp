// Program to loop over phase registers in ROC configuration

#include <iostream>
#include <string>
#include <vector>
#include <map>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

int main(int ac, const char *av[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    // bool configure_vmm;
    std::string config_filename;
    po::options_description desc("This program configures ROC with some command line options");
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "integration_config.json"),
        "Configuration file path");

    /*po::options_description vmm("VMM Options");
    vmm.add_options()
        ("configure-vmm,v", po::bool_switch(&configure_vmm)->default_value(false),
        "Configure also all the VMMs on the ROC(Default: False)");
    */

    // Declare an options description instance which will include all the options
    po::options_description all("Allowed options");
    all.add(desc);  // .add(vmm);

    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, all), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << all << "\n";
        return 1;
    }

    nsw::ConfigReader reader1("json://" + config_filename);
    auto config1 = reader1.readConfig();

    // ROC Config
    auto rocconfig0 = reader1.readConfig("A01.ROC_L01_M01");
    nsw::ROCConfig roc0(rocconfig0);

    nsw::ConfigSender cs;

    std::string input = "";
    // for (uint32_t i = 0; i < 128; i++) {
    for (uint32_t i = 80; i < 128; i=i+1) {
        //std::cout << "Press enter to go to next step: " << std::endl;
        //getline(std::cin, input);
        sleep(1);
        uint32_t phase40 = i;

        uint32_t phase160 = i % 32;
        uint32_t shift40_ps = 200 * i;  // Phase shift of 40MHz in ps
        uint32_t shift160_ps = 200 * phase160;

        std::cout << "----------------> phase40: " << shift40_ps << " - phase160: " << shift160_ps << std::endl;
        //std::cout << " - phase160: " << shift160_ps << std::endl;
        //continue;

        roc0.analog.setRegisterValue("reg115", "ePllPhase160MHz_0[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg116", "ePllPhase160MHz_0[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg117", "ePllPhase160MHz_0[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg118", "ePllPhase160MHz_0[0:3]", phase160 & 15);
        roc0.analog.setRegisterValue("reg118", "ePllPhase160MHz_1[0:3]", phase160 & 15);
        roc0.analog.setRegisterValue("reg119", "ePllPhase160MHz_0[3:0]", phase160 & 15);

        roc0.analog.setRegisterValue("reg115", "ePllPhase40MHz_0", phase40);
        roc0.analog.setRegisterValue("reg116", "ePllPhase40MHz_0", phase40);
        roc0.analog.setRegisterValue("reg117", "ePllPhase40MHz_0", phase40);

        /* Following are for setting the phase of data lines
        roc0.analog.setRegisterValue("reg064ePllVmm0", "ePllPhase160MHz_0[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg065ePllVmm0", "ePllPhase160MHz_1[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg066ePllVmm0", "ePllPhase160MHz_2[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg067ePllVmm0", "ePllPhase160MHz_3[4]", phase160 >> 4);

        // roc0.analog.setRegisterValue("reg064ePllVmm0", "ePllPhase40MHz_0", phase40);
        // roc0.analog.setRegisterValue("reg065ePllVmm0", "ePllPhase40MHz_1", phase40);
        // roc0.analog.setRegisterValue("reg066ePllVmm0", "ePllPhase40MHz_2", phase40);
        // roc0.analog.setRegisterValue("reg067ePllVmm0", "ePllPhase40MHz_3", phase40);

        roc0.analog.setRegisterValue("reg068ePllVmm0", "ePllPhase160MHz_0[3:0]", phase160 & 15);
        roc0.analog.setRegisterValue("reg068ePllVmm0", "ePllPhase160MHz_1[3:0]", phase160 & 15);
        roc0.analog.setRegisterValue("reg069ePllVmm0", "ePllPhase160MHz_2[3:0]", phase160 & 15);
        roc0.analog.setRegisterValue("reg069ePllVmm0", "ePllPhase160MHz_3[3:0]", phase160 & 15);

        // ----
        roc0.analog.setRegisterValue("reg080ePllVmm1", "ePllPhase160MHz_0[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg081ePllVmm1", "ePllPhase160MHz_1[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg082ePllVmm1", "ePllPhase160MHz_2[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg083ePllVmm1", "ePllPhase160MHz_3[4]", phase160 >> 4);

        // roc0.analog.setRegisterValue("reg080ePllVmm1", "ePllPhase40MHz_0", phase40);
        // roc0.analog.setRegisterValue("reg081ePllVmm1", "ePllPhase40MHz_1", phase40);
        // roc0.analog.setRegisterValue("reg082ePllVmm1", "ePllPhase40MHz_2", phase40);
        // roc0.analog.setRegisterValue("reg083ePllVmm1", "ePllPhase40MHz_3", phase40);

        roc0.analog.setRegisterValue("reg084ePllVmm1", "ePllPhase160MHz_0[3:0]", phase160 & 15);
        roc0.analog.setRegisterValue("reg084ePllVmm1", "ePllPhase160MHz_1[3:0]", phase160 & 15);
        roc0.analog.setRegisterValue("reg085ePllVmm1", "ePllPhase160MHz_2[3:0]", phase160 & 15);
        roc0.analog.setRegisterValue("reg085ePllVmm1", "ePllPhase160MHz_3[3:0]", phase160 & 15);

        // ----
        roc0.analog.setRegisterValue("reg096ePllTdc", "ePllPhase160MHz_0[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg097ePllTdc", "ePllPhase160MHz_1[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg098ePllTdc", "ePllPhase160MHz_2[4]", phase160 >> 4);
        roc0.analog.setRegisterValue("reg099ePllTdc", "ePllPhase160MHz_3[4]", phase160 >> 4);

        // roc0.analog.setRegisterValue("reg096ePllTdc", "ePllPhase40MHz_0", phase40);
        // roc0.analog.setRegisterValue("reg097ePllTdc", "ePllPhase40MHz_1", phase40);
        // roc0.analog.setRegisterValue("reg098ePllTdc", "ePllPhase40MHz_2", phase40);
        // roc0.analog.setRegisterValue("reg099ePllTdc", "ePllPhase40MHz_3", phase40);

        roc0.analog.setRegisterValue("reg100ePllTdc", "ePllPhase160MHz_0[3:0]", phase160 & 15);
        roc0.analog.setRegisterValue("reg100ePllTdc", "ePllPhase160MHz_1[3:0]", phase160 & 15);
        roc0.analog.setRegisterValue("reg101ePllTdc", "ePllPhase160MHz_2[3:0]", phase160 & 15);
        roc0.analog.setRegisterValue("reg101ePllTdc", "ePllPhase160MHz_3[3:0]", phase160 & 15);
        */


        // Send all ROC config
        cs.sendRocConfig(roc0);

        //std::cout << "Press enter to configure VMM: " << std::endl;
        //getline(std::cin, input);

        if (true) {
            // Inverse VMM enable to get VMM into config mode
            uint8_t data[] = {0xff};
            size_t size = 1;
            auto opc_ip = roc0.getOpcServerIp();
            auto sca_roc_address_analog = roc0.getAddress() + "." + roc0.analog.getName();
            cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data, size);

            std::vector<std::string> vmmids = {"0", "1", "2", "3", "4", "5", "6", "7"};
            for (auto vmmid : vmmids) {
                auto vmmconfig = reader1.readConfig("A01.VMM_L01_M01_0" + vmmid);
                nsw::VMMConfig vmm(vmmconfig);
                // auto vec = vmm.getByteVector();  /// Create a vector of bytes
                cs.sendVmmConfig(vmm);

                // for (auto el : vec) {
                //    std::cout << "0x" << std::hex << unsigned(el) << ", ";
                //}
                // std::cout << std::dec << std::endl;;
            }

            // Set back the register
            data[0] = {static_cast<uint8_t>(0x0)};
            cs.sendI2cRaw(opc_ip, sca_roc_address_analog + ".reg122vmmEnaInv",  data, size);
        }
    }

    return 0;
}
