#ifndef NSWCONFIGURATION_HW_TP_H
#define NSWCONFIGURATION_HW_TP_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "NSWConfiguration/TPConfig.h"

namespace nsw {
  namespace hw {
    /**
     * \brief Class representing a Trigger Processor
     *
     * Provides methods to read/write individual TP registers, as well
     * as to write a complete configuration and read back all the
     * registers contained in the configuration.
     */
    class TP
    {
    public:
      /**
       * \brief Constrctor from a \ref TPConfig object
       */
      explicit TP(const TPConfig& config);

      /**
       * \brief Read the full TP address space
       *
       * \returns a map of address to register value
       */
      std::map<std::uint8_t, std::vector<std::uint32_t>> readConfiguration() const;

      /**
       * \brief Write the full TP configuration
       */
      void writeConfiguration() const;

      /**
       * \brief Write a value to a TP register address
       *
       * \param regAddress is the address of the register
       * \param value is the value to be written
       */
      void writeRegister(std::uint8_t registerId, std::uint32_t value) const;

      /**
       * \brief Read an individual TP register by its address
       */
      std::vector<std::uint8_t> readRegister(std::uint8_t registerId) const;

      /**
       * \brief Get the \ref TPConfig object associated with this TP object
       *
       * Both const and non-const overloads are provided
       */
      [[nodiscard]] TPConfig& getConfig() { return m_config; }
      [[nodiscard]] const TPConfig& getConfig() const { return m_config; }  //!< \overload

    private:
      TPConfig m_config;          //!< TPConfig object associated with this TP
      std::string m_opcserverIp;  //!< address and port of Opc Server
      std::string m_scaAddress;   //!< SCA address of TP item in Opc address space
    };
  }  // namespace hw
}  // namespace nsw

#endif
