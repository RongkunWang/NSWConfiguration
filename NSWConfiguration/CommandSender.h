#ifndef NSWCONFIGURATION_COMMANDSENDER_H
#define NSWCONFIGURATION_COMMANDSENDER_H

#include <string>

#include <fmt/format.h>
#include <fmt/ranges.h>

#include <ers/ers.h>
#include <RunControl/Common/CommandSender.h>
#include <RunControl/Common/RunControlCommands.h>

namespace nsw {
  class CommandSender
  {
  public:
    CommandSender() = default;
    CommandSender(std::string name, std::unique_ptr<daq::rc::CommandSender>&& sender) :
      m_name{std::move(name)}, m_commandSender{std::move(sender)}
    {}
    void send(const std::string_view command, const std::vector<std::string>& args = {}) const
    {
      ERS_LOG(fmt::format("Sending message '{}' to '{}': args {}", command,  m_name, args));
      m_commandSender->sendCommand(m_name, daq::rc::UserCmd{std::string{command}, args});
    }
    void send(const std::string_view command, const unsigned long timeout) const
    {
      ERS_LOG(fmt::format("Sending message '{}' to '{}': timeout {}", command, m_name, timeout));
      auto userCmd = daq::rc::UserCmd{std::string{command}, {}};
      m_commandSender->sendCommand(m_name, userCmd, timeout);
    }
    void send(const std::string_view command,
              const std::vector<std::string>& args,
              const unsigned long timeout) const
    {
      ERS_LOG(fmt::format("Sending message '{}' to '{}': timeout {}, args {}", command,  m_name, timeout, args));
      auto userCmd = daq::rc::UserCmd{std::string{command}, args};
      m_commandSender->sendCommand(m_name, userCmd, timeout);
    }
    [[nodiscard]] bool valid() const { return m_commandSender != nullptr; }

  private:
    std::string m_name{};
    std::unique_ptr<daq::rc::CommandSender> m_commandSender{};
  };
}  // namespace nsw

#endif
