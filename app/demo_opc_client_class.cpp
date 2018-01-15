#include <unistd.h>
#include <time.h>

#include <iostream>
#include <vector>
#include <string>

#include "NSWConfiguration/OpcClient.h"

int main() {
    auto client = nsw::OpcClient("pcatlnswfelix01.cern.ch:4841");

    std::vector<uint8_t> vmmbytes = {0x11, 0x12, 0x14, 0x15, 0x11, 0x12, 0x14, 0x15, 0x11, 0x12, 0x14, 0x14};
    std::vector<uint8_t> tdsbytes = {0x11, 0x12, 0x13, 0x14};
    bool gpio_value = true;

    std::string sca = "SCA on Felix (elink 0x80)";
    std::string vmmnode = sca + ".spi.vmm0";
    std::string tdsnode = sca + ".TDS.Register 0";
    std::string gpionode = sca + ".gpio.led2";

    while (1) {
        client.writeSpiSlave(vmmnode, vmmbytes);
        client.writeI2c(tdsnode, tdsbytes);

        gpio_value = !gpio_value;
        client.writeGPIO(gpionode, gpio_value);
        auto gpio_read = client.readGPIO(gpionode);
        std::cout << "Reading back gpio: " << gpio_read << std::endl;

        auto tdsbytes_read = client.readI2c(tdsnode);
        std::cout << "Reading back the bytes, size: "  << tdsbytes_read.size() << ", values: ";
        for (auto byte : tdsbytes_read) {
            std::cout << static_cast<unsigned>(byte) << " ";
        }
        std::cout << "\n";
        usleep(1000000);
    }
}
