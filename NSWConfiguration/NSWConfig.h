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
#include "NSWConfiguration/hw/DeviceManager.h"
#include "NSWConfiguration/monitoring/RocConfigurationRegisters.h"
#include "NSWConfiguration/monitoring/RocStatusRegisters.h"
#include "NSWConfiguration/monitoring/MmtpInRunStatusRegisters.h"
#include "NSWConfiguration/monitoring/MmtpOutRunStatusRegisters.h"
#include "NSWConfiguration/monitoring/StgctpInRunStatusRegisters.h"
#include "NSWConfiguration/monitoring/StgctpOutRunStatusRegisters.h"
#include "NSWConfiguration/monitoring/PadTriggerRegisters.h"
#include "NSWConfiguration/monitoring/CarriertpInRunStatusRegisters.h"

#include <boost/property_tree/ptree.hpp>

#include <dal/ResourceBase.h>
#include <dal/util.h>
#include <ers/ers.h>

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
        const auto jsonConfiguration = m_dbcon.find(".json") != std::string::npos;
        nsw::oks::parseDeviceMap(
          deviceHierarchy, nswApp->get_Contains(), daq::core::get_partition(conf, ""), jsonConfiguration);
        if (nswApp->get_Contains().empty()) {
          if (jsonConfiguration) {
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

    //! Exectue the connect step
    void connectRc();

    //! Retrieve the ConfigReader
    const nsw::ConfigReader& getReader() const { return *m_reader; };

    void readConfigurationResource();

    //! Substitute the configuration ptree
    void substituteConf(const boost::property_tree::ptree& tree);

    void unconfigureRc();

    //! Prepare for run transition. Enable VMMs
    void startRc();

    //! Stop recording transition. Disable VMMs (symmetry)
    void stopRc();

    //! Enable VMMs (after VMM config); used in a user() function
    void enableVmmCaptureInputs();

    //! Disable VMMs (setting register 8 to 0); symmetric to the above
    void disableVmmCaptureInputs();

    //! \see nsw::hw::DeviceManager::resetSTGCTP()
    void resetSTGCTP() { m_deviceManager.resetSTGCTP(); }

    /**
     * \brief Set the command sender to the RC application
     *
     * \param sender Command sender to RC application
     */
    void setCommandSender(nsw::CommandSender&& sender) { m_deviceManager.setCommandSender(std::move(sender)); }
    
    /**
     * \brief Recover from OPC crashes
     *
     * Try to create a connection to any board
     *
     * \return recovery successful
     */
    bool recoverOpc();

    /**
     * \brief Monitor a given group
     *
     * \param name Name of the monitoring group
     * \param isDict IS dictionary
     * \param serverName Name of the IS server
     */
    void monitor(const std::string& name, ISInfoDictionary* isDict, std::string_view serverName);

    /**
     * \brief Get the fraction of devices that failed to configure
     *
     * \return double failed fraction
     */
    double getFractionFailed() const { return m_deviceManager.getFractionFailed(); }

    /**
     * @brief Publish state to IS
     *
     * Triggered by publish function of RC app. Publishes any configuration errors
     *
     * @param isDict IS dict to publish to
     * @param isServer Name of the IS server
     */
    void publish(const ISInfoDictionary* isDict, std::string_view isServer) const;

    hw::DeviceManager& getDeviceManager() { return m_deviceManager; }
    const hw::DeviceManager& getDeviceManager() const { return m_deviceManager; }
private:
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

    //! Configure all Trigger Processors
    void configureTPs();

    std::unique_ptr<nsw::ConfigReader> m_reader;
    std::unique_ptr<nsw::ConfigSender> m_sender;

    std::map<std::string, ADDCConfig>          m_addcs;       //! Each element is [frontend_name, frontend_config]
    std::map<std::string, TPConfig>            m_tps;         //!
    std::map<std::string, L1DDCConfig>         m_l1ddcs;      //!

    hw::DeviceManager m_deviceManager;

    using MonitoringVariant = std::variant<
      nsw::mon::RocStatusRegisters, nsw::mon::RocConfigurationRegisters, 
      nsw::mon::MmtpInRunStatusRegisters, nsw::mon::MmtpOutRunStatusRegisters,
      nsw::mon::StgctpInRunStatusRegisters, nsw::mon::StgctpOutRunStatusRegisters,
      nsw::mon::PadTriggerRegisters,
      nsw::mon::CarriertpInRunStatusRegisters>;
    std::map<std::string, MonitoringVariant> m_monitoringMap;

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
