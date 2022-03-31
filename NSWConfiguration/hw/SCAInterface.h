#ifndef NSWCONFIGURATION_HW_SCA_H
#define NSWCONFIGURATION_HW_SCA_H

#include <vector>
#include <string>
#include <memory>

#include "NSWConfiguration/I2cMasterConfig.h"
#include "NSWConfiguration/OpcClient.h"

namespace nsw::hw::SCA {
  /**
   * \brief Low-level I2c send function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param vdata vector of data
   */
  void sendI2c(const nsw::OpcClientPtr& opcConnection,
               const std::string& node,
               const std::vector<std::uint8_t>& vdata);

  /**
   * \brief Low-level I2c send function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param data array of data
   * \param dataSize size of array
   */
  void sendI2cRaw(const nsw::OpcClientPtr& opcConnection,
                  const std::string& node,
                  const std::uint8_t* data,
                  size_t dataSize);

  /**
   * \brief Low-level GPIO send function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param data array of data
   * \param dataSize size of array
   */
  void sendGPIO(const nsw::OpcClientPtr& opcConnection, const std::string& node, bool data);

  /**
   * \brief Read back GPIO register
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   */
  bool readGPIO(const nsw::OpcClientPtr& opcConnection, const std::string& node);

  /**
   * \brief Read ADC from SCA analog input
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param nSamples Number of samples of ADC
   * \return std::vector<std::uint16_t> samples
   */
  std::vector<std::uint16_t> readAnalogInputConsecutiveSamples(
    const nsw::OpcClientPtr& opcConnection,
    const std::string& node,
    std::size_t nSamples);

  /**
   * \brief Read back I2c register as vector
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param numberOfBytes number of bytes to be read
   * \return std::vector<std::uint8_t> Byte vector containing the data
   */
  std::vector<std::uint8_t> readI2c(const nsw::OpcClientPtr& opcConnection,
                                    const std::string& node,
                                    size_t numberOfBytes = 1);

  /**
   * \brief Read back I2c register as vector for ADDCs
   *
   * First, write the address without data and then read the data.
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param address I2c address to be read in bytes
   * \param addressSize number of bytes in \ref address array
   * \param numberOfBytes number of bytes to be read
   * \return std::vector<std::uint8_t> Byte vector containing the data
   */
  std::vector<std::uint8_t> readI2cAtAddress(const nsw::OpcClientPtr& opcConnection,
                                             const std::string& node,
                                             const std::uint8_t* address,
                                             size_t addressSize,
                                             size_t numberOfBytes = 1);

  /**
   * \brief Send I2c register for ADDC
   *
   * Add the address to the front of the data vector before sending.
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param address I2c address to be read in bytes
   * \param data Byte vector containing the data
   */
  void sendI2cAtAddress(const nsw::OpcClientPtr& opcConnection,
                        const std::string& node,
                        const std::vector<std::uint8_t>& address,
                        std::vector<std::uint8_t> data);

  /**
   * \brief High-level function to send configuration to all addresses under an I2cMaster
   *
   * \param opcConnection OPC server connection
   * \param topnode Top level name of the OPC node
   * \param cfg config object holding addresses and data
   */
  void sendI2cMasterConfig(const nsw::OpcClientPtr& opcConnection,
                           const std::string& topnode,
                           const nsw::I2cMasterConfig& cfg);

  /**
   * \brief Write only one of the register addresses in I2cMaster
   *
   * \param opcConnection OPC server connection
   * \param topnode Top level name of the OPC node
   * \param cfg config object holding addresses and data
   * \param regAddress address of register to be written
   */
  void sendI2cMasterSingle(const nsw::OpcClientPtr& opcConnection,
                           const std::string& topnode,
                           const nsw::I2cMasterConfig& cfg,
                           const std::string& regAddress);

  /**
   * \brief Write only one of the register addresses in I2cMaster
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param data byte vector holding the data
   * \param regAddress address of register to be written
   */
  void sendI2cMasterSingle(const nsw::OpcClientPtr& opcConnection,
                           const std::string& node,
                           const std::vector<std::uint8_t>& data,
                           const std::string& regAddress);

  /**
   * \brief Low-level SPI send function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param data byte array holding the data
   * \param dataSize size of the array
   */
  void sendSpiRaw(const nsw::OpcClientPtr& opcConnection,
                  const std::string& node,
                  const std::uint8_t* data,
                  size_t dataSize);

  /**
   * \brief Low-level function to read over SPI
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param dataSize number of bytes to be read
   * \return std::vector<std::uint8_t> vector of bytes holding the read data
   */
  std::vector<std::uint8_t> readSpi(const nsw::OpcClientPtr& opcConnection,
                                    const std::string& node,
                                    size_t dataSize);

  /**
   * \brief Low-level SPI send function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param vdata byte vector holding the data
   */
  void sendSpi(const nsw::OpcClientPtr& opcConnection,
               const std::string& node,
               const std::vector<std::uint8_t>& vdata);

  /**
   * \brief Xilinx FPGA write function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param bitfile_path path to bitfile
   */
  void writeXilinxFpga(const OpcClientPtr& opcConnection,
                       const std::string& node,
                       const std::string& bitfile_path);


  /**
   * \brief Write anytype SCA OPC UA's FreeVariable
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \tparam value to be written
   */
  template<typename T>
  void writeFreeVariable(const OpcClientPtr& opcConnection,
                         const std::string& node,
                         const T& value)
  {
    opcConnection->writeFreeVariable(node, value);
  };

}  // namespace nsw::hw::SCA

#endif
