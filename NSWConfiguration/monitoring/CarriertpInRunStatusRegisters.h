#ifndef NSWCONFIGURATION_MONITORING_CARRIERTPINRUNSTATUSREGISTERS_H
#define NSWCONFIGURATION_MONITORING_CARRIERTPINRUNSTATUSREGISTERS_H

#include <string_view>

#include <ipc/threadpool.h>

#include <NSWConfigurationIs/CarriertpInRunStatusRegisters.h>

#include "NSWConfiguration/hw/DeviceManager.h"
#include "NSWConfiguration/monitoring/Helper.h"

namespace nsw::mon {
  /**
   * \brief Monitor configuration registers of the TP carrier
   */
  class CarriertpInRunStatusRegisters
  {
  public:
    /**
     * \brief Constructor
     *
     * \param deviceManager device manager containing HWIs
     */
    explicit CarriertpInRunStatusRegisters(const nsw::hw::DeviceManager& deviceManager);

    /**
     * \brief Monitor and publish information for all devices to IS
     *
     * \param isDict IS dictionary
     * \param serverName name of the monitoring IS server
     */
    void monitor(ISInfoDictionary* isDict, std::string_view serverName);
    static constexpr std::string_view NAME{"CarriertpInRunStatusRegisters"};

  private:
    /**
     * \brief Read data from one TPCarrier and put it into IS info type
     *
     * \param TP carrier HWI
     * \return nsw::mon::is::CarriertpInRunStatusRegisters IS info struct with values
     */
    [[nodiscard]] static nsw::mon::is::CarriertpInRunStatusRegisters getData(const nsw::hw::TPCarrier& tp);

    std::reference_wrapper<const std::vector<nsw::hw::TPCarrier>> m_devices;
    constexpr static std::int64_t NUM_CONCURRENT{1};
    IPCThreadPool m_threadPool{NUM_CONCURRENT};
    internal::MonitorHelper m_helper;
  };
}  // namespace nsw::mon

#endif
