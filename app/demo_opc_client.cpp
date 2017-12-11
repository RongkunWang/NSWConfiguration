#include <unistd.h>
#include <time.h>

#include <iostream>

// From UaoForQuasar
#include "UaoClientForOpcUaSca/include/ClientSessionFactory.h"

// Generated  files
#include "UaoClientForOpcUaSca/include/SpiSlave.h"
#include "UaoClientForOpcUaSca/include/AnalogInput.h"

// From: open62541-compat
#include "uaplatformlayer.h"

int main() {
    UaPlatformLayer::init();

    UaClientSdk::UaSession* session = ClientSessionFactory::connect("opc.tcp://pcatlnswdev01.cern.ch:4841");
    if (!session)
        return -1;

    SpiSlave ss(session, UaNodeId("scaFelix1.spi.vmm0", 2));
    AnalogInput mm(session, UaNodeId("scaFelix1.ai.voltage", 2));

    OpcUa_Byte data[] =  { 0x11, 0x12, 0x14, 0x15, 0x11, 0x12, 0x14, 0x15, 0x11, 0x12, 0x14, 0x14 };
    UaByteString bs;
    bs.setByteString(sizeof data, data);

    float temperature;

    while (1) {
        try {
            ss.writeWrite(bs);
            std::cout << "Written value to vmm" << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Epic fail: " <<  e.what() << std::endl;
        }
        usleep(1000000);
        temperature = mm.readValue();
        std::cout << temperature << std::endl;
    }
}

