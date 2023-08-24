#ifndef NSWCONFIGURATION_HW_SCAX_H
#define NSWCONFIGURATION_HW_SCAX_H

#include <ers/Issue.h>

#include "NSWConfiguration/TPConstants.h"
#include "NSWConfiguration/hw/SCAInterface.h"

ERS_DECLARE_ISSUE(nsw,
                  SCAXReadbackMismatch,
                  message,
                  ((const char *)message))

namespace nsw::hw::SCAX {

  /**
   * \brief Write register function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param regAddress register address
   * \param value value to be written
   */
  void writeRegister(const OpcClientPtr opcConnection,
                     const std::string& node,
                     const std::uint32_t regAddress,
                     const std::uint32_t value);

  /**
   * \brief Read register function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param regAddress register address
   * \param mask bitmask for the 32-bits read
   */
  std::uint32_t readRegister(const OpcClientPtr opcConnection,
                             const std::string& node,
                             const std::uint32_t regAddress,
                             const std::uint32_t mask = nsw::scax::BITMASK_ALL);

  /**
   * \brief Write, readback, and check register function
   *
   * \param opcConnection OPC server connection
   * \param node name of the OPC node
   * \param regAddress register address
   * \param value value to be written
   * \param mask bitmask for the 32-bits read
   */
  void writeAndReadbackRegister(const OpcClientPtr opcConnection,
                                const std::string& node,
                                const std::uint32_t regAddress,
                                const std::uint32_t value,
                                const std::uint32_t mask = nsw::scax::BITMASK_ALL);

}  // namespace nsw::hw::SCAX

#endif

