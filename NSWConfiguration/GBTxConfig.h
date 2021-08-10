#ifndef NSWCONFIGURATION_GBTXCONFIG_H
#define NSWCONFIGURATION_GBTXCONFIG_H

#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <unordered_map>

#include <boost/property_tree/ptree.hpp>

#include <ers/ers.h>

#include "NSWConfiguration/Types.h"

ERS_DECLARE_ISSUE(nsw,
                  NSWBoardIssue,
                  message,
                  ((std::string)message)
                  )


namespace nsw {
    constexpr std::size_t NUM_GBTX_REGISTERS = 436;
}

namespace nsw{

/*!
* Configuration class for GBTx
* Quick summary: this loads a ptree of "setting":value pairs,
* and produces a bitmap to configure the GBTx
* ----------------------------------------------------------------------------------------------------
* How it works:
* The GBTx manual is: https://espace.cern.ch/GBT-Project/GBTX/Manuals/gbtxManual.pdf
* The GBTx has 436 8-bit registers. Here is an example: 
*   There are setting configurations are things like the Phase Aligner Tracking Mode (paMode). These
*   are stored in the DB in pairs of string:value, eg "paMode":0b10 or "paMode":2
*   This setting is 2 bites long, and can be AB = b00, b01, b10, b11
*   This mode is set by controlling triplet-redundant bits in register #62. Eg 0bXXABABAB where XX is
*   not used.
*   So we need a map from the DB input of "paMode":0b10 to the setting of register #62 of 0b00101010
*   This class provides this map.
*       * The maps between setting strings and the registers (with shift offsets) is stored in 
*         NSWConfiguration/GBTxRegisterMap.h
*   So what this class does:
*       * Read an input ptree of string:value pairs
*       * Consult NSWConfiguration/GBTxRegisterMap.h to see which register/shift offsets to change
*       * Return the resulting configuration bytestream
*/

class GBTxConfig{
private: 
    std::array<uint8_t,nsw::NUM_GBTX_REGISTERS> m_config{};
    std::unordered_map<std::string,gbtx::regMap> m_registerMaps;

public: 
    /**
     * \brief Construct a new GBTxConfig object
     * 
     */
    GBTxConfig();

    /**
     * \brief Print formatted configuration to ERS
     * 
     */
    void debugPrintRegisterMaps();

    // interface
    /**
     * \brief Sets the value in the config object for a given setting
     * 
     * \param name name of the setting
     * \param value value to be set
     */
    void set(const std::string& name,std::size_t value);

    /**
     * \brief Sets a register in the config object to a given value
     * 
     * \param r register address
     * \param value value to be set
     * \param shift bit shift of the setting
     */
    void setRegister(std::size_t r, std::uint8_t value, std::size_t shift);

    /**
     * \brief Return configuration as vector
     * 
     * \return std::vector<std::uint8_t> config as register vector
     */
    std::vector<std::uint8_t> configAsVector() const ;
    
    /**
     * \brief Set the config from ptree object
     * 
     * \param pt ptree
     */
    void setConfigFromPTree(const boost::property_tree::ptree& pt);

    /**
     * \brief Return nicely formatted string with configuration details
     * 
     * \return std::string formatted config
     */
    std::string getPrintableConfig() const;

    /**
     * \brief Return the number of registers
     * 
     * \return std::size_t number of registers
     */
    std::size_t size() const {return m_config.size();}

    /**
     * \brief Return value of given register
     * 
     * \param i register address
     * \return uint8_t value of register
     */
    std::uint8_t reg(const std::size_t i) const {return m_config.at(i);}


};

}  // namespace nsw



#endif  // NSWCONFIGURATION_GBTXCONFIG_H

