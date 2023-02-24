#ifndef NSWCONFIGURATION_MONITORING_ISPUBLISHER_H
#define NSWCONFIGURATION_MONITORING_ISPUBLISHER_H

#include <cstdint>
#include <string>
#include <map>

#include <is/infodictionary.h>

namespace nsw::mon {
  class ISPublisher
  {
  public:
    /**
     * \brief Publish data to IS
     *
     * Publishes a dictionary of values to IS with the key
     * <serverName>.<deviceType>.<groupName>.<deviceName>
     *
     * \param isDict IS dictionary
     * \param serverName IS server name
     * \param groupName name of the monitoring group
     * \param deviceType device type
     * \param deviceName device name
     * \param values IS info struct
     */
    template<typename IsType>
    static void publish(ISInfoDictionary* isDict,
                        const std::string_view serverName,
                        const std::string_view groupName,
                        const std::string_view deviceType,
                        const std::string_view deviceName,
                        const IsType& values)
    {
      isDict->checkin(buildKey(serverName, groupName, deviceType, deviceName), values);
    }

  private:
    /**
     * \brief Builds the kedy for the IS dict
     *
     * Format: <serverName>.<deviceType>.<groupName>.<deviceName>
     *
     * \param serverName IS server name
     * \param groupName name of the monitoring group
     * \param deviceType device type
     * \param deviceName device name
     * \return std::string key for IS
     */
    static std::string buildKey(std::string_view serverName,
                                std::string_view groupName,
                                std::string_view deviceType,
                                std::string_view deviceName);
  };
}  // namespace nsw::mon

#endif
