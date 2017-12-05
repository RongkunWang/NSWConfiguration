#include <unistd.h>
#include <time.h>

#include <iostream>
#include <vector>

#include "NSWConfiguration/OpcClient.h"

int main() {
    auto client = nsw::OpcClient("pcatlnswdev01.cern.ch:4841");

    std::vector<uint8_t> vmmconfig = {0x11, 0x12, 0x14, 0x15, 0x11, 0x12, 0x14, 0x15, 0x11, 0x12, 0x14, 0x14};

    while (1) {
        client.writeSpiSlave("scaFelix1.spi.vmm0", vmmconfig);
        usleep(1000000);
    }
}

