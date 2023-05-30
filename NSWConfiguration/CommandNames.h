#ifndef NSWCONFIGURATION_COMMANDNAMES_H
#define NSWCONFIGURATION_COMMANDNAMES_H

#include <string_view>

namespace nsw::commands {
  constexpr static std::string_view STOP{"stop"};
  constexpr static std::string_view START{"start"};
  // the CONFIGURE of all main devices in a subTransition in Configure transition
  constexpr static std::string_view CONFIGURE{"configure"};
  // reset the stgc tp in another subTransition in Configure transition
  constexpr static std::string_view RESET_STGCTP{"resetStgcTp"};
  constexpr static std::string_view CONNECT{"connect"};
  constexpr static std::string_view UNCONFIGURE{"unconfigure"};
  constexpr static std::string_view ENABLE_VMM{"enableVmmCaptureInputs"};
  constexpr static std::string_view RECONNECT_OPC{"reconnectOpc"};
  constexpr static std::string_view MONITOR{"monitor"};
  constexpr static std::string_view SCA_DISCONNECTED{"scaServiceUnavailable"};
  constexpr static std::string_view SCA_RECONNECTED{"scaServiceReconnected"};
  constexpr static std::string_view RECOVER_OPC{"recoverOpc"};
  constexpr static std::string_view RECOVER_OPC_MESSAGE{"recoverOpcAndMessage"};
  constexpr static std::string_view MON_IS_SERVER_NAME{"monitoringIsServerName"};
}  // namespace nsw::commands

#endif
