#ifndef NSWCONFIGURATION_MONITORING_STGCTPINRUNSTATUSREGISTERS_H
#define NSWCONFIGURATION_MONITORING_STGCTPINRUNSTATUSREGISTERS_H

#include <string_view>

#include <ipc/threadpool.h>

#include <NSWConfigurationIs/StgctpInRunStatusRegisters.h>

#include "NSWConfiguration/hw/DeviceManager.h"
#include "NSWConfiguration/monitoring/Helper.h"

namespace nsw::mon {
  /**
   * \brief Monitor configuration registers of the ROC
   */
  class StgctpInRunStatusRegisters
  {
  public:
    /**
     * \brief Constructor
     *
     * \param deviceManager device manager containing HWIs
     */
    explicit StgctpInRunStatusRegisters(const nsw::hw::DeviceManager& deviceManager);

    /**
     * \brief Monitor and publish information for all devices to IS
     *
     * \param isDict IS dictionary
     * \param serverName name of the monitoring IS server
     */
    void monitor(ISInfoDictionary* isDict, std::string_view serverName);
    static constexpr std::string_view NAME{"StgctpInRunStatusRegisters"};

  private:
    /**
     * \brief Read data from one STGCTP and put it into IS info type
     *
     * \param feb HWI
     * \return nsw::mon::is::StgctpInRunStatusRegisters IS info struct with values
     */
    [[nodiscard]] static nsw::mon::is::StgctpInRunStatusRegisters getData(const nsw::hw::STGCTP& tp);

    std::reference_wrapper<const std::vector<nsw::hw::STGCTP>> m_devices;
    constexpr static std::int64_t NUM_CONCURRENT{1};
    IPCThreadPool m_threadPool{NUM_CONCURRENT};
    internal::MonitorHelper m_helper;
  };
}  // namespace nsw::mon

#endif
