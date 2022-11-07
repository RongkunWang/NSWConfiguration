#ifndef NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_ROCSTATUSREGISTERS_H
#define NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_ROCSTATUSREGISTERS_H

#include <string_view>

#include <ipc/threadpool.h>

#include "NSWConfiguration/hw/DeviceManager.h"
#include "NSWConfiguration/hw/FEB.h"
#include "NSWConfiguration/monitoring/Helper.h"
#include "NSWConfigurationIs/RocStatus.h"

namespace nsw::mon {
  /**
   * \brief Monitor status registers of the ROC
   */
  class RocStatusRegisters
  {
  public:
    /**
     * \brief Constructor
     *
     * \param deviceManager device manager containing HWIs
     */
    explicit RocStatusRegisters(const nsw::hw::DeviceManager& deviceManager);

    /**
     * \brief Monitor and publish information for all devices to IS
     *
     * \param isDict IS dictionary
     * \param serverName name of the monitoring IS server
     */
    void monitor(ISInfoDictionary* isDict, std::string_view serverName);
    static constexpr std::string_view NAME{"RocStatusRegisters"};

  private:
    /**
     * \brief Read data from one ROC and put it into IS info type
     *
     * \param feb HWI
     * \return nsw::mon::is::RocConfiguration IS info struct with values
     */
    [[nodiscard]] static nsw::mon::is::RocStatus getData(const nsw::hw::FEB& feb);

    std::reference_wrapper<const std::vector<nsw::hw::FEB>> m_devices;
    constexpr static std::int64_t NUM_CONCURRENT{20};
    IPCThreadPool m_threadPool{NUM_CONCURRENT};
    internal::MonitorHelper m_helper;
  };
}  // namespace nsw::mon

#endif
