#ifndef NSWCONFIGURATION_ISSUES_H
#define NSWCONFIGURATION_ISSUES_H

#include <ers/ers.h>

#include <fmt/format.h>

ERS_DECLARE_ISSUE(nsw,
                  NSWUnkownCommand,
                  fmt::format("Unknown command: {}", message),
                  ((std::string)message))
ERS_DECLARE_ISSUE(nsw,
                  NSWInvalidCommand,
                  fmt::format("Invalid command: {}", message),
                  ((std::string)message))
ERS_DECLARE_ISSUE(nsw, NSWInvalidPartition, "Did not find a partition", )
ERS_DECLARE_ISSUE(nsw,
                  NSWOpcRetryLimitReached,
                  fmt::format("Did not manage to complete command within {} retries", nRetries),
                  ((std::uint64_t)nRetries))
ERS_DECLARE_ISSUE(nsw,
                  NSWOpcErrorIgnored,
                  fmt::format("Ignored OPC errors as configured in OKS. Sector is NOT configured properly"),
                  )
ERS_DECLARE_ISSUE(nsw,
                  NSWConfigurationError,
                  fmt::format("Configuration of {:.2f}% failed. {}", fraction * 100, message),
                  ((double)fraction)((std::string)message))
ERS_DECLARE_ISSUE(nsw,
                  NSWAppNotFound,
                  fmt::format("Did not find application with class {} in segment {}",
                              className,
                              segmentName),
                  ((std::string)className)((std::string)segmentName))

#endif
