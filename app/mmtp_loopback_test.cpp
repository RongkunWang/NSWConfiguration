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
    bool readMode;
    bool writeMode;
    std::string slaveAddr;
    std::string regAddr;
    std::string  niter;
    std::string message;
    std::string bc_center; 
    std::string bc_left; 
    std::string bc_right; 
    std::string BCID;
    std::string bcid_addr;
    po::options_description desc(description);
    desc.add_options()
        ("help,h", "produce help message")
        ("read,R", po::bool_switch(&readMode)->default_value(false),
            "read value at address(Default: False)")
        ("write,W", po::bool_switch(&writeMode)->default_value(false),
            "write value at address(Default: False)")
	("iterations,C", po::value <std::string>(&niter)->default_value("1"),
	 "number of iterations to write the payload (Default: 1)")
 	("BCw_center, bc_c", po::value <std::string>(&bc_center)->default_value("20"),
         " #L1a data packet builder options. BC window center ")
	("BC_left, bc_l" , po::value <std::string>(&bc_left)->default_value("08"),
         "left extent of the window (Default: 08)")
	("BC_right, bc_r" , po::value <std::string>(&bc_right)->default_value("08"),
         "right extent of the window (Default: 08)")
	("BCID, bcid" , po::value <std::string>(&BCID)->default_value("100"),
         "BCID (Default: 100)")
	("BCID address, bcid_addr" , po::value <std::string>(&bcid_addr)->default_value("10a"),
         "BCID address (where the data is being sent on) (Default: 10A)")
        ("slaveAddr,s", po::value<std::string>(&slaveAddr)->default_value("NSW_TrigProc_STGC.I2C_0.bus0"),
            "slave bus to write to(Default: NSW_TrigProc_STGC.I2C_0.bus0)")
        ("regAddr,r", po::value<std::string>(&regAddr)->default_value("0x0000"),
            "register to read from/write to(Default: 0x2AD)")
        ("opc_ip,o", po::value<std::string>(&opc_ip)->default_value("pcatlnswfelix01.cern.ch:48020"),
            "hostname for OPC server");


    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    int n_iter = std::stoi(niter);
    //hex string to int 
    int bcId_addr;
    std::stringstream ss;
    ss << std::hex << bcid_addr;
    ss >> bcId_addr;    
    int bc_ctr;
    std::stringstream sss;
    sss << std::hex << bc_center;
    sss >> bc_ctr;
     int sum = bcId_addr + bc_ctr;
    //turn the sum into a hex string
    std::stringstream stream;
    stream << std::hex << sum;
    std::string l1a_addr( stream.str() );


    //PRINTING COMMAND LINE INPUT FLAGS
    std::cout << " bcid address  : " << bcId_addr << std::endl;
    std::cout << " sum in decimal : " << sum << std::endl;
    std::cout << " sum in hex string : " << l1a_addr << std::endl;
    std::cout << " bc_center : in string " << bc_center << std::endl;
    std::cout << " bc_left : in string " << bc_left << std::endl;
    std::cout << " bc_right : in string " << bc_right << std::endl;
    std::cout << " BCID : in string " << BCID << std::endl;
    if (vm.count("help")) {
        std::cout << desc << "\n";
        return 1;
    }

    nsw::ConfigSender cs;  // in principle the config sender is all that is needed for now

    // auto addcconfig0 = reader1.readConfig("A01.ROC_L01_M01"); // THIS NEEDS CHANGE!!! fine for now
    // nsw::TPConfig tp(addcconfig0);
    // tp.dump();

    // Seqence of actions to send ADDC config
    // auto opc_ip = tp.getOpcServerIp();
    // auto sca_tp_address = tp.getAddress();

    int test = 0;

    std::vector<uint8_t> data;
    std::vector<uint8_t> outdata;

    std::vector<uint8_t> regAddrVec;

    unsigned long registerAddressValue = std::strtoul(regAddr.data(), 0, 16);
    assert(registerAddressValue < 0x0400);

    // Clean up strings
    auto cleanup = [](std::string & string) {
        size_t found = string.find("0x");
        if (found != std::string::npos) string.erase(found, 2);
        found = string.find("0X");
        if (found != std::string::npos) string.erase(found, 2);
        return;
    };
	

    // Zero pad the hex so that it fits into a round number of 8-bit bytes.
    if (regAddr.size()%2) {
        regAddr.insert(0, "0");
    }
    if (message.size()%2) {
        message.insert(0, "0");
    }
   
    // w,0,01,01, # disables the ADDC emulator output
    // w,0,11,00000020, #L1a data packet builder options. only do this once. #center
    // w,0,12,00000008 #left size of window
    // w,0,13,00000008 #end of window
    // w,0,10,000000FF
    // w,0,10,00000000
    // w,0,20,0008A100, # 20 is an offset for the fiber location. loading GBT data for fiber 0. 0008 represents the ECR
    // w,0,20,00000000
    // w,0,20,00000000
    // w,0,20,00000000
    // w,0,21,0008A100, # same for fiber 1
    // w,0,21,00000000
    // w,0,21,00000000
    // w,0,21,00000000
    // w,0,20,0000A10A, # Actual data
    // w,0,20,00010101
    // w,0,20,01F00000
    // w,0,20,00104104
    // w,0,21,0000A10A
    // w,0,21,00010101
    // w,0,21,01F00000
    // w,0,21,00104104
    // w,0,20,0004A12A, # 0004 says L1a and the next two bytes are the BCID
    // w,0,20,00000000
    // w,0,20,00000000
    // w,0,20,00000000
    // w,0,21,0004A12A
    // w,0,21,00000000
    // w,0,21,00000000
    // w,0,21,00000000
    // w,0,01,03 # turns on ADDC emulator enable bit
    // r,0,01,

    // Clean up strings
    auto buildEntireMessage = [](std::string & tmp_addr, std::string & tmp_message) {
        std::vector<uint8_t> tmp_data = nsw::hexStringToByteVector(tmp_message,4,true);
        std::vector<uint8_t> tmp_addrVec = nsw::hexStringToByteVector(tmp_addr, 4, true);
        std::vector<uint8_t> tmp_entirePayload(tmp_addrVec);
        tmp_entirePayload.insert(tmp_entirePayload.end(), tmp_data.begin(), tmp_data.end() );
        return tmp_entirePayload;
    };

    std::vector<std::pair<std::string, std::string> > header = {
        {"11", "000000"+ bc_center},  // L1a data packet builder options. only do this once. // sets center of BC window
        {"12", "000000"+ bc_left},  // left extent of window
        {"13", "000000"+ bc_right},  // right extent of window
        {"10", "000000FF"},
        {"10", "00000000"}
    };
    std::vector<std::pair<std::string, std::string> > messageList = {
        // This is the setup of the test including the size of the BC window to consider
        {"01", "01"},  // disables the ADDC emulator output
       
        // Represents the event counter reset (ECR) for the two fibers

        {"20", "0008A"+BCID},  // 20 is an offset for the fiber location. loading GBT data for fiber 0. 0008 represents the ECR
            // This is apparently sending at BCID 100
        {"20", "00000000"},
        {"20", "00000000"},
        {"20", "00000000"},
        {"21", "0008A"+BCID},  // same for fiber 1
            // This is apparently sending at BCID 100
        {"21", "00000000"},
        {"21", "00000000"},
        {"21", "00000000"},

        // Actual simulated ADDC data to push out of the Tx

        {"20", "0000A10A"},  // Actual data
        {"20", "00010101"},
        {"20", "01F00000"},
        {"20", "00104104"},
        {"21", "0000A10A"},
        {"21", "00010101"},
        {"21", "01F00000"},
        {"21", "00104104"},

        {"20", "0004A"+ l1a_addr},  // 0004 says L1a and the next two bytes are the BCID
            // This is sending an L1a at 12A, so 0x20 after the data. 0x20 is the center of the readout window.
        {"20", "00000000"},
        {"20", "00000000"},
        {"20", "00000000"},
        {"21", "0004A"+l1a_addr},  // same here
        {"21", "00000000"},
        {"21", "00000000"},
        {"21", "00000000"},

        {"01", "03" } // turns on ADDC emulator enable bit
    };
	
    std::vector<uint8_t> entirePayload;
    std::vector <uint8_t> entireLine;
    for (auto header_ele : header){
	std::cout << " writing header once" << std::endl;
    	cleanup(header_ele.first); cleanup(header_ele.second);
	if (header_ele.first.size()%2) {
            header_ele.first.insert(0, "0");
        }
        if (header_ele.second.size()%2) {
            header_ele.second.insert(0, "0");
        }
	entireLine = buildEntireMessage(header_ele.first, header_ele.second);
        cs.sendI2cRaw(opc_ip, slaveAddr, entireLine.data(), entireLine.size() ); 	
    }

    for (int iter =0 ; iter< n_iter; iter ++) { 
    	for (auto packet : messageList) {
		std::cout << " writing the " <<iter<<"th "<<"packet"<< std::endl;
        	cleanup(packet.first); cleanup(packet.second);
        	if (packet.first.size()%2) {
            		packet.first.insert(0, "0");
        }
        	if (packet.second.size()%2) {
            		packet.second.insert(0, "0");
        }
        	entirePayload = buildEntireMessage(packet.first, packet.second);
        	cs.sendI2cRaw(opc_ip, slaveAddr, entirePayload.data(), entirePayload.size() );
    	}

	   }

    regAddrVec = nsw::hexStringToByteVector("01", 4, true);
    cs.readI2cAtAddress(opc_ip, slaveAddr, regAddrVec.data(), regAddrVec.size(), 4);

    // regAddrVec = nsw::hexStringToByteVector(regAddr, 4, true);
    // std::cout << "... Register address array: ";
    // for (uint i=0; i < regAddrVec.size(); i++) {
    //     std::cout << std::hex << unsigned(regAddrVec[i]) << " ";
    // }
    // std::cout << std::endl;

    // if (readMode) {
    //     if (test == 0) {
    //         std::cout << "... Testing the readout of a register via I2c..." << std::endl;
    //         outdata = cs.readI2cAtAddress(opc_ip, slaveAddr, regAddrVec.data(), regAddrVec.size(), 4);
    //         for (uint i=0; i < outdata.size(); i++) {
    //             std::cout << std::hex << unsigned(outdata[i]) << std::endl;
    //         }
    //     }
    // }

    // if (writeMode) {
    //     // Example
    //     // data = {0x00, 0x00, 0x00, 0x0a, 0xff, 0xff, 0xff, 0xff}; //8
    //     //             [0] first byte of address
    //     //             [1] second byte of address
    //     //             [4,5,6,7] last..first bytes of message
    //     // Already have the first four bytes of vector stored as regAddrVec.
    //     // Just need to encode the message.
    //     data = nsw::hexStringToByteVector(message,4,true);
    //     std::cout << "... Message to write: " << message << std::endl;
    //     std::cout << "... ... in a byte vector: ";
    //     for (uint i=0; i < data.size(); i++) {
    //         std::cout << std::hex << unsigned(data[i]) << " ";
    //     }
    //     std::cout << std::endl;

    //     std::vector<uint8_t> entirePayload(regAddrVec);
    //     entirePayload.insert(entirePayload.end(), data.begin(), data.end() );

    //     for (uint i=0; i < entirePayload.size(); i++) {
    //         std::cout << std::hex << unsigned(entirePayload[i]) << " ";
    //     }

    //     if (test == 0) {
    //         cs.sendI2cRaw(opc_ip, slaveAddr, entirePayload.data(), entirePayload.size() );
    //         if (readMode) {
    //             std::cout << "... Reading data back from register: " << std::endl;
    //             outdata = cs.readI2cAtAddress(opc_ip, slaveAddr, regAddrVec.data(), regAddrVec.size(), 4);
    //             for (uint i=0; i < outdata.size(); i++) {
    //                 std::cout << std::hex << unsigned(outdata[i]) << std::endl;
    //             }
    //         }
    //     }
    // }

    std::cout << "... Done with TP Comm test" << std::endl;

    return 0;
}
