#ifndef NSWCONFIGURATION_HW_PADTRIGGER_H
#define NSWCONFIGURATION_HW_PADTRIGGER_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "NSWConfiguration/PadTriggerSCAConfig.h"

namespace nsw {
  namespace hw {
    /**
     * \brief Class representing a Pad Trigger
     *
     * Provides methods to read/write individual PadTrigger registers,
     * as well as to write a complete configuration and read back all
     * the registers contained in the configuration.
     */
    class PadTrigger
    {
    public:
      /**
       * \brief Constrctor from a \ref PadTriggerConfig object
       */
      explicit PadTrigger(const PadTriggerSCAConfig& config);

      /**
       * \brief Read the full PadTrigger address space
       *
       * \returns a map of address to register value
       */
      std::map<std::uint8_t, std::vector<std::uint32_t>> readConfiguration();

      /**
       * \brief Write the full PadTrigger configuration
       */
      void writeConfiguration();

      /**
       * \brief Write a value to a PadTrigger register address
       *
       * \param regAddress is the address of the register
       * \param scaNode is the address of the SCA that this PadTrigger is connected to
       * \param data is an std::vector containing the bytes to be written to the adddress
       *        - SCAControlRegister has size 4 bytes
       *        - RouterConfigRegister has size 2 bytes
       *        - VTTxControlRegister has size 2 bytes
       */
      void writeRegister(std::uint8_t regAddress,
                         const std::string& scaNode,
                         const std::vector<std::uint8_t>& data);

      /**
       * \brief Read an individual PadTrigger register by its address
       *
       * \param regAddress is the name of the register node
       * \param scaNode is the address of the SCA that this PadTrigger is connected to
       * \param bytesToRead is the number of bytes to read out of the adddress
       *        - SCAControlRegister has size 4 bytes
       *        - RouterConfigRegister has size 2 bytes
       *        - VTTxControlRegister has size 2 bytes
       */
      std::vector<std::uint8_t> readRegister(std::uint8_t regAddress,
                                             const std::string& scaNode,
                                             std::size_t bytesToRead);

      /**
       * \brief Get the \ref PadTriggerConfig object associated with this PadTrigger object
       *
       * Both const and non-const overloads are provided
       */
      [[nodiscard]] PadTriggerSCAConfig& getConfig() { return m_config; }
      [[nodiscard]] const PadTriggerSCAConfig& getConfig() const { return m_config; }  //!< overload

    private:
      PadTriggerSCAConfig m_config;  //!< PadTriggerConfig object associated with this PadTrigger
      std::string m_opcserverIp;     //!< Address and port of OPC Server
      std::string m_scaAddress;      //!< SCA address of PadTrigger item in the OPC address space
    };
  }  // namespace hw
}  // namespace nsw

#endif
