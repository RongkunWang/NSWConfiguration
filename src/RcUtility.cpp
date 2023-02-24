#include "NSWConfiguration/RcUtility.h"

#include <config/Configuration.h>
#include <dal/BaseApplication.h>
#include <dal/Segment.h>
#include <ers/ers.h>
#include <RunControl/Common/OnlineServices.h>

#include "NSWConfiguration/Issues.h"

std::string nsw::findSegmentSiblingApp(const std::string& className)
{
  daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();
  for (const auto* app : rcSvc.getSegment().get_all_applications()) {
    if (app->class_name() == className) {
      return app->UID();
    }
  }
  ers::fatal(nsw::NSWAppNotFound(ERS_HERE, className, rcSvc.getSegment().UID()));
  return "";
}

std::string nsw::extractSectorIdFromApp(const std::string& appName)
{
  constexpr static std::string_view DELIMITER{"-"};
  return appName.substr(0, appName.rfind(DELIMITER));
}