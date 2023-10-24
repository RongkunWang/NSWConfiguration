#ifndef NSWCONFIGURATION_HW_SCAX_H
#define NSWCONFIGURATION_HW_SCAX_H

#include <ers/Issue.h>

#include "NSWConfiguration/TPConstants.h"
#include "NSWConfiguration/hw/SCAInterface.h"

ERS_DECLARE_ISSUE(nsw,
                  SCAXReadbackMismatch,
                  message,
                  ((const char *)message))

  /**
   * \brief The SCAX namespace provides an interface for write read std::uint32_t registeres. 
   * Internally it uses functions from SCAInterfaces after SCAX module is fixed and behaves the same as SCA
   */
namespace nsw::hw::SCAX {


  /**
   * \brief Write register function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param regAddress register address
   * \param value value to be written
   *
   * Uses nsw::OpcClient::writeI2cRaw
   */
  void writeRegister(const OpcClientPtr opcConnection,
                     std::string_view node,
                     const std::uint32_t value);


  /**
   * \brief Read register function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node (including the register name that maps address)
   * \param mask bitmask for the 32-bits read
   *
   * Uses nsw::OpcClient::readI2c
   */
  std::uint32_t readRegister(const OpcClientPtr opcConnection,
                             std::string_view node);

  /**
   * \brief Write, readback, and check register function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node (including the register name that maps address)
   * \param value value to be written
   * \param mask bitmask for the 32-bits read
   */
  void writeAndReadbackRegister(const OpcClientPtr opcConnection,
                                std::string_view node,
                                const std::uint32_t value);


    /**
     * \brief Get the "SkipRegisters" provided by the user configuration, in the form of string_view
     */
    std::set<std::string_view> SkipRegisters(const boost::property_tree::ptree& config);

}  // namespace nsw::hw::SCAX

#endif

