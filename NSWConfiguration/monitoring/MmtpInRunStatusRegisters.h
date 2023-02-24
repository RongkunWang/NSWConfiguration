#ifndef NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_MMTPINRUNSTATUSREGISTERS_H
#define NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_MMTPINRUNSTATUSREGISTERS_H

#include <string_view>

#include <ipc/threadpool.h>

#include "NSWConfiguration/hw/DeviceManager.h"
#include "NSWConfiguration/monitoring/Helper.h"
#include "NSWConfigurationIs/MmtpInRunStatusRegisters.h"

namespace nsw::mon {
  /**
   * \brief Monitor configuration registers of the ROC
   */
  class MmtpInRunStatusRegisters
  {
  public:
    /**
     * \brief Constructor
     *
     * \param deviceManager device manager containing HWIs
     */
    explicit MmtpInRunStatusRegisters(const nsw::hw::DeviceManager& deviceManager);

    /**
     * \brief Monitor and publish information for all devices to IS
     *
     * \param isDict IS dictionary
     * \param serverName name of the monitoring IS server
     */
    void monitor(ISInfoDictionary* isDict, std::string_view serverName);
    static constexpr std::string_view NAME{"MmtpInRunStatusRegisters"};

  private:
    /**
     * \brief Read data from one MMTP and put it into IS info type
     *
     * \param feb HWI
     * \return nsw::mon::is::MmtpInRunStatusRegisters IS info struct with values
     */
    [[nodiscard]] static nsw::mon::is::MmtpInRunStatusRegisters getData(const nsw::hw::MMTP& tp);

    std::reference_wrapper<const std::vector<nsw::hw::MMTP>> m_devices;
    constexpr static std::int64_t NUM_CONCURRENT{5};
    IPCThreadPool m_threadPool{NUM_CONCURRENT};
    internal::MonitorHelper m_helper;
  };
}  // namespace nsw::mon

#endif
