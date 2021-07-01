#include "NSWConfiguration/VMMInterface.h"

#include <iterator>
#include <stdexcept>
#include <string>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/I2cRegisterMappings.h"
#include "NSWConfiguration/OpcManager.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/SCAInterface.h"

void nsw::DeviceInterface::VMM::writeConfiguration(const nsw::FEBConfig& config,
                                                   const std::size_t numVmm) {
  // Set Vmm Configuration Enable
  constexpr uint8_t VMM_ACC_DISABLE = 0xff;
  constexpr uint8_t VMM_ACC_ENABLE  = 0x00;
  const auto&       opcConnection =
    OpcManager::getConnection(config.getOpcServerIp());

  // Set Vmm Acquisition Disable
  const auto sca_roc_address_analog =
    config.getAddress() + "." + config.getRocAnalog().getName();
  const std::string regVmmAccName = "reg122vmmEnaInv";
  nsw::DeviceInterface::SCA::sendI2c(opcConnection,
                                     sca_roc_address_analog + '.' +
                                       regVmmAccName,
                                     {VMM_ACC_DISABLE});

  const auto vmm  = config.getVmms().at(numVmm);
  const auto data = vmm.getByteVector();

  ERS_LOG("Sending configuration to " << config.getAddress() << ".spi."
                                      << vmm.getName());

  nsw::DeviceInterface::SCA::sendSpiRaw(opcConnection,
                                        config.getAddress() + ".spi." +
                                          vmm.getName(),
                                        data.data(),
                                        data.size());

  ERS_DEBUG(5, "Hexstring:\n" << nsw::bitstringToHexString(vmm.getBitString()));

  // Set Vmm Acquisition Enable
  nsw::DeviceInterface::SCA::sendI2c(opcConnection,
                                     sca_roc_address_analog + '.' +
                                       regVmmAccName,
                                     {VMM_ACC_ENABLE});
}

std::map<std::uint8_t, std::vector<std::uint8_t>>
nsw::DeviceInterface::VMM::readConfiguration(
  [[maybe_unused]] const nsw::FEBConfig& config,
  [[maybe_unused]] const std::size_t     numVmm) {
  throw std::logic_error("Not implemented");
}

void nsw::DeviceInterface::VMM::writeRegister(
  [[maybe_unused]] const nsw::FEBConfig& config,
  [[maybe_unused]] const std::size_t     numVmm,
  [[maybe_unused]] const std::uint8_t    registerId,
  [[maybe_unused]] const std::uint64_t   value) {
  throw std::logic_error("Not implemented");
}

void nsw::DeviceInterface::VMM::writeRegister(
  [[maybe_unused]] const nsw::FEBConfig& config,
  [[maybe_unused]] const std::size_t     numVmm,
  [[maybe_unused]] const std::string&    regAddress,
  [[maybe_unused]] const std::uint64_t   value) {
  throw std::logic_error("Not implemented");
}

std::vector<std::uint8_t> nsw::DeviceInterface::VMM::readRegister(
  [[maybe_unused]] const nsw::FEBConfig& config,
  [[maybe_unused]] const std::size_t     numVmm,
  [[maybe_unused]] const std::uint8_t    registerId) {
  throw std::logic_error("Not implemented");
}