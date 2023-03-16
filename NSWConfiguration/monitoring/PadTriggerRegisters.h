#ifndef NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_PADTRIGGERREGISTERS_H
#define NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_PADTRIGGERREGISTERS_H

#include <ipc/threadpool.h>

#include "NSWConfiguration/hw/DeviceManager.h"
#include "NSWConfiguration/monitoring/Helper.h"
#include "NSWConfigurationIs/PadTriggerRegisters.h"

namespace nsw::mon {
  /**
   * \brief Monitor registers of the pad trigger
   */
  class PadTriggerRegisters
  {
  public:
    /**
     * \brief Constructor
     *
     * \param deviceManager device manager containing HWIs
     */
    explicit PadTriggerRegisters(const nsw::hw::DeviceManager& deviceManager);

    /**
     * \brief Monitor and publish information for all devices to IS
     *
     * \param isDict IS dictionary
     * \param serverName name of the monitoring IS server
     */
    void monitor(ISInfoDictionary* isDict, std::string_view serverName);
    static constexpr std::string_view NAME{"PadTriggerRegisters"};

    /**
     * \brief Read data from one pad trigger and put it into IS info type
     *
     * \param dev Pad trigger device
     * \return nsw::mon::is::PadTriggerRegisters IS info struct with values
     */
    [[nodiscard]]
    static nsw::mon::is::PadTriggerRegisters getData(const nsw::hw::PadTrigger& dev);

  private:
    const nsw::hw::DeviceManager& m_devices;
    constexpr static std::int64_t NUM_CONCURRENT{1};
    IPCThreadPool m_threadPool{NUM_CONCURRENT};
    internal::MonitorHelper m_helper;
  };
}

#endif
