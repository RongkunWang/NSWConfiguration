
/// MMTP Configuration class(sTGC configuration class see hw/STGCTP)

#ifndef NSWCONFIGURATION_TPCONFIG_H_
#define NSWCONFIGURATION_TPCONFIG_H_

#include "NSWConfiguration/SCAConfig.h"

#include <string>
#include <vector>
#include <map>

namespace nsw {
class I2cMasterConfig;

class TPConfig: public SCAConfig {
 private:
    const uint8_t m_numMasters;
    std::map<std::string, I2cMasterConfig*> m_registerFiles;
    void restructureConfig();

 public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
    explicit TPConfig(const boost::property_tree::ptree& config);
    ~TPConfig();

    uint32_t getRegisterValue(const std::string& master, const std::string& slave,
        const std::string& register_name = "register") const;
    void setRegisterValue(const std::string& master, const std::string& slave, uint32_t value,
        const std::string& register_name = "register");
    void dump() const;

    std::map<std::string, I2cMasterConfig*> getI2cMastersMap() { return m_registerFiles; }
    uint8_t getNumMasters() const { return m_numMasters; }

    int ARTWindowCenter() const;
    int ARTWindowLeft() const;
    int ARTWindowRight() const;
    uint32_t FiberBCOffset() const;
    int GlobalInputPhase() const;
    int GlobalInputOffset() const;
    int SelfTriggerDelay() const;
    int VmmMaskHotThresh() const;
    int VmmMaskHotThreshHyst() const;
    int VmmMaskDrainPeriod() const;
    bool EnableChannelRates() const;
    int L1ALatencyScanStart() const;
    int L1ALatencyScanEnd() const;
    int HorxEnvMonAddr() const;

    void setARTWindowCenter(int val);
    void setARTWindowLeft(int val);
    void setARTWindowRight(int val);
    void setFiberBCOffset(uint32_t val);
    void setGlobalInputPhase(int val);
    void setGlobalInputOffset(int val);
    void setSelfTriggerDelay(int val);
    void setVmmMaskHotThresh(int val);
    void setVmmMaskHotThreshHyst(int val);
    void setVmmMaskDrainPeriod(int val);
    void setEnableChannelRates(bool val);
    void setL1ALatencyScanStart(int val);
    void setL1ALatencyScanEnd(int val);
    void setHorxEnvMonAddr(bool tx, std::uint8_t microPod, bool temp, bool loss, std::uint8_t fiber);
};

}  // namespace nsw

#endif  // NSWCONFIGURATION_TPCONFIG_H_


