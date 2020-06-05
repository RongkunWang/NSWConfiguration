
#ifndef NSWCONFIGURATION_NSWCONFIG_H_
#define NSWCONFIGURATION_NSWCONFIG_H_

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <future>

#include "ers/ers.h"

#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ConfigReader.h"

using boost::property_tree::ptree;

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
    ~NSWConfig() {}

    //! Connects to configuration database/ or reads file based config database
    //! Reads the names of front ends that should be configured and constructs
    //! FEBConfig objects in the map m_frontends
    //! Template class to allow the reading of both the Config and Calib configurations
    template <class U>
    void readConf(const U* nswApp) {
      try {
        m_dbcon = nswApp->get_dbConnection();
        m_resetvmm = nswApp->get_resetVMM();
        m_resettds = nswApp->get_resetTDS();
        m_max_threads = nswApp->get_maxThreads();
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

      m_reader  = std::make_unique<nsw::ConfigReader>(m_dbcon);
      m_sender  = std::make_unique<nsw::ConfigSender>();
      m_threads = std::make_unique<std::vector< std::future<void> > >();

      auto config = m_reader->readConfig();
    }

    //! Send the configuration to the boards
    void configureRc();

    //! Retrieve the ptree configuration
    ptree getConf();

    //! Substitute the configuration ptree
    void substituteConf(ptree tree);

    void unconfigureRc();

 private:
    //! Configure ROCs in all FEBs in m_frontends
    void configureROCs();

    //! Configure VMMs in all FEBs in m_frontends
    void configureVMMs();

    //! Configure all front ends in m_frontends
    void configureFEBs();
    void configureFEB(std::string name);

    //! Count how many threads are running
    size_t active_threads();
    bool too_many_threads();

    //! Configure all ADDCs in m_addcs
    void configureADDCs();
    void configureADDC(std::string name);
    void alignADDCsToTP();

    //! Configure all Routers, Pad Triggers, and Trigger Processors
    void configureRouters();
    void configurePadTriggers();
    void configureTPs();

    std::unique_ptr<nsw::ConfigReader> m_reader;
    std::unique_ptr<nsw::ConfigSender> m_sender;

    std::map<std::string, FEBConfig>           m_frontends;   //! Each element is [frontend_name, frontend_config]
    std::map<std::string, ADDCConfig>          m_addcs;       //!
    std::map<std::string, RouterConfig>        m_routers;     //!
    std::map<std::string, PadTriggerSCAConfig> m_ptscas;      //!
    std::map<std::string, TPConfig>            m_tps;         //!

    // Database connection string
    std::string m_dbcon;

    // reset the vmms before config
    bool m_resetvmm;
    // reset the tds SER, logic, ePLL after configuration
    bool m_resettds;

    // thread management
    size_t m_max_threads;
    std::unique_ptr< std::vector< std::future<void> > > m_threads;

    // Run the program in simulation mode, don't send any configuration
    bool m_simulation;
};
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIG_H_
