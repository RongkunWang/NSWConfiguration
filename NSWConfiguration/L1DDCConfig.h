#ifndef NSWCONFIGURATION_L1DDCCONFIG_H_
#define NSWCONFIGURATION_L1DDCCONFIG_H_

#include <string>
#include <vector>
#include "NSWConfiguration/Utility.h"

#include <boost/property_tree/ptree.hpp>

#include "NSWConfiguration/GBTxConfig.h"

ERS_DECLARE_ISSUE(nsw,
                  NSWL1DDCIssue,
                  message,
                  ((std::string)message)
                  )


namespace nsw {

/*!
* This class handels the preparation of the L1DDC configuration
* The input is a ptree with GBTx branches
* The output is provided by getGbtxBytestream(gbtxId), where gbtxId=0,1,2
*   Note: GBTx1,2 are not supported yet
* It can also be updated to configure the SCA
*/

struct GBTxContainer {
    GBTxConfig GBTx;
    bool configureGBTx{};
    bool ecElinkTrain{};
    GBTxContainer(GBTxConfig gbtx, const bool configure, const bool ecTrain) 
        : GBTx(gbtx), configureGBTx(configure), ecElinkTrain(ecTrain)
    {}
};

class L1DDCConfig {
 private:

    std::size_t m_portToGBTx{};
    std::size_t m_portFromGBTx{};
    std::size_t m_elinkId{};
    std::string m_boardType{};
    std::string m_GBTxPhaseOutputDBPath{};
    std::vector<GBTxContainer> m_GBTxContainers{};
    bool m_trainGBTxPhaseAlignment{};
    int m_trainGBTxPhaseWaitTime{};
    int m_i2cDelay{};
    int m_configOption{};
    int m_i2cBlockSize{};
    std::string m_felixServerIp{};
    std::string m_opcServerIp{};
    std::string m_opcNodeId{};
    std::string m_name{};
    std::string m_nodeName{};

 public:

    /**
     * \brief Construct a new L1DDCConfig object
     *
     * Example configuration ptree:
     * {
     *   "felixServerIp": "pcatlnswfelix10.cern.ch",
     *   "portToGBTx": "12340",
     *   "portFromGBTx": "12350",
     *   "elinkId": 62,
     *   gbtx0:{
     *       "resetPLLBar":0,
     *       "enableTestBar":1,
     *       ...
     *   },
     *   gbtx1:{
     *       ...
     *   },
     *   gbtx2:{
     *       ...
     *   }
     * }
     *
     * \param config configuration ptree (see example above)
     */
    explicit L1DDCConfig(const boost::property_tree::ptree& config);

    /**
     * Simple configuration for standalone use
     */
    explicit L1DDCConfig(const nsw::GBTxSingleConfig& config);

    /**
     * Set up all GBTx objects
     */
    void initGBTxs(const boost::property_tree::ptree& config);

    /**
     * Set up individual GBTx
     */
    void initGBTx(const boost::property_tree::ptree& config, const std::size_t gbtxId);

    /**
     * Get number of GBTx objects
     */
    int getNumberGBTx() const {return m_GBTxContainers.size();}

    /**
     * \brief Get GBTx of given number
     *
     * \param gbtxId GBTx ID
     * \return GBTx
     */
    const GBTxConfig& getGBTx(const std::size_t i) const {return m_GBTxContainers.at(i).GBTx;}
    GBTxConfig& getGBTx(const std::size_t i) {return m_GBTxContainers.at(i).GBTx;} //!< \overload

    /**
     * \brief Get the port to Gbtx
     *
     * \return std::size_t port
     */
    std::size_t getPortToGbtx() const {return m_portToGBTx;}

    /**
     * \brief Get the port from GBTx
     *
     * \return std::size_t port
     */
    std::size_t getPortFromGbtx() const {return m_portFromGBTx;}

    /**
     * \brief Get the elink ID
     *
     * \return std::size_t elink ID
     */
    std::size_t getElinkId() const {return m_elinkId;}

    /**
     * \brief Get the name of GBTx phase output directory.
     *
     * \return std::string name of output directory for GBTx phases
     */
    std::string getGBTxPhaseOutputDBPath() const {return m_GBTxPhaseOutputDBPath;}

    /**
     * \brief Get the name of l1ddc node in json file. 
     *
     * \return std::string name of l1ddc node
     */
    std::string getNodeName() const {return m_nodeName;}

    /**
     * \brief Get the name of l1ddc. The format is L1DDC:felixServerIp/portToGBTx/portFromGBTx/elinkId
     *
     * \return std::string name of l1ddc
     */
    std::string getName() const {return m_name;}

    /**
     * \brief Get the IP address of the felix server
     *
     * \return std::string felix IP address
     */
    std::string getFelixServerIp() const {return m_felixServerIp;}

    /**
     * \brief Get whether to config gbtx
     *
     * \param gbtxId GBTx ID
     * \return bool
     */
    bool getConfigureGBTx(const std::size_t gbtxId) const {return (gbtxId<getNumberGBTx())?m_GBTxContainers.at(gbtxId).configureGBTx:false;}

    /**
     * \brief Get the IP address of the opc server
     *
     * \return std::string opc IP address
     */
    std::string getOpcServerIp() const {return m_opcServerIp;}

    /**
     * \brief Get the node ID
     *
     * \return std::string opc IP address
     */
    std::string getOpcNodeId() const {return m_opcNodeId;}

    /**
     * \brief Return the time to wait while training the GBTx phase alignment
     *
     * \return int train wait time in us
     */
    int trainGBTxPhaseWaitTime() const {return m_trainGBTxPhaseWaitTime;}

    /**
     * \brief Return config option
     *
     * \return int config option
     */
    int configOption() const {return m_configOption;}

    /**
     * \brief Return the block size for i2c configuration
     *
     * \return int I2C configuration block size in bytes
     */
    int i2cBlockSize() const {return m_i2cBlockSize;}

    /**
     * \brief Return the time to wait during i2c configuration
     *
     * \return int delay time in us
     */
    int i2cDelay() const {return m_i2cDelay;}

    /**
     * \brief Return whether to train the GBTx phase alignment has been configured. Can be "mmg", "stg", or "none"
     *
     * \return bool train setting
     */
    bool trainGBTxPhaseAlignment() const {return m_trainGBTxPhaseAlignment;}

    /**
     * \brief Get the configuration bytestream for a given GBTx
     *
     * \param gbtxId GBTx ID
     * \return std::vector<uint8_t> bytestream
     */
    std::vector<std::uint8_t> getGBTxBytestream(std::size_t gbtxId) const;

    /**
     * \brief Update config objects in each GBTx instance to start training e-links
     */
    void trainGbtxsOn();

    /**
     * \brief Update config objects in each GBTx instance to stop training e-links
     */
    void trainGbtxsOff();

};
}  // namespace nsw

#endif  // NSWCONFIGURATION_L1DDCCONFIG_H_
