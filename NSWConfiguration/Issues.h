#ifndef NSWCONFIGURATION_ISSUES_H
#define NSWCONFIGURATION_ISSUES_H

#include <ers/ers.h>

namespace nsw {
    ERS_DECLARE_ISSUE(nsw, NSWUnkownCommand, "Unknown command: " << message, ((std::string)message))
} // namespace nsw

#endif
