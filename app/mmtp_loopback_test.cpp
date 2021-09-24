// Sample program to read configuration from db/json

#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <streambuf>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/OpcClient.h"

#include <boost/program_options.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
namespace po = boost::program_options;


int main(int argc, const char *argv[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    std::string description = "This program is for sending/receiving messages from the SCX on the TP.";

    std::string opc_ip;
    std::string slaveAddr0;
    std::string slaveAddr1;
    std::string niter;
    std::string bc_center;
    std::string bc_left;
    std::string bc_right;
    std::string resetBCID;
    std::string artBCID;
    std::string data_source;
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("iterations,n", po::value <std::string>(&niter)->default_value("1"),
            "number of iterations to write the payload (Default: 1)")
        ("windowCenter, bc_c", po::value <std::string>(&bc_center)->default_value("20"),
            "L1a data packet builder BC window center in hex (Default: 20)")
        ("windowLeft, bc_l" , po::value <std::string>(&bc_left)->default_value("08"),
            "left extent of the window in hex (Default: 08)")
        ("windowRight, bc_r" , po::value <std::string>(&bc_right)->default_value("08"),
            "right extent of the window in hex (Default: 08)")
        ("resetBCID" , po::value <std::string>(&resetBCID)->default_value("100"),
            "resetBCID in hex (Default: 100)")
        ("artBCID" , po::value <std::string>(&artBCID)->default_value("10a"),
             "artBCID in hex (Default: 10A)")
        ("slaveAddr0,s", po::value<std::string>(&slaveAddr0)->default_value("NSW_TrigProc_MM.I2C_0.bus0"),
            "slave bus to write to (Default: NSW_TrigProc_MM.I2C_0.bus0)")
        ("slaveAddr1,t", po::value<std::string>(&slaveAddr1)->default_value("NSW_TrigProc_MM_ADDC_EMU.I2C_0.bus0"),
            "slave bus to write to (Default: NSW_TrigProc_MM_ADDC_EMU.I2C_0.bus0)")
        ("opc_ip,o", po::value<std::string>(&opc_ip)->default_value("pcatlnswfelix01.cern.ch:48020"),
            "hostname for OPC server (Default: pcatlnswfelix01.cern.ch:48020)")
        ("data_source,d", po::value<std::string>(&data_source),
            "playback data");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    size_t n_iter = std::stoi(niter);

    // todo: fix this section to actually use the functions we already have...
    //hex string to int
    int artBCID_int;
    std::stringstream ss;
    ss << std::hex << artBCID;
    ss >> artBCID_int;
    int bc_center_int;
    std::stringstream sss;
    sss << std::hex << bc_center;
    sss >> bc_center_int;
    int sum = artBCID_int + bc_center_int;
    //turn the sum into a hex string
    std::stringstream stream;
    stream << std::hex << sum;
    std::string l1aBCID( stream.str() );


    //PRINTING COMMAND LINE INPUT FLAGS
    std::cout << "... reset BCID : " << resetBCID << std::endl;
    std::cout << "... art BCID  : " << artBCID_int << std::endl;
    std::cout << "... l1a BCID : " << l1aBCID << std::endl;
    std::cout << "... bc_center : " << bc_center << std::endl;
    std::cout << "... bc_left : " << bc_left << std::endl;
    std::cout << "... bc_right : " << bc_right << std::endl;
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    nsw::ConfigSender cs;  // in principle the config sender is all that is needed for now

    // Clean up strings
    auto cleanup = [](std::string & string) {
        size_t found = string.find("0x");
        if (found != std::string::npos) string.erase(found, 2);
        found = string.find("0X");
        if (found != std::string::npos) string.erase(found, 2);

        if (string.size()%2) {
            string.insert(0, "0");
        }

        return;
    };

    // Clean up strings
    auto buildEntireMessage = [](std::string & tmp_addr, std::string & tmp_message) {
        std::vector<uint8_t> tmp_data = nsw::hexStringToByteVector(tmp_message,4,true);
        std::vector<uint8_t> tmp_addrVec = nsw::hexStringToByteVector(tmp_addr, 4, true);
        std::vector<uint8_t> tmp_entirePayload(tmp_addrVec);
        tmp_entirePayload.insert(tmp_entirePayload.end(), tmp_data.begin(), tmp_data.end() );
        return tmp_entirePayload;
    };

    auto sendRegData = [&](std::string & tmp_addr, std::string & tmp_message, int targetSCA = 0) {
        cleanup(tmp_addr);
        cleanup(tmp_message);
        std::vector<uint8_t> entirePayload;
        entirePayload = buildEntireMessage(tmp_addr, tmp_message);
        if (targetSCA == 0) cs.sendI2cRaw(opc_ip, slaveAddr0, entirePayload.data(), entirePayload.size() );
        else if (targetSCA == 1) cs.sendI2cRaw(opc_ip, slaveAddr1, entirePayload.data(), entirePayload.size() );
    };

    std::vector<std::pair<std::string, std::string> > header = {
        {"01", "01"},  // disables the ADDC emulator output
        {"11", "000000"+ bc_center},  // L1a data packet builder options. only do this once. // sets center of BC window
        {"12", "000000"+ bc_left},  // left extent of window
        {"13", "000000"+ bc_right},  // right extent of window
        {"10", "000000FF"},
        {"10", "00000000"},
    };

    // Read in input data file

    std::ifstream inputData(data_source);


    std::string line;
    std::vector<std::string> strs;

    // Process input data file into vector object

    std::vector<std::vector<std::string> > messageList;
    std::vector<std::string> tmpVec;
    while (std::getline(inputData, line)) {
        std::cout << line << std::endl;
        tmpVec.clear();
        boost::split(strs,line,boost::is_any_of("\t ,"),boost::token_compress_on);
        if (strs.size() == 2) {
            tmpVec.push_back("0");
            tmpVec.push_back(strs.at(0));
            tmpVec.push_back(strs.at(1));
        } else if (strs.size() == 3){
            // tmpVec.push_back(strs.at(0));
            std::string tmpString = "0";
            tmpVec.push_back(tmpString);
            tmpVec.push_back(strs.at(1));
            tmpVec.push_back(strs.at(2));
        } else {
            continue;
        }
        messageList.push_back(tmpVec);
    }

    // Sending header messages -- Initialization

    std::vector<uint8_t> entirePayload;
    std::cout << "... writing initialization of L1a packet builder options" << std::endl;
    for (auto header_ele : header){
        sendRegData(header_ele.first, header_ele.second);
    }

    // Sending messages from input data file

    for (size_t iter =0 ; iter < n_iter; iter++) {
        std::cout << "... sending loopback data iteration: " << iter << std::endl;
        for (auto packet : messageList) {
            std::cout << "... ... Addr: " << packet.at(1) << ", Data: " << packet.at(2) << std::endl;
            sendRegData(packet.at(1), packet.at(2), std::stoi(packet.at(0)) );
        }

    }

    return 0;
}
