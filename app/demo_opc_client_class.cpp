#include <unistd.h>
#include <time.h>

#include <iostream>
#include <vector>
#include <string>

#include "NSWConfiguration/OpcClient.h"

int main() {

    std::string server_ip = "pcatlnswfelix01.cern.ch:48020";

    auto client = std::make_unique<nsw::OpcClient>(server_ip);
   /*
    std::vector<uint8_t> vmmbytes = {0x11, 0x12, 0x14, 0x15, 0x11, 0x12, 0x14, 0x15, 0x11, 0x12, 0x14, 0x14};
    std::vector<uint8_t> tdsbytes = {0x11, 0x12, 0x13, 0x14};
    bool gpio_value = true;

    std::string sca = "SCA on Felix (elink 0x80)";
    std::string vmmnode = sca + ".spi.vmm0";
    std::string tdsnode = sca + ".TDS.Register 0";
    std::string gpionode = sca + ".gpio.led2";

    while (1) {
        client->writeSpiSlave(vmmnode, vmmbytes);
        client->writeI2c(tdsnode, tdsbytes);

        gpio_value = !gpio_value;
        client->writeGPIO(gpionode, gpio_value);
        auto gpio_read = client->readGPIO(gpionode);
        std::cout << "Reading back gpio: " << gpio_read << std::endl;

        auto tdsbytes_read = client->readI2c(tdsnode);
        std::cout << "Reading back the bytes, size: "  << tdsbytes_read.size() << ", values: ";
        for (auto byte : tdsbytes_read) {
            std::cout << static_cast<unsigned>(byte) << " ";
        }
        std::cout << "\n";
        usleep(1000000);
    }
    */


    std::string node = "SCA on MM-L1DDCvPreProduction.gbtx3.gbtx3";

    std::vector<uint8_t> data;

    data = {0x00, 0x00, 0xab};

    client->writeI2cRaw(node, data.data(), data.size());

    data = {0x00, 0x00};
    client->writeI2cRaw(node, data.data(), data.size());

    uint32_t size = 1;
    auto res = client->readI2c(node, size);
    std::cout << "I2c readback" << std::endl;
    for (auto el : res) {
      std::cout << std::hex << static_cast<uint32_t>(el) << ",";
    }
    std::cout << std::endl;

    std::cout << " ---- Writing several registers at once ----" << std::endl;
    data = {0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E};

    client->writeI2cRaw(node, data.data(), data.size());

    data = {0x00, 0x00};
    client->writeI2cRaw(node, data.data(), data.size());

    size = 14;
    res = client->readI2c(node, size);
    std::cout << "I2c readback" << std::endl;
    for (auto el : res) {
      std::cout << std::hex << static_cast<uint32_t>(el) << ",";
    }
    std::cout << std::endl;
}
