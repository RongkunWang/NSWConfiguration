// Sample program to read MMFE8 pcb location based on SCA GPIOS

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <numeric>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/FEBConfig.h"

#include "boost/program_options.hpp"

namespace po = boost::program_options;

std::string returnMMFE8Location(std::string input)
{
 
        //Eta 
        if(input== "111111") return "Large Sector, Eta, PCB 8 Right";
        if(input== "111110") return "Large Sector, Eta, PCB 8 Left";
        if(input== "111101") return "Large Sector, Eta, PCB 7 Right";
        if(input== "111100") return "Large Sector, Eta, PCB 7 Left";    
        if(input== "111011") return "Large Sector, Eta, PCB 6 Right";
        if(input== "111010") return "Large Sector, Eta, PCB 6 Left";
        if(input== "111001") return "Large Sector, Eta, PCB 5 Right";
        if(input== "111000") return "Large Sector, Eta, PCB 5 Left";
        if(input== "111111") return "Large Sector, Eta, PCB 4 Right";
        if(input== "110110") return "Large Sector, Eta, PCB 4 Left";
        if(input== "110101") return "Large Sector, Eta, PCB 3 Right";    
        if(input== "110100") return "Large Sector, Eta, PCB 3 Left";    
        if(input== "110011") return "Large Sector, Eta, PCB 2 Right";
        if(input== "110010") return "Large Sector, Eta, PCB 2 Left";
        if(input== "110001") return "Large Sector, Eta, PCB 1 Right";
        if(input== "110000") return "Large Sector, Eta, PCB 1 Left";   

        if(input== "011111") return "Small Sector, Eta, PCB 8 Right";
        if(input== "011110") return "Small Sector, Eta, PCB 8 Left";
        if(input== "011101") return "Small Sector, Eta, PCB 7 Right";
        if(input== "011100") return "Small Sector, Eta, PCB 7 Left";    
        if(input== "011011") return "Small Sector, Eta, PCB 6 Right";
        if(input== "011010") return "Small Sector, Eta, PCB 6 Left";
        if(input== "011001") return "Small Sector, Eta, PCB 5 Right";
        if(input== "011000") return "Small Sector, Eta, PCB 5 Left";
        if(input== "011111") return "Small Sector, Eta, PCB 4 Right";
        if(input== "010110") return "Small Sector, Eta, PCB 4 Left";
        if(input== "010101") return "Small Sector, Eta, PCB 3 Right";    
        if(input== "010100") return "Small Sector, Eta, PCB 3 Left";    
        if(input== "010011") return "Small Sector, Eta, PCB 2 Right";
        if(input== "010010") return "Small Sector, Eta, PCB 2 Left";
        if(input== "010001") return "Small Sector, Eta, PCB 1 Right";
        if(input== "010000") return "Small Sector, Eta, PCB 1 Left";   


        //Stereo 
        if(input== "101111") return "Large Sector, Stereo, PCB 8 Right";
        if(input== "101110") return "Large Sector, Stereo, PCB 8 Left";
        if(input== "101101") return "Large Sector, Stereo, PCB 7 Right";
        if(input== "101100") return "Large Sector, Stereo, PCB 7 Left";    
        if(input== "101011") return "Large Sector, Stereo, PCB 6 Right";
        if(input== "101010") return "Large Sector, Stereo, PCB 6 Left";
        if(input== "101001") return "Large Sector, Stereo, PCB 5 Right";
        if(input== "101000") return "Large Sector, Stereo, PCB 5 Left";
        if(input== "101111") return "Large Sector, Stereo, PCB 4 Right";
        if(input== "100110") return "Large Sector, Stereo, PCB 4 Left";
        if(input== "100101") return "Large Sector, Stereo, PCB 3 Right";    
        if(input== "100100") return "Large Sector, Stereo, PCB 3 Left";    
        if(input== "100011") return "Large Sector, Stereo, PCB 2 Right";
        if(input== "100010") return "Large Sector, Stereo, PCB 2 Left";
        if(input== "100001") return "Large Sector, Stereo, PCB 1 Right";
        if(input== "010000") return "Large Sector, Stereo, PCB 1 Left";   

        if(input== "001111") return "Small Sector, Stereo, PCB 8 Right";
        if(input== "001110") return "Small Sector, Stereo, PCB 8 Left";
        if(input== "001101") return "Small Sector, Stereo, PCB 7 Right";
        if(input== "001100") return "Small Sector, Stereo, PCB 7 Left";    
        if(input== "001011") return "Small Sector, Stereo, PCB 6 Right";
        if(input== "001010") return "Small Sector, Stereo, PCB 6 Left";
        if(input== "001001") return "Small Sector, Stereo, PCB 5 Right";
        if(input== "001000") return "Small Sector, Stereo, PCB 5 Left";
        if(input== "001111") return "Small Sector, Stereo, PCB 4 Right";
        if(input== "000110") return "Small Sector, Stereo, PCB 4 Left";
        if(input== "000101") return "Small Sector, Stereo, PCB 3 Right";    
        if(input== "000100") return "Small Sector, Stereo, PCB 3 Left";    
        if(input== "000011") return "Small Sector, Stereo, PCB 2 Right";
        if(input== "000010") return "Small Sector, Stereo, PCB 2 Left";
        if(input== "000001") return "Small Sector, Stereo, PCB 1 Right";
        if(input== "000000") return "Small Sector, Stereo, PCB 1 Left";      
}

