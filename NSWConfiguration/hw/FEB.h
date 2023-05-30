#ifndef NSWCONFIGURATION_HW_FEB_H
#define NSWCONFIGURATION_HW_FEB_H

#include <vector>

#include "NSWConfiguration/Constants.h"
#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/hw/ROC.h"
#include "NSWConfiguration/hw/ScaAddressBase.h"
#include "NSWConfiguration/hw/TDS.h"
#include "NSWConfiguration/hw/VMM.h"

namespace nsw::hw {
  /**
   * \brief Class representing a FEB
   *
   * Provides methods to configure the devices of a FEB and to retrieve the corresponding
   * device interface objects from this FEB.
   */
  class FEB : public ScaAddressBase
  {
  public:
    /**
     * \brief Constructor from a \ref FEBConfig object
     */
    FEB(OpcManager& manager, const nsw::FEBConfig& config);

    /**
     * \brief Get the \ref ROC object
     *
     * Both const and non-const overloads are provided
     */
    [[nodiscard]] ROC& getRoc() { return m_roc; }
    [[nodiscard]] const ROC& getRoc() const { return m_roc; }  //!< \overload

    /**
     * \brief Get the \ref VMM object specified by `id`
     *
     * Both const and non-const overloads are provided
     *
     * \param id Number of VMM
     */
    // clang-format off
    [[nodiscard]] VMM& getVmm(const std::size_t id) { return m_vmms.at(id - m_firstVmm); }
    [[nodiscard]] const VMM& getVmm(const std::size_t id) const { return m_vmms.at(id - m_firstVmm); }  //!< \overload
    // clang-format on

    /**
     * \brief Get the \ref VMM objects connected to this FEB
     *
     * \return std::vector<VMM> reference to the VMM collection
     */
    [[nodiscard]] const std::vector<VMM>& getVmms() const { return m_vmms; }

    /**
     * \brief Get the number of VMMs of this FEB
     *
     * \return std::size_t Number of VMMs
     */
    [[nodiscard]] std::size_t getNumVmms() const { return std::size(m_vmms); }

    /**
     * \brief Get the board index of the first VMM of this FEB
     *
     * \return std::size_t Board index of the first VMM
     */
    [[nodiscard]] std::size_t getFirstVmmIndex() const { return m_firstVmm; }

    /**
     * \brief Get the \ref TDS object specified by `id`
     *
     * Both const and non-const overloads are provided
     *
     * \param id Number of TDS
     */
    // clang-format off
    [[nodiscard]] TDS& getTds(const std::size_t id) { return m_tdss.at(id - m_firstTds); }
    [[nodiscard]] const TDS& getTds(const std::size_t id) const { return m_tdss.at(id - m_firstTds); }  //!< \overload
    // clang-format on

    /**
     * \brief Get the \ref TDS objects connected to this FEB
     *
     * \return std::vector<TDS> reference to the TDS collection
     */
    [[nodiscard]] const std::vector<TDS>& getTdss() const { return m_tdss; }

    /**
     * \brief Test if this FEB is a SFEB6, based on the number of VMMs of this FEB
     *
     * \return bool is SFEB6
     */
    [[nodiscard]] bool isSFEB6() const { return getNumVmms() == nsw::NUM_VMM_PER_SFEB - nsw::SFEB6_FIRST_VMM; }

    /**
     * \brief Configure a FEB
     *
     * \param resetVmm Reset VMMs
     * \param resetTds Reset TDSs
     * \param disableVmmCaptureInputs Disable VMM capture inputs after configuring ROC (THEY STAY DISABLED) 
     */
    void writeConfiguration(bool resetVmm = false, bool resetTds = false, bool disableVmmCaptureInputs = false) const;
  private:
    ROC m_roc;                //!< ROC assiociated to this FEB
    std::vector<VMM> m_vmms;  //!< VMMs assiociated to this FEB
    std::vector<TDS> m_tdss;  //!< TDSs assiociated to this FEB
    std::size_t m_firstVmm{nsw::MAX_NUMBER_OF_VMM};  //!< Hold the board ID of the first VMM to correctly access by index of container
    std::size_t m_firstTds{nsw::MAX_NUMBER_OF_TDS};  //!< Hold the board ID of the first TDS to correctly access by index of container
  };
}  // namespace nsw::hw

#endif
