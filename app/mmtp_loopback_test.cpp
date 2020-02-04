// Sample program to read configuration from db/json

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/OpcClient.h"

#include "boost/program_options.hpp"
namespace po = boost::program_options;


int main(int argc, const char *argv[]) {
    std::string base_folder = "/eos/atlas/atlascerngroupdisk/det-nsw/sw/configuration/config_files/";

    std::string description = "This program is for sending/receiving messages from the SCX on the TP.";

    std::string opc_ip;
    std::string slaveAddr;
    std::string niter;
    std::string bc_center;
    std::string bc_left;
    std::string bc_right;
    std::string resetBCID;
    std::string artBCID;
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
        ("slaveAddr,s", po::value<std::string>(&slaveAddr)->default_value("NSW_TrigProc_STGC.I2C_0.bus0"),
            "slave bus to write to (Default: NSW_TrigProc_STGC.I2C_0.bus0)")
        ("opc_ip,o", po::value<std::string>(&opc_ip)->default_value("pcatlnswfelix01.cern.ch:48020"),
            "hostname for OPC server (Default: pcatlnswfelix01.cern.ch:48020)");


    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    int n_iter = std::stoi(niter);

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

    std::vector<std::pair<std::string, std::string> > header = {
        {"01", "01"},  // disables the ADDC emulator output
        {"11", "000000"+ bc_center},  // L1a data packet builder options. only do this once. // sets center of BC window
        {"12", "000000"+ bc_left},  // left extent of window
        {"13", "000000"+ bc_right},  // right extent of window
        {"10", "000000FF"},
        {"10", "00000000"},
    };

    std::vector<std::pair<std::string, std::string> > messageList = {
        // This is the setup of the test including the size of the BC window to consider
        {"01", "01"},  // disables the ADDC emulator output

        // Represents the event counter reset (ECR) for the two fibers

        {"20", "0008A"+resetBCID},  // 20 is an offset for the fiber location. loading GBT data for fiber 0. 0008 represents the ECR
        {"20", "00000000"},
        {"20", "00000000"},
        {"20", "00000000"},
        {"21", "0008A"+resetBCID},  // same for fiber 1
        {"21", "00000000"},
        {"21", "00000000"},
        {"21", "00000000"},

        // Actual simulated ADDC data to push out of the Tx

        // The data is...
        // Header: 0b1010 = A
        // BCID:   12 bits
        // Error Flags: 8 bits
        // Hit List: 32 bits
        // ART Data Parity: 8 bits
        // 8xART Data: 6 bits each

        {"20", "0000A"+artBCID},  // Actual data. 0, header, BCID
        {"20", "00010101"},  // 00 flags, hit list 01
        {"20", "01F00000"},  // 01 hit list continued, F0 is data parity
        {"20", "00104104"},  // actual data is 0000 0010 4104
        {"21", "0000A"+artBCID},
        {"21", "00010101"},
        {"21", "01F00000"},
        {"21", "00104104"},

        {"20", "0004A"+l1aBCID},  // 0004 says L1a and the next two bytes are the BCID
        {"20", "00000000"},
        {"20", "00000000"},
        {"20", "00000000"},
        {"21", "0004A"+l1aBCID},  // same here
        {"21", "00000000"},
        {"21", "00000000"},
        {"21", "00000000"},

        {"01", "03" } // turns on ADDC emulator enable bit
    };

    std::vector<uint8_t> entirePayload;
    for (auto header_ele : header){
        std::cout << "... writing initialization of L1a packet builder options" << std::endl;
        cleanup(header_ele.first);
        cleanup(header_ele.second);
        entirePayload = buildEntireMessage(header_ele.first, header_ele.second);
        cs.sendI2cRaw(opc_ip, slaveAddr, entirePayload.data(), entirePayload.size() );
    }

    for (size_t iter =0 ; iter < n_iter; iter++) {
        for (auto packet : messageList) {
            std::cout << "... sending loopback data iteration: " << iter << std::endl;
            cleanup(packet.first);
            cleanup(packet.second);
            entirePayload = buildEntireMessage(packet.first, packet.second);
            cs.sendI2cRaw(opc_ip, slaveAddr, entirePayload.data(), entirePayload.size() );
        }

    }

    std::vector<uint8_t> regAddrVec = nsw::hexStringToByteVector("01", 4, true);
    cs.readI2cAtAddress(opc_ip, slaveAddr, regAddrVec.data(), regAddrVec.size(), 4);

    std::cout << "... Done with MMTP Loopback test" << std::endl;

    return 0;
}
