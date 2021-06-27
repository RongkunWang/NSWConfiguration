#ifndef NSWCONFIGURATION_HW_OPCMANAGER_H
#define NSWCONFIGURATION_HW_OPCMANAGER_H

#include <map>
#include <shared_mutex>
#include <string>

#include "NSWConfiguration/OpcClient.h"

namespace nsw {
  class OpcManager
  {
    static std::map<std::string, OpcClientPtr> m_clients;
    static std::shared_mutex m_mutex;

  public:
    [[nodiscard]] static OpcClientPtr getConnection(
      const std::string& opcserver_ipport);

    static void addConnection(const std::string& opcserver_ipport);

    static bool exists(const std::string& opcserver_ipport);
  };
}  // namespace nsw

#endif