std::string boolstring( _Bool b ) { return b ? "1" : "0"; }

int main(int ac, const char *av[]) {

    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    std::string description = "This program read MMFE8 PCB location ";

    std::string config_filename;
    std::string fe_name;
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("configfile,c", po::value<std::string>(&config_filename)->
        default_value(base_folder + "integration_config.json"),
        "Configuration file path")
        ("name,n", po::value<std::string>(&fe_name)->
        default_value(""),
        "The name of MMFE8 to read PCB location.\n"
        "If this option is left empty, all front end elements in the config file will be scanned.");


    po::variables_map vm;
    po::store(po::parse_command_line(ac, av, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    nsw::ConfigReader reader1("json://" + config_filename);
    try {
      auto config1 = reader1.readConfig();
    } catch (std::exception & e) {
      std::cout << "Make sure the json is formed correctly. "
                << "Can't read config file due to : " << e.what() << std::endl;
      std::cout << "Exiting..." << std::endl;
      exit(0);
    }

    std::set<std::string> frontend_names;
    if (fe_name != "") {
      frontend_names.emplace(fe_name);
    } else {  // If no name is given, find all elements
      frontend_names = reader1.getAllElementNames();
    }

    std::vector<nsw::FEBConfig> frontend_configs;

    std::cout << "\nFollowing front ends will be readed:\n";
    std::cout <<   "========================================\n";
    for (auto & name : frontend_names) {
      try {
        frontend_configs.emplace_back(reader1.readConfig(name));
        std::cout << name << std::endl;
      } catch (std::exception & e) {
        std::cout << name << " - ERROR: Skipping this FE!"
                  << " - Problem constructing configuration due to : " << e.what() << std::endl;
      }
      // frontend_configs.back().dump();
    }

    std::cout << "\n";

    nsw::ConfigSender cs;

    bool mmfe8id0, mmfe8id1, mmfe8id2, mmfe8id3, boardId, sectorId;
    //bool panelId;

    // boardId :  Eta/Stereo (1/0)
    // sectorId: Large/Small (1/0)
    // panelId : External/Internal (1/0)

    std::string input, compareInput;
    std::string layer, pcb, lerfOrRight;

    bool compareResult;
    int febCounter=0, febCompareCounter=0;

    for (auto & feb : frontend_configs) {
	febCounter++;

    auto opc_ip = feb.getOpcServerIp();
    auto feb_address = feb.getAddress();

    mmfe8id0=cs.readGPIO(opc_ip, feb_address + ".gpio.mmfe8Id0");
    mmfe8id1=cs.readGPIO(opc_ip, feb_address + ".gpio.mmfe8Id1");
    mmfe8id2=cs.readGPIO(opc_ip, feb_address + ".gpio.mmfe8Id2");
    mmfe8id3=cs.readGPIO(opc_ip, feb_address + ".gpio.mmfe8Id3");

    boardId=cs.readGPIO(opc_ip, feb_address + ".gpio.boardId");
    sectorId=cs.readGPIO(opc_ip, feb_address + ".gpio.sectorId");
	//panelId=cs.readGPIO(opc_ip, feb_address + ".gpio.panelId");

    input=boolstring(sectorId) + boolstring(boardId) + boolstring(mmfe8id3) + boolstring(mmfe8id2) + boolstring(mmfe8id1) + boolstring(mmfe8id0);
    
    layer=feb_address.substr(7,1);
    if(layer=="1" || layer=="2")
    	layer="Eta";
    if(layer=="3" || layer=="4")
    	layer="Stereo";   
    pcb=feb_address.substr(9,1);

    lerfOrRight=feb_address.substr(13,1);
    if(lerfOrRight=="L")
    	lerfOrRight="Left";
    if(lerfOrRight=="R")
    	lerfOrRight="Right";

    compareInput="Small Sector, "+layer+", PCB "+pcb+" "+lerfOrRight;

    if(returnMMFE8Location(input)==compareInput)
    {
    	compareResult=true;
    	febCompareCounter++;
    }
    else
    	compareResult=false;

    std::cout << feb_address << " -> " << " (" << input << ") " <<returnMMFE8Location(input) <<" | "<<compareResult<<std::endl;
	}

	std::cout<<"*********** Result: "<<febCompareCounter<<"/"<<febCounter<<std::endl;

}

