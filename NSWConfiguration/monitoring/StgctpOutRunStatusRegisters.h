#ifndef NSWCONFIGURATION_MONITORING_STGCTPOUTRUNSTATUSREGISTERS_H
#define NSWCONFIGURATION_MONITORING_STGCTPOUTRUNSTATUSREGISTERS_H

#include <string_view>

#include <ipc/threadpool.h>

#include <NSWConfigurationIs/StgctpOutRunStatusRegisters.h>

#include "NSWConfiguration/hw/DeviceManager.h"
#include "NSWConfiguration/monitoring/Helper.h"

namespace nsw::mon {
  /**
   * \brief Monitor configuration registers of the ROC
   */
  class StgctpOutRunStatusRegisters
  {
  public:
    /**
     * \brief Constructor
     *
     * \param deviceManager device manager containing HWIs
     */
    explicit StgctpOutRunStatusRegisters(const nsw::hw::DeviceManager& deviceManager);

    /**
     * \brief Monitor and publish information for all devices to IS
     *
     * \param isDict IS dictionary
     * \param serverName name of the monitoring IS server
     */
    void monitor(ISInfoDictionary* isDict, std::string_view serverName);
    static constexpr std::string_view NAME{"StgctpOutRunStatusRegisters"};

  private:
    /**
     * \brief Read data from one STGCTP and put it into IS info type
     *
     * \param feb HWI
     * \return nsw::mon::is::StgctpOutRunStatusRegisters IS info struct with values
     */
    [[nodiscard]] static nsw::mon::is::StgctpOutRunStatusRegisters getData(const nsw::hw::STGCTP& tp);

    /**
     * @brief Read HORX environment monitoring data from TP
     * 
     * @param tp TP to read from
     * @return std::uint32_t HORX environment data
     */
    [[nodiscard]] static std::uint32_t readHorxEnvMonData(const nsw::hw::STGCTP& tp);

    std::reference_wrapper<const std::vector<nsw::hw::STGCTP>> m_devices;
    constexpr static std::int64_t NUM_CONCURRENT{1};
    IPCThreadPool m_threadPool{NUM_CONCURRENT};
    internal::MonitorHelper m_helper;
  };
}  // namespace nsw::mon

#endif
