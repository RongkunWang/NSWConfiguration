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

ERS_DECLARE_ISSUE(nsw,
              InvalidGBTxTrainingType,
              "GBTx training issue: bad GBTx type " << gbtx_type,
              ((std::string)gbtx_type)
              )

namespace nsw {
    constexpr std::size_t NUM_GBTX_WRITABLE_REGISTERS{366};
    constexpr std::size_t NUM_GBTX_READONLY_REGISTERS{70};
    constexpr std::size_t NUM_GBTX_TOTAL_REGISTERS{NUM_GBTX_WRITABLE_REGISTERS + NUM_GBTX_READONLY_REGISTERS};
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
    std::array<uint8_t,nsw::NUM_GBTX_WRITABLE_REGISTERS> m_config{};
    std::string m_gbtxType{};
    bool m_active{};

public: 

    /**
     * \brief Construct a new GBTxConfig object
     */
    GBTxConfig();

    /**
     * \brief Construct a new GBTxConfig object
     * 
     * \param gbtxType indicates what type (eg mmg1, stg1) of gbtx this is
     */
    void setType(const std::string& gbtxType){m_gbtxType=gbtxType;};

    /**
     * \brief Print formatted configuration to ERS
     * 
     */
    void debugPrintRegisterMaps();

    /**
     * \brief Sets the whole config, eg to match readback config
     * 
     * \param config: what to set the internal config to
     */
    void setConfig(const std::vector<std::uint8_t>& config);

    /**
     * \brief Sets the register for a given setting to 0x00, then set the setting to value
     * 
     * \param name name of the setting
     * \param value value to be set
     */
    void reset(const std::string& name, const std::size_t value);

    /**
     * \brief Sets the value in the config object for a given setting
     * 
     * \param name name of the setting
     * \param value value to be set
     */
    void set(const std::string& name, const std::size_t value);

    /**
     * @brief Enable training
     *
     * @param trainEc Enable training of EC link
     */
    void enableTraining(bool trainEc);

    /**
     * @brief Disable training
     *
     * @param trainEc Disable training of EC link
     */
    void disableTraining(bool trainEc);

    /**
     * \brief Set the EC e-link phase training register on to begin training EC
     * \param on wether to set on or off
     */
    void setEcTrainingRegisters(const bool on);

    /**
     * \brief Set the e-link phase training registers on to begin training
     * Details: reset channels
     *          set phase tracker mode to 01
     *          set phase training pins on
     */
    void setTrainingRegistersOn();

    /**
     * \brief Set the e-link phase training registers off to stop training
     * Details: set phase training pins off
     */
    void setTrainingRegistersOff();

    /**
     * \brief Set reset channels on
     */
    void setResetChannelsOn();

    /**
     * \brief Set reset channels off
     */
    void setResetChannelsOff();

    /**
     * \brief Sets a register in the config object to a given value
     * 
     * \param r register address
     * \param value value to be set
     * \param shift bit shift of the setting
     */
    void setRegister(std::size_t r, std::uint8_t value, std::size_t shift);

    /**
     * \brief Return true if m_gbtxType contains string type
     * 
     * \param type a string which describes the GBTx type
     * \return bool whether the GBTx type matches the argument
     */
    bool isType(const std::string& type) const {return m_gbtxType.find(type)!=std::string::npos;}

    /**
     * \brief Return configuration as vector
     * 
     * \return std::vector<std::uint8_t> config as register vector
     */
    std::vector<std::uint8_t> configAsVector() const ;

    /**
     * \brief Set the config from file path
     * 
     * \param string file path
     */
    void setConfigFromFile(const std::string& iPath);
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
     * \brief Return whether GBTx is active (set during config)
     * 
     * \return bool
     */
     bool isActive() const {return m_active;}

    /**
     * \brief Set active flag
     */
     void setActive() {m_active = true;}

    /**
     * \brief Parse full GBTx config, return phases
     * 
     * \return std::vector<uint8_t> list of phase values
     */
     static std::vector<uint8_t> getPhasesVector(const std::vector<uint8_t>& config) ;

    /**
     * \brief Parse full GBTx config, return phases
     * 
     * \return boost::property_tree::ptree phase values keyed by setting names
     */
     static boost::property_tree::ptree getPhasesTree(const std::vector<uint8_t>& config) ;

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

