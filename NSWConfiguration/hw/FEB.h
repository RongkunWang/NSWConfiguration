#ifndef NSWCONFIGURATION_HW_FEB_H
#define NSWCONFIGURATION_HW_FEB_H

#include <vector>

#include "NSWConfiguration/FEBConfig.h"
#include "NSWConfiguration/hw/ROC.h"
#include "NSWConfiguration/hw/TDS.h"
#include "NSWConfiguration/hw/VMM.h"

namespace nsw::hw {
  /**
   * \brief Class representing a FEB
   *
   * Provides methods to configure the devices of a FEB and to retrieve the corresponding
   * device interface objects from this FEB.
   */
  class FEB
  {
  public:
    /**
     * \brief Constructor from a \ref FEBConfig object
     */
    explicit FEB(const nsw::FEBConfig& config) :
      m_roc(config),
      m_vmms([&config]() {
        std::vector<VMM> vmms;
        vmms.reserve(config.getVmms().size());
        for (const auto& vmm : config.getVmms()) {
          vmms.emplace_back(vmm);
        }
        return vmms;
      }()),
      m_tdss([&config]() {
        std::vector<TDS> tdss;
        tdss.reserve(config.getTdss().size());
        for (const auto& tds : config.getTdss()) {
          tdss.emplace_back(tds);
        }
        return tdss;
      }())
    {}

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
    [[nodiscard]] VMM& getVmm(const std::size_t id) { return m_vmms.at(id); }
    [[nodiscard]] const VMM& getVmm(const std::size_t id) const { return m_vmms.at(id); }  //!< \overload
    // clang-format on

    /**
     * \brief Get the \ref TDS object specified by `id`
     *
     * Both const and non-const overloads are provided
     *
     * \param id Number of TDS
     */
    // clang-format off
    [[nodiscard]] TDS& getTds(const std::size_t id) { return m_tdss.at(id); }
    [[nodiscard]] const TDS& getTds(const std::size_t id) const { return m_tdss.at(id); }  //!< \overload
    // clang-format on

    /**
     * \brief Get the Opc Node Id
     *
     * \return std::string Opc Node Id
     */
    [[nodiscard]] std::string getOpcNodeId() const { return m_opcNodeId; }

  private:
    ROC m_roc;                //!< ROC assiociated to this FEB
    std::vector<VMM> m_vmms;  //!< VMMs assiociated to this FEB
    std::vector<TDS> m_tdss;  //!< TDSs assiociated to this FEB
    std::string m_opcNodeId;  //!< SCA address of FE item in Opc address space
  };
}  // namespace nsw::hw

#endif
