/// MMFE8 Configuration class

#ifndef NSWCONFIGURATION_FEBCONFIG_H_
#define NSWCONFIGURATION_FEBCONFIG_H_

#include <string>
#include <vector>

#include "NSWConfiguration/Utility.h"
#include "NSWConfiguration/SCAConfig.h"
#include "NSWConfiguration/VMMConfig.h"
#include "NSWConfiguration/I2cMasterConfig.h"

namespace nsw {

//! Base class or configuration for any front end board that has
//! - SCA
//! - ROC and VMM(multiple)
//! - TDS(zero or multiple)
//!
//! Used as base class for MMFE8, PFEB and SBEF Configurations

class FEBConfig: public SCAConfig {
 private:
    std::vector<VMMConfig> m_vmms;
    I2cMasterConfig m_roc_analog;
    I2cMasterConfig m_roc_digital;
    std::vector<I2cMasterConfig> m_tdss;
    std::vector<std::string> m_gpios;  // List of GPIO names in the FEB
    std::size_t m_firstVmm{nsw::MAX_NUMBER_OF_VMM};  //!< Hold the board ID of the first VMM to correctly access by index of container
    std::size_t m_firstTds{nsw::MAX_NUMBER_OF_TDS};  //!< Hold the board ID of the first TDS to correctly access by index of container

 public:
    //! Constructor.
    //! The ptree in the argument should contain
    //! - OpcServerIp, OpcNodeId
    //! - rocPllCoreAnalog, rocCoreDigital
    //! - Multiple vmm instances named vmm0 to vmmN
    //! - Multiple tds instances named tds0 to tdsN (optional)
    explicit FEBConfig(const boost::property_tree::ptree& config);
    ~FEBConfig() = default;

    void dump() const;

    //! Get a reference to the \c VMMConfig object for VMM<i>
    //!
    //! Both const and non-const overloads are provided
    //!
    //! \param i is the board position index of the VMM of which to
    //!        return the configuration. `i` must be in the range
    //!        [0,7] and myst be greater than \c m_firstVmm for the
    //!        given FEB
    VMMConfig & getVmm(size_t i) {return m_vmms.at(i-m_firstVmm);}
    //! Get a const reference to the \c VMMConfig object for VMM<i>, where `i` is the board position index
    const VMMConfig & getVmm(size_t i) const {return m_vmms.at(i-m_firstVmm);} //!< \overload
    const std::vector<VMMConfig> & getVmms() const {return m_vmms;}
    const I2cMasterConfig & getRocAnalog() const {return m_roc_analog;}
    const I2cMasterConfig & getRocDigital() const {return m_roc_digital;}
    const std::vector<I2cMasterConfig> & getTdss() const {return m_tdss;}

    std::size_t getFirstVmmIndex() const {return m_firstVmm;}
    std::size_t getFirstTdsIndex() const {return m_firstTds;}

    I2cMasterConfig & getRocAnalog() {
        return const_cast<I2cMasterConfig &>(static_cast<const FEBConfig&>(*this).getRocAnalog());
    }

    I2cMasterConfig & getRocDigital() {
        return const_cast<I2cMasterConfig &>(static_cast<const FEBConfig&>(*this).getRocDigital());
    }

    std::vector<VMMConfig> & getVmms() {
        return const_cast<std::vector<VMMConfig>&>(static_cast<const FEBConfig&>(*this).getVmms());
    }

    std::vector<I2cMasterConfig> & getTdss() {
        return const_cast<std::vector<I2cMasterConfig>&>(static_cast<const FEBConfig&>(*this).getTdss());
    }
};
}  // namespace nsw

#endif  // NSWCONFIGURATION_FEBCONFIG_H_
