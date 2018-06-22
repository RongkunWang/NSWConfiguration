// Sample program to read configuration from db/json

#include <iostream>
#include <string>
#include <vector>

#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/ROCConfig.h"
#include "NSWConfiguration/OpcClient.h"

int main(int argc, const char *argv[]) 
{

    // not sure if/what is needed for ADDC
    std::string base_folder = "/afs/cern.ch/user/c/cyildiz/public/nsw-work/work2/NSWConfiguration/data/";
    nsw::ConfigReader reader1("json://" + base_folder + "integration_config.json"); //here is OPC address and SCA address that can be useful
    auto config1 = reader1.readConfig();
    write_json(std::cout, config1);
    // write_xml(std::cout, config1);

    // now we start with (preliminary) ADDC part

    nsw::ConfigSender cs; // in principle the config sender is all that is needed for now

    auto addcconfig0 = reader1.readConfig("A01.ROC_L01_M01"); // THIS NEEDS CHANGE!!! fine for now
    nsw::ROCConfig addc0(addcconfig0);
    addc0.dump();

    // Seqence of actions to send ADDC config
    auto opc_ip = addc0.getOpcServerIp();
    auto sca_addc_address = addc0.getAddress();

    std::string sca_address = "SCA on ADDCv3";
    // std::string sca_roc_address_digital = sca_address + ".mmfe8RocCoreDigital";

    // 1. Reset GBTx0 and GBTx1
    cs.sendGPIO(opc_ip, sca_address + ".gpio.gbtx0Rstn", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.gbtx1Rstn", 0);

    sleep(1);

    // 2. and 3. set GBTx0 and GBTx1 registers (from config file GBTX_cfg_reg_extclk_rev02_110117.txt)

    // vector from GBTX_cfg_reg_extclk_rev02_110117.txt
    std::vector<uint8_t> ADDCConfigurationData {
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x23,0x03,0x33,0x03,0x03,0x00,0x00,0x00,
            0x1f,0x03,0x7f,0x28,0x00,0x15,0x15,0x15,0x66,0x00,0x0d,0x42,
            0x00,0x0f,0x04,0x08,0x00,0x20,0x00,0x00,0x00,0x00,0x15,0x15,
            0x15,0x00,0x00,0x00,0x07,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x3f,0xdd,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x11,0x11,
            0x00,0x00,0x00,0x3f,0xdd,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x11,0x11,
            0x00,0x00,0x00,0x3f,0xdd,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x11,0x11,
            0x00,0x00,0x00,0x3f,0xdd,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x11,0x11,
            0x00,0x00,0x00,0x3f,0xdd,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x11,0x11,0x11,
            0x00,0x00,0x00,0x3f,0xdd,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,
            0x00,0x00,0x00,0x3f,0xdd,0x0d,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,
            0x00,0x00,0x00,0x00,0x00,0x70,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x3f,0x3f,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x07,
            0x00,0x00,0x08,0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x88,0x88,0x08,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0xff,0xff,0xff,0x40,0x40,0x40,0x2a,
            0x2a,0x2a,0x00,0x00,0xff,0xff,0xff,0x40,0x40,0x40,0x2a,0x2a,
            0x2a,0x4e,0x4e,0x4e,0xaa,0x0a,0x07,0x00,0x11,0x11,0x11,0x11,
            0x11,0x22,0x22,0x00,0x00,0x00,0x00,0x00,0x08,0x00,0x00,0x08,
            0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x08,
            0x00,0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
            0x00,0x00,0x00,0x00,0x00,0xaa,0x01,0x00,0x01
    };

    size_t gbtx_size = 3;
    uint8_t gbtx_data[] = {0x0,0x0,0x0}; // 2 for address (i), 1 for data

    size_t art_size = 2;
    uint8_t art_data[] = {0x0,0x0}; // 1 for address (i), 1 for data

    for (uint i=0; i<ADDCConfigurationData.size(); i++)
    {
        gbtx_data[0] = ((uint8_t) ((i) >> 8));
        gbtx_data[1] = ((uint8_t) ((i) & 0xff));
        gbtx_data[2] = ADDCConfigurationData[i];
        cs.sendI2cRaw(opc_ip, sca_address + ".gbtx0.gbtx0", gbtx_data, gbtx_size);
        cs.sendI2cRaw(opc_ip, sca_address + ".gbtx1.gbtx1", gbtx_data, gbtx_size);
    }


    // 4. Reset ART0 and ART1
    cs.sendGPIO(opc_ip, sca_address + ".gpio.art0SRstn", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.art1SRstn", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.art0CRstn", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.art1CRstn", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.art0Rstn", 0);
    cs.sendGPIO(opc_ip, sca_address + ".gpio.art1Rstn", 0);

    // 6.a set the ART0 ASIC to provide continuous “010101…” 
    std::vector<uint8_t> ARTregisters {21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 2}; 
    std::vector<uint8_t> ARTregistervalues {0xff, 0x3f, 0x00, 0xf0, 0xff, 0x03, 0x00, 0xff, 0x3f, 0x00, 0xf0, 0xff, 0x03, 0x00, 0x80};

    for (uint i=0; i<ARTregisters.size(); i++) 
    {
        art_data[0] = ARTregisters[i];
        art_data[1] = ARTregistervalues[i];
        cs.sendI2cRaw(opc_ip, sca_address + ".art0Core", art_data, art_size);
    }

    // 6.b Set the GBTx0 to training mode
    gbtx_data[0] = 0;
    gbtx_data[1] = 62;
    gbtx_data[2] = 0x15;
    cs.sendI2cRaw(opc_ip, sca_address + ".gbtx0.gbtx0", gbtx_data, gbtx_size);

    // 6.c Enable the GBTx training for eport
    std::vector<uint8_t> GBTxregisters {78,79,80,102,103,104,126,127,128,150,151,152,174,175,176,198,199,200,222,223,224};
    for (uint i=0; i<GBTxregisters.size(); i++) 
    {
        gbtx_data[0] = ((uint8_t) ((GBTxregisters[i]) >> 8)); 
        gbtx_data[1] = ((uint8_t) ((GBTxregisters[i]) & 0xff));
        gbtx_data[2] = 0xff;
        cs.sendI2cRaw(opc_ip, sca_address + ".gbtx0.gbtx0", gbtx_data, gbtx_size);
    }

    // 6.d fix the e-port phase
    sleep(1);
    for (uint i=0; i<GBTxregisters.size(); i++) 
    {
        gbtx_data[0] = ((uint8_t) ((GBTxregisters[i]) >> 8)); 
        gbtx_data[1] = ((uint8_t) ((GBTxregisters[i]) & 0xff));
        gbtx_data[2] = 0x00;
        cs.sendI2cRaw(opc_ip, sca_address + ".gbtx0.gbtx0", gbtx_data, gbtx_size);
    }


    // 7.a repeat above for ART1 and GBTx0: set the ART1 ASIC to provide continuous “010101…” 
    for (uint i=0; i<ARTregisters.size(); i++)
    {
        art_data[0] = ARTregisters[i];
        art_data[1] = ARTregistervalues[i];
        cs.sendI2cRaw(opc_ip, sca_address + ".art1Core", art_data, art_size);
    }

    // 7.b Set the GBTx0 to training mode
    gbtx_data[0] = 0;
    gbtx_data[1] = 62;
    gbtx_data[2] = 0x15;
    cs.sendI2cRaw(opc_ip, sca_address + ".gbtx1.gbtx1", gbtx_data, gbtx_size);

    // 7.c Enable the GBTx training for eport
    for (uint i=0; i<GBTxregisters.size(); i++) 
    {
        gbtx_data[0] = ((uint8_t) ((GBTxregisters[i]) >> 8)); 
        gbtx_data[1] = ((uint8_t) ((GBTxregisters[i]) & 0xff));
        gbtx_data[2] = 0xff;
        cs.sendI2cRaw(opc_ip, sca_address + ".gbtx1.gbtx1", gbtx_data, gbtx_size);
    }

    // 7.d fix the e-port phase
    sleep(1);
    for (uint i=0; i<GBTxregisters.size(); i++) 
    {
        gbtx_data[0] = ((uint8_t) ((GBTxregisters[i]) >> 8)); 
        gbtx_data[1] = ((uint8_t) ((GBTxregisters[i]) & 0xff));
        gbtx_data[2] = 0x00;
        cs.sendI2cRaw(opc_ip, sca_address + ".gbtx1.gbtx1", gbtx_data, gbtx_size);
    }

    // 8. bit order alignment
    // 8.a 
    art_data[0] = 2;
    art_data[1] = 0x10;
    cs.sendI2cRaw(opc_ip, sca_address + ".art0Core", art_data, art_size);

    // 8.b Mask off the ART input channels to avoid interferences
    std::vector<uint8_t> ARTCoreregisters {9, 10, 11, 12};
    for (uint i=0; i<ARTCoreregisters.size(); i++)
    {
        art_data[0] = ARTCoreregisters[i];
        art_data[1] = 0xff;
        cs.sendI2cRaw(opc_ip, sca_address + ".art0core", art_data, art_size);
    }

    // 8.c Check the received ART data on the Trigger processor end, scan the phase of the GBTx0 160M clock. 
    // To do that, write the phase value to the GBTx0 register 8, 
    // check the output 112 bit data until the 12-bit BCID counter be found in bit 107~96 and the rest bits are fixed. --> ?????

    // loop and test values between 0 and 255 to be written to GBTx0 register 8
    // sleep for some time

    // rest: manually...

    for (uint i=0; i<256; i++) 
    {
        gbtx_data[0] = 0; 
        gbtx_data[1] = 8;
        gbtx_data[2] = i;
        cs.sendI2cRaw(opc_ip, sca_address + ".gbtx0.gbtx0", gbtx_data, gbtx_size);
        sleep(2);
    }


    // 9. repeat bit order alignment on ART1/GBTx1
    // 9.a 
    art_data[0] = 2;
    art_data[1] = 0x10;
    cs.sendI2cRaw(opc_ip, sca_address + ".art1Core", art_data, art_size);

    // 9.b Mask off the ART input channels to avoid interferences
    for (uint i=0; i<ARTCoreregisters.size(); i++)
    {
        art_data[0] = ARTCoreregisters[i];
        art_data[1] = 0xff;
        cs.sendI2cRaw(opc_ip, sca_address + ".art1core", art_data, art_size);
    }

    // 9.c Check the received ART data on the Trigger processor end, scan the phase of the GBTx1 160M clock. 
    // To do that, write the phase value to the GBTx1 register 8, 
    // check the output 112 bit data until the 12-bit BCID counter be found in bit 107~96 and the rest bits are fixed. --> ?????

    for (uint i=0; i<256; i++) 
    {
        gbtx_data[0] = 0; 
        gbtx_data[1] = 8;
        gbtx_data[2] = i;
        cs.sendI2cRaw(opc_ip, sca_address + ".gbtx1.gbtx1", gbtx_data, gbtx_size);
        sleep(2);
    }

    // 10.
    for (uint i=0; i<ARTCoreregisters.size(); i++)
    {
        art_data[0] = ARTCoreregisters[i]; 
        art_data[1] = 0x00;
        cs.sendI2cRaw(opc_ip, sca_address + ".art0core", art_data, art_size);
        cs.sendI2cRaw(opc_ip, sca_address + ".art1core", art_data, art_size);
    }

    // anything else needed?

    // Yes: bits for BCR counters
    // 11 
    art_data[0] = 14;
    art_data[1] = 0xF;
    cs.sendI2cRaw(opc_ip, sca_address + ".art0Core", art_data, art_size);
    cs.sendI2cRaw(opc_ip, sca_address + ".art1Core", art_data, art_size);
    art_data[0] = 15;
    art_data[1] = 0xFF;
    cs.sendI2cRaw(opc_ip, sca_address + ".art0Core", art_data, art_size);
    cs.sendI2cRaw(opc_ip, sca_address + ".art1Core", art_data, art_size);

    return 0;

}
