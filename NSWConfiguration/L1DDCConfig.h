#ifndef NSWCONFIGURATION_L1DDCCONFIG_H_
#define NSWCONFIGURATION_L1DDCCONFIG_H_

#include <string>
#include <vector>

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

class L1DDCConfig {
 private:
    GBTxConfig m_gbtx0;

    std::size_t m_portToGBTx;
    std::size_t m_portFromGBTx;
    std::size_t m_elinkId;
    std::string m_opcServerIp;

 public:

    /**
     * \brief Construct a new L1DDCConfig object
     * 
     * Example configuration ptree:
     * {
     *   "OpcServerIp": "pcatlnswfelix10.cern.ch",
     *   "OpcNodeId": "L1DDC_0",
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
     * \brief Get the IP address of the OPC server
     * 
     * \return std::string OPC IP address
     */
    std::string getOpcServerIp() const {return m_opcServerIp;}

    /**
     * \brief Get the configuration bytestream for a given GBTx
     * 
     * \param gbtxId GBTx ID
     * \return std::vector<uint8_t> bytestream
     */
    std::vector<std::uint8_t> getGbtxBytestream(std::size_t gbtxId) const;
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_L1DDCCONFIG_H_
