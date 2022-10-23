#ifndef NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_HELPERS_H
#define NSWCONFIGURATION_NSWCONFIGURATION_MONITORING_HELPERS_H

#include <cstddef>
#include <execution>

#include <is/infodictionary.h>
#include <semaphore>

#include "NSWConfiguration/Concepts.h"
#include "NSWConfiguration/monitoring/IsPublisher.h"

namespace nsw::mon::internal {
  /** @brief A simple lock for a semaphore
   */
  template<typename SemaphoreType>
  class SemaphoreGuard
  {
  public:
    explicit SemaphoreGuard(SemaphoreType& semaphore) : m_semaphore{semaphore}
    {
      m_semaphore.acquire();
    }

    ~SemaphoreGuard() { m_semaphore.release(); }

    SemaphoreGuard(const SemaphoreGuard&) = delete;
    SemaphoreGuard& operator=(const SemaphoreGuard&) = delete;
    SemaphoreGuard(SemaphoreGuard&&) noexcept = default;
    SemaphoreGuard& operator=(SemaphoreGuard&&) noexcept = default;

  private:
    SemaphoreType& m_semaphore;
  };

  /**
   * @brief Helper do monitor boards and publish the result to IS
   */
  class MonitorHelper
  {
  public:
    /**
     * @brief Constructor
     *
     * @param numConcurrent Number of boards monitored concurrently
     */
    explicit MonitorHelper(std::int64_t numConcurrent) : m_semaphore{numConcurrent} {}

    /**
     * \brief Loop over all devices and publish result to IS
     *
     * \tparam T HW interface type
     * \param hwis Set of devices to be monitored
     * \param isDict IS dictionary (from partition)
     * \param serverName Name of the IS monitoring server
     * \param groupName Name of the monitoring group
     * \param func Function that fills the IS object for each device
     */
    template<nsw::HWI T>
    void monitorAndPublish(const std::vector<T>& hwis,
                           ISInfoDictionary* isDict,
                           const std::string_view serverName,
                           const std::string_view groupName,
                           const std::regular_invocable<T> auto& func) const
    {
      std::vector<std::future<void>> threads{};
      threads.reserve(std::size(hwis));
      for (const auto& device : hwis) {
        threads.push_back(std::async(std::launch::async,
                                     [this, &isDict, &serverName, &groupName, &func, &device]() {
                                       doWork(device, isDict, serverName, groupName, func);
                                     }));
      }
      for (auto& thread : threads) {
        thread.get();
      }
    }

  private:
    /**
     * \brief Monitor a single device and publish to IS
     *
     * \tparam T HW interface type
     * \param device device to be monitored
     * \param isDict IS dictionary (from partition)
     * \param serverName Name of the IS monitoring server
     * \param groupName Name of the monitoring group
     * \param func Function that fills the IS object for each device
     */
    template<nsw::HWI T>
    void doWork(const T& device,
                ISInfoDictionary* isDict,
                const std::string_view serverName,
                const std::string_view groupName,
                const std::regular_invocable<T> auto& func) const
    {
      auto guard = SemaphoreGuard(m_semaphore);
      try {
        const auto values = func(device);
        ISPublisher::publish(isDict,
                             serverName,
                             groupName,
                             nsw::getElementType(device.getScaAddress()),
                             device.getScaAddress(),
                             values);
      } catch (const std::exception& ex) {
        ERS_LOG("Monitoring failed due to " << ex.what());
      }
    }

    static constexpr std::ptrdiff_t MIN_MAX_THREADS{200};
    mutable std::counting_semaphore<MIN_MAX_THREADS> m_semaphore;
  };
}  // namespace nsw::mon::internal

#endif
