#ifndef NSWCONFIGURATION_ISSUES_H
#define NSWCONFIGURATION_ISSUES_H

#include <ers/ers.h>

#include <fmt/format.h>

ERS_DECLARE_ISSUE(nsw, NSWUnkownCommand, fmt::format("Unknown command: {}", message), ((std::string)message))
ERS_DECLARE_ISSUE(nsw, NSWInvalidCommand, fmt::format("Invalid command: {}", message), ((std::string)message))
ERS_DECLARE_ISSUE(nsw, NSWInvalidPartition, "Did not find a partition",)
ERS_DECLARE_ISSUE(nsw, NSWOpcRetryLimitReached, fmt::format("Did not manager to complete command within {} retries", nRetries), ((int)nRetries))
ERS_DECLARE_ISSUE(nsw, NSWConfigurationOpcError, fmt::format("Configuration not successful due to OPC error: {}", message), ((std::string)message))

#endif
