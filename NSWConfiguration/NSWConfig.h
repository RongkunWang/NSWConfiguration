#ifndef NSWCONFIGURATION_NSWCONFIG_H_
#define NSWCONFIGURATION_NSWCONFIG_H_

#include <sstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <future>

#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ConfigReader.h"
#include "NSWConfiguration/OKSDeviceHierarchy.h"
#include "NSWConfiguration/Types.h"

#include <boost/property_tree/ptree.hpp>

#include "dal/ResourceBase.h"
#include "dal/util.h"
#include "ers/ers.h"

ERS_DECLARE_ISSUE(nsw,
                  NSWConfigIssue,
                  message,
                  ((std::string)message)
                  )

namespace nsw {
class FEBConfig;
}

namespace nsw {
class NSWConfig {
 public:
    // override only the needed methods
    explicit NSWConfig(bool simulation);
    ~NSWConfig() = default;

    //! Connects to configuration database/ or reads file based config database
    //! Reads the names of front ends that should be configured and constructs
    //! FEBConfig objects in the map m_frontends
    //! Template class to allow the reading of both the Config and Calib configurations
    template <class U>
    void readConf(const U* nswApp) {
      auto deviceHierarchy            = nsw::oks::initDeviceMap();
      try {
        m_dbcon = nswApp->get_dbConnection();
        m_resetvmm = nswApp->get_resetVMM();
        m_resettds = nswApp->get_resetTDS();
        m_max_threads = nswApp->get_maxThreads();
        ERS_INFO("Read device hierarchy");
        auto conf = Configuration("");
        [[maybe_unused]] const auto val = nsw::oks::parseDeviceMap(
          deviceHierarchy, nswApp->get_Contains(), nswApp->class_name(), daq::core::get_partition(conf, ""));
        if (nswApp->get_Contains().empty()) {
          if (m_dbcon.find(".json") != std::string::npos) {
            ERS_INFO("NSWConfig application is empty. Configure everything in JSON.");
          } else {
            ers::warning(NSWConfigIssue(ERS_HERE, "NSWConfig application is empty. Nothing will be configured."));
          }
          deviceHierarchy = {};
        }
        ERS_INFO("DB Configuration: " << m_dbcon);
        ERS_INFO("Reset VMM: "   << m_resetvmm);
        ERS_INFO("Reset TDS: "   << m_resettds);
        ERS_INFO("max threads: " << m_max_threads);
      } catch(std::exception& ex) {
          std::stringstream ss;
          ss << "Problem reading OKS configuration of NSWConfig: " << ex.what();
          nsw::NSWConfigIssue issue(ERS_HERE, ss.str());
          ers::fatal(issue);
      }

      m_reader  = std::make_unique<nsw::ConfigReader>(m_dbcon, deviceHierarchy);
      m_sender  = std::make_unique<nsw::ConfigSender>();
      m_threads = std::make_unique<std::vector<std::future<void> > >();

      auto config = m_reader->readConfig();
    }

    //! Send the configuration to the boards
    void configureRc();

    //! Retrieve the ptree configuration
    boost::property_tree::ptree getConf();

    //! Substitute the configuration ptree
    void substituteConf(const boost::property_tree::ptree& tree);

    void unconfigureRc();

    //! Prepare for run transition. Enable VMMs
    void startRc();

    //! Stop recording transition. Disable VMMs (symmetry)
    void stopRc();

    //! Enable VMMs (after VMM config)
    void enableVmmCaptureInputs();

    //! Disable VMMs (setting register 8 to 0)
    void disableVmmCaptureInputs();

    //! Enable MMTP channel rates reporting
    void enableMmtpChannelRates(bool enable) const;

private:
    //! Configure ROCs in all FEBs in m_frontends
    void configureROCs();

    //! Configure VMMs in all FEBs in m_frontends
    void configureVMMs();

    //! Configure all front ends in m_frontends
    void configureFEBs();
    void configureFEB(const std::string& name);

    //! Count how many threads are running
    size_t active_threads();
    bool too_many_threads();

    //! Configure all ADDCs in m_addcs
    void configureADDCs();
    void configureADDC(const std::string& name);
    void alignADDCsToTP();

    //! Configure L1DDC's
    void configureL1DDCs();
    void configureL1DDC(const nsw::L1DDCConfig& l1ddc);

    //! Configure all Routers, Pad Triggers, and Trigger Processors
    void configureRouters();
    void configureRouter(const std::string& name);
    void configurePadTriggers();
    void configureTPs();
    void configureTPCarriers();

    std::unique_ptr<nsw::ConfigReader> m_reader;
    std::unique_ptr<nsw::ConfigSender> m_sender;

    std::map<std::string, FEBConfig>           m_frontends;   //! Each element is [frontend_name, frontend_config]
    std::map<std::string, ADDCConfig>          m_addcs;       //!
    std::map<std::string, RouterConfig>        m_routers;     //!
    std::map<std::string, PadTriggerSCAConfig> m_ptscas;      //!
    std::map<std::string, TPConfig>            m_tps;         //!
    std::map<std::string, TPCarrierConfig>     m_tpcarriers;  //!
    std::map<std::string, L1DDCConfig>         m_l1ddcs;      //!

    // Database connection string
    std::string m_dbcon;

    // reset the vmms before config
    bool m_resetvmm;
    // reset the tds SER, logic, ePLL after configuration
    bool m_resettds;

    // thread management
    size_t m_max_threads;
    std::unique_ptr<std::vector<std::future<void> > > m_threads;

    // Run the program in simulation mode, don't send any configuration
    bool m_simulation;

};
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIG_H_
