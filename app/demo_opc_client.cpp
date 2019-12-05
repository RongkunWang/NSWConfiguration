#include <unistd.h>
#include <time.h>

#include <string>
#include <iostream>

// From UaoForQuasar
#include "UaoClientForOpcUaSca/include/ClientSessionFactory.h"

// Generated  files
#include "UaoClientForOpcUaSca/include/SpiSlave.h"
#include "UaoClientForOpcUaSca/include/AnalogInput.h"
#include "UaoClientForOpcUaSca/include/DigitalIO.h"
#include "UaoClientForOpcUaSca/include/I2cSlave.h"

// From: open62541-compat
// #include "uaplatformlayer.h"

int main() {
    // UaPlatformLayer::init();

    // UaClientSdk::UaSession* session = ClientSessionFactory::connect("opc.tcp://pcatlnswdev01.cern.ch:4841");

    UaClientSdk::UaSession* session = ClientSessionFactory::connect("opc.tcp://pcatlnswfelix01.cern.ch:4841");
    if (!session)
        return -1;

    std::string sca = "SCA on Felix (elink 0x80)";

    auto vmm = sca + ".spi.vmm0";
    SpiSlave ss(session, UaNodeId(vmm.c_str(), 2));

    auto aivoltage = sca + ".ai.voltage";
    AnalogInput mm(session, UaNodeId(aivoltage.c_str(), 2));

    auto tds_reg0name = sca + ".TDS.Register 0";
    I2cSlave tds_reg0(session, UaNodeId(tds_reg0name.c_str(), 2));

    auto gpio_node = sca + ".gpio.led2";

    DigitalIO gpio(session, UaNodeId(gpio_node.c_str(), 2));

    OpcUa_Byte data[] =  { 0x11, 0x12, 0x14, 0x15, 0x11, 0x12, 0x14, 0x15, 0x11, 0x12, 0x14, 0x14 };
    UaByteString bs;
    bs.setByteString(sizeof data, data);

    OpcUa_Byte data2[] =  { 0x1, 0x2, 0x3, 0x4};
    UaByteString bs2;
    bs2.setByteString(sizeof data2, data2);

    double temperature;
    bool gpio_value = true;

    while (1) {
        try {
            ss.writeWrite(bs);
            std::cout << "Written value to vmm" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Epic fail: " <<  e.what() << std::endl;
        }
        // temperature = mm.readValue();
        // std::cout << temperature << std::endl;

        tds_reg0.writeValue(bs2);
        std::cout << "Written value to TDS Reg 0 " << std::endl;
        usleep(1000000);
        auto bsread = tds_reg0.readValue();
        auto byteread = bsread.data();
        auto length = bsread.length();
        std::cout << "Read back, length: " << length << " data[0]: "
                  << static_cast<unsigned>(byteread[0]) << std::endl;

        try {
            gpio_value = !gpio_value;
            gpio.writeValue(gpio_value);
            std::cout << "Written value: " << gpio_value << " to node: " << gpio_node << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Can't write GPIO: " <<  e.what() << std::endl;
        }

        auto gpio_read = gpio.readValue();
        std::cout << "Reading back gpio: " << gpio_read << std::endl;

        usleep(1000000);
    }
}

