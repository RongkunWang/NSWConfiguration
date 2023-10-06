#ifndef NSWCONFIGURATION_HW_ADDC_H
#define NSWCONFIGURATION_HW_ADDC_H

#include "NSWConfiguration/hw/ART.h"
#include "NSWConfiguration/hw/OpcConnectionBase.h"
#include "NSWConfiguration/hw/ScaAddressBase.h"

namespace nsw {
  class OpcManager;
  class ADDCConfig;
}

namespace nsw::hw {
  /**
   * \brief Class representing an ADDC
   *
   * Provides methods to call SCA on ADDC
   * hosts ART devices that does actual configuration
   *
   * Register mapping: http://cern.ch/go/8jKB
   * Documentation: TODO
   */
  class ADDC : public ScaAddressBase, public OpcConnectionBase
  {
  public:
    /**
     * \brief Constructor from an \ref ADDCConfig object
     * 
     * @param manager Link to the OPC Manager
     * @param config Configuration of the ADDC
     */
    ADDC(nsw::OpcManager& manager, const nsw::ADDCConfig& config);
    ADDC(nsw::OpcManager& manager, const boost::property_tree::ptree& config);
    [[nodiscard]] const std::vector<ART>& getARTs() const {return m_arts;}

    void writeConfiguration() const;
    void writeConfiguration(const std::size_t i_art) const;

  private:
    std::vector<ART> m_arts;  //!< ARTs assiociated to this ADDC
  };
}  // namespace nsw::hw
#endif
