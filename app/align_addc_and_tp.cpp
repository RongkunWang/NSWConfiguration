// Program to set ADDC configuration and compare goodness with TP

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/OpcClient.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

size_t art_size = 2;
uint8_t art_data[] = {0x0,0x0};
bool debug = true;

struct ThreadConfig {};
int align_art_and_tp (std::shared_ptr<nsw::ConfigSender> cs, nsw::FEConfig feb, int art);
int alignment_loop   (std::shared_ptr<nsw::ConfigSender> cs, nsw::FEConfig feb, int art);
int check_tp_link    (int tmp);

int main(int argc, const char *argv[]) 
{

    bool addc_config;
    bool phase_scan_gbtx;
    bool phase_scan_art;
    std::string maskA0;
    std::string maskA1;
    bool mask_art_channels;
	int addc_id;
    std::string config_filename;
    std::string fe_name;

    po::options_description desc(std::string("ADDC configuration script"));
    desc.add_options()
        ("help,h", "produce help message")
        ("config_file", po::value<std::string>(&config_filename)->
         default_value("/afs/cern.ch/user/n/nswdaq/public/addc/addc_test_mmfe8_1_mwe.json"),
         "Configuration file path")
        ("name,n", po::value<std::string>(&fe_name)->
         default_value("ADDC_config_00,ADDC_config_01"),
         "The name of frontend to configure (should contain ADDC).")
        ("configure_addc,c", po::bool_switch(&addc_config)->default_value(false),
        "configure ADDC GBTx and ART ASICs")
        ("phase_scan_gbtx,g", po::bool_switch(&phase_scan_gbtx)->default_value(false),
        "Perform phase scan to align 160MHz GBTx - ART Elink")
        ("phase_scan_art,a", po::bool_switch(&phase_scan_art)->default_value(false),
        "Perform phase scan of ART input ports")
        ("mask_art,m", po::bool_switch(&mask_art_channels)->default_value(false),
        "Mask ART input channels of the ART ASICs")
        ("mask_art_0_reg,o", po::value<std::string>(&maskA0)->default_value("0xffffffff"),
        "ART0 Input mask pattern")
        ("mask_art_1_reg,p", po::value<std::string>(&maskA1)->default_value("0xffffffff"),
        "ART1 Input mask pattern")
		("addc_id,d", po::value<int>(&addc_id)->default_value(0),
		"Select which addc to configure (0 - 1)");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }    

    // create a json reader
    nsw::ConfigReader reader1("json://" + config_filename);
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
    std::set<std::string> frontend_names;
    if (fe_name != ""){
        if (std::count(fe_name.begin(), fe_name.end(), ',')){
            std::istringstream ss(fe_name);
            while(!ss.eof()){
                std::string buf;
                std::getline(ss, buf, ',');
                if (buf != "")
                    frontend_names.emplace(buf);
            }
        }
        else
            frontend_names.emplace(fe_name);
    }
    else
        frontend_names = reader1.getAllElementNames();

    // make FE objects
    std::vector<nsw::FEConfig> frontend_configs;
    for (auto & name : frontend_names) {
        try {
            frontend_configs.emplace_back(reader1.readConfig(name));
            std::cout << "Found " << name << std::endl;
        } 
        catch (std::exception & e) {
            std::cout << name << " - ERROR: Skipping this FE!"
                      << " - Problem constructing configuration due to : " << e.what() << std::endl;
        }
    }

    // check the boards
    auto cs = std::make_shared<nsw::ConfigSender>();
    for (auto feb: frontend_configs){
        int status_0 = align_art_and_tp(cs, feb, 0);
        int status_1 = align_art_and_tp(cs, feb, 1);
        std::cout << "Phase for " << feb.getAddress() << " ART0: " << status_0 << std::endl;
        std::cout << "Phase for " << feb.getAddress() << " ART1: " << status_1 << std::endl;
    }

    return 0;
}

int align_art_and_tp(std::shared_ptr<nsw::ConfigSender> cs, nsw::FEConfig feb, int art) {

    int n_retries     = 0;
    int n_max_retries = 3;
    while (n_retries < n_max_retries) {
        int status = alignment_loop(cs, feb, art);
        if (status != -1)
            return status;
        else
            n_retries++;
    }
    std::cout << "Failed to align ART" << art << std::endl;
    return -1;

}

int alignment_loop(std::shared_ptr<nsw::ConfigSender> cs, nsw::FEConfig feb, int art) {

    size_t gbtx_size = 3;
    uint8_t gbtx_data[] = {0x0,0x0,0x0};
    std::string sca_reg = ".gbtx" + std::to_string(art) + ".gbtx" + std::to_string(art);
    if (debug)
        std::cout << "alignment_loop :: " << feb.getAddress() + sca_reg << std::endl;

    for (uint i=0; i<32; i++)
        {
            if (debug)
                std::cout << "phase = " << i << std::endl;
            gbtx_data[1] = 0; 
            gbtx_data[0] = 8;
            gbtx_data[2] = i;
            cs->sendI2cRaw(feb.getOpcServerIp(), feb.getAddress() + sca_reg, gbtx_data, gbtx_size);
            sleep(1);
            if (check_tp_link(i)){
                if (debug)
                    std::cout << " ^ Success!" << std::endl;
                return (int)(i);
            }
        }
    return -1;

}

int check_tp_link(int tmp) {
    return (tmp==7) ? 1 : 0;
}
