#include "NSWConfiguration/NSWConfig.h"

#include <utility>
#include <string>
#include <memory>

// Header to the RC online services
#include "RunControl/Common/OnlineServices.h"

#include "dal/ResourceSet.h"
#include "config/Configuration.h"
#include "ers/ers.h"

using boost::property_tree::ptree;

nsw::NSWConfig::NSWConfig(bool simulation):m_simulation {simulation} {
    ERS_LOG("Constructing NSWConfig instance");
    if (m_simulation) {
        ERS_INFO("Running in simulation mode, no configuration will be sent");
    }
}

ptree nsw::NSWConfig::getConf() {
  return m_reader->readConfig();
}

void nsw::NSWConfig::substituteConf(const ptree& tree) {
  m_reader  = std::make_unique<nsw::ConfigReader>(tree);
}

void nsw::NSWConfig::configureRc() {
    // TODO(cyildiz): Instead of reading all front ends from the database,
    // we should find the ones that are at the same links with the swROD
    auto frontend_names = m_reader->getAllElementNames();

    ERS_LOG("\nFollowing front ends will be configured now:\n");
    for (auto & name : frontend_names) {
      try {
        auto element = nsw::getElementType(name);
        ERS_LOG(name << ", an instance of " << element);
        auto this_pair = std::make_pair(name, m_reader->readConfig(name));
        if      (element == "ADDC")          m_addcs     .emplace(this_pair);
        else if (element == "Router")        m_routers   .emplace(this_pair);
        else if (element == "PadTriggerSCA") m_ptscas    .emplace(this_pair);
        else if (element == "TP")            m_tps       .emplace(this_pair);
        else if (element == "TPCarrier")     m_tpcarriers.emplace(this_pair);
        else if (element == "L1DDC")         m_l1ddcs    .emplace(this_pair);
        else
          m_frontends.emplace(this_pair);
        std::cout << name << std::endl;
      } catch (std::exception & e) {
        std::stringstream ss;
        ss << " Skipping FE: " << name
           << " - Problem constructing configuration due to : "
           << e.what() << std::endl;
        nsw::NSWConfigIssue issue(ERS_HERE, ss.str());
        ERS_INFO(ss.str());
        ers::fatal(issue);
      }
    }

    configureL1DDCs();        // Configure all l1ddc's
    configureFEBs();          // Configure all front-ends
    configureADDCs();         // Configure all ADDCs
    configureRouters();       // Configure all Routers
    configurePadTriggers();   // Configure all Pad Triggers
    configureTPs();           // Configure all Trigger processors
    configureTPCarriers();    // Configure all Trigger processor carriers
    alignADDCsToTP();         // Adjust MMTP serializers based on ART data
    ERS_LOG("End");
}

void nsw::NSWConfig::unconfigureRc() {
    ERS_INFO("Start");
    m_frontends.clear();
    m_addcs.clear();
    m_l1ddcs.clear();
    m_routers.clear();
    m_ptscas.clear();
    m_tps.clear();
    m_tpcarriers.clear();
    // m_reader.reset();
    ERS_INFO("End");
}


void nsw::NSWConfig::configureFEBs() {
    ERS_INFO("reset VMM: "   << m_resetvmm);
    ERS_INFO("reset TDS: "   << m_resettds);
    ERS_INFO("max threads: " << m_max_threads);
    m_threads->clear();
    for (const auto& kv : m_frontends) {
        while (too_many_threads())
            usleep(500000);
        m_threads->push_back(std::async(std::launch::async,
                                        &nsw::NSWConfig::configureFEB,
                                        this,
                                        kv.first));
    }
    // wait
    for (auto& thread : *m_threads) {
        try {
            thread.get();
        } catch (std::exception & ex) {
            nsw::NSWConfigIssue issue(ERS_HERE, "Configuration of FEB failed due to : " + std::string(ex.what()));
            // TODO: This should be an error
            ers::fatal(issue);
        }
    }
}

void nsw::NSWConfig::configureFEB(const std::string& name) {
    auto local_sender = std::make_unique<nsw::ConfigSender>();
    ERS_INFO("Configuring front end " + name);
    if (m_frontends.count(name) == 0) {
        std::string err = "FEB has bad name: " + name;
        nsw::NSWConfigIssue issue(ERS_HERE, err);
        ers::error(issue);
    }
    auto configuration = m_frontends.at(name);
    if (!m_simulation) {
        local_sender->sendRocConfig(configuration);
        local_sender->disableVmmCaptureInputs(configuration);
        if (m_resetvmm) {
            std::vector <unsigned> reset_ori;
            for (auto & vmm : configuration.getVmms()) {
                reset_ori.push_back(vmm.getGlobalRegister("reset"));  // Set reset bits to 1
                vmm.setGlobalRegister("reset", 3);  // Set reset bits to 1
            }
            local_sender->sendVmmConfig(configuration);

            size_t i = 0;
            for (auto & vmm : configuration.getVmms()) {
                vmm.setGlobalRegister("reset", reset_ori[i++]);  // Set reset bits to original
            }
        }
        local_sender->sendVmmConfig(configuration);

        local_sender->sendTdsConfig(configuration, m_resettds);
    }
    usleep(100000);
    ERS_LOG("Sending config to: " << name);
}

void nsw::NSWConfig::configureL1DDCs() {
    ERS_INFO("Configuring all L1DDCs");
    m_threads->clear();
    for (const auto& [name,l1ddc] : m_l1ddcs) {
        while (too_many_threads()) usleep(500000);
        // ThreadConfig cfg;
        // cfg.configure_l1ddc     = configure_l1ddc;
        m_threads->push_back(std::async(std::launch::async, &nsw::NSWConfig::configureL1DDC,this, l1ddc));
    }
    for (auto& thread : *m_threads) {
        try {  // If configureADDC throws exception, it will be caught here
            thread.get();
        } catch (std::exception & ex) {
            std::string message = "Skipping L1DDC due to : " + std::string(ex.what());
            nsw::NSWConfigIssue issue(ERS_HERE, message);
            ers::warning(issue);
        }
    }
}

void nsw::NSWConfig::configureL1DDC(const nsw::L1DDCConfig& l1ddc) {
    // Configure L1DDC
    nsw::ConfigSender cs;
    cs.sendL1DDCConfig(l1ddc);
}


void nsw::NSWConfig::configureADDCs() {
    ERS_LOG("Configuring all ADDCs");
    m_threads->clear();
    for (const auto& kv : m_addcs) {
        while (too_many_threads())
            usleep(500000);
        m_threads->push_back(std::async(std::launch::async,
                                        &nsw::NSWConfig::configureADDC,
                                        this,
                                        kv.first));
    }
    for (auto& thread : *m_threads) {
        try {
            thread.get();
        } catch (std::exception & ex) {
            std::string message = "Skipping ADDC due to : " + std::string(ex.what());
            nsw::NSWConfigIssue issue(ERS_HERE, message);
            ers::fatal(issue);
        }
    }
}

void nsw::NSWConfig::configureADDC(const std::string& name) {
    ERS_INFO("Configuring ADDC: " + name);
    if (m_addcs.count(name) == 0) {
        std::string err = "ADDC has bad name: " + name;
        nsw::NSWConfigIssue issue(ERS_HERE, err);
        ers::error(issue);
    }
    auto local_sender = std::make_unique<nsw::ConfigSender>();
    auto configuration = m_addcs.at(name);
    for (size_t iart = 0; iart < nsw::NUM_ART_PER_ADDC; iart++) {
      try {
        if (!m_simulation) {
          local_sender->sendAddcConfig(configuration, iart);
        }
        usleep(5000);
      } catch (std::exception & ex) {
        if (configuration.getART(iart).MustConfigure()) {
          throw;
        } else {
          const std::string msg = "Allowed to fail: "
            + name
            + ".art" + std::to_string(iart)
            + ": " + std::string(ex.what());
          nsw::NSWConfigIssue issue(ERS_HERE, msg);
          ers::warning(issue);
        }
      }
    }
    ERS_LOG("Finished config to: " << name);
}

void nsw::NSWConfig::alignADDCsToTP() {
    ERS_LOG("Checking alignment of ADDCs to TP");
    if (!m_simulation) {
        m_sender->alignArtGbtxMmtp(m_addcs, m_tps);
    }
    ERS_LOG("Finished checking alignment of ADDCs to TP");
}

void nsw::NSWConfig::configureRouters() {
    ERS_LOG("Configuring all Routers");
    m_threads->clear();
    for (const auto& kv : m_routers) {
        while (too_many_threads())
            usleep(500000);
        m_threads->push_back(std::async(std::launch::async,
                                        &nsw::NSWConfig::configureRouter,
                                        this,
                                        kv.first));
    }
    for (auto& thread : *m_threads) {
        try {
            thread.get();
        } catch (std::exception & ex) {
            std::string message = "Skipping Router due to : " + std::string(ex.what());
            nsw::NSWConfigIssue issue(ERS_HERE, message);
            ers::fatal(issue);
        }
    }
    ERS_LOG("Finished configuration of Routers");
}

void nsw::NSWConfig::configureRouter(const std::string& name) {
    ERS_LOG("Configuring Router: " + name);
    if (m_routers.count(name) == 0) {
        std::string err = "Router has bad name: " + name;
        nsw::NSWConfigIssue issue(ERS_HERE, err);
        ers::error(issue);
    }
    auto local_sender = std::make_unique<nsw::ConfigSender>();
    auto configuration = m_routers.at(name);
    if (!m_simulation)
        local_sender->sendRouterConfig(configuration);
    ERS_LOG("Finished config to: " << name);
}

void nsw::NSWConfig::configurePadTriggers() {
    ERS_LOG("Configuring all PadTriggerSCAs");
    for (auto obj : m_ptscas) {
        auto name = obj.first;
        auto configuration = obj.second;
        ERS_LOG("Sending config to: " << name);
        if (!m_simulation)
            m_sender->sendPadTriggerSCAConfig(configuration);
    }
    ERS_LOG("Finished configuration of PadTriggerSCAs");
}

void nsw::NSWConfig::configureTPs() {
    ERS_LOG("Configuring TPs. Total number: " << m_tps.size() );
    for (const auto& kv : m_tps) {
        auto configuration = m_tps.at(kv.first);
        if (!m_simulation) {
            m_sender->sendTPConfig(configuration);
        }
        ERS_LOG("Finished config to: " << kv.first);
    }
}

void nsw::NSWConfig::configureTPCarriers() {
    ERS_LOG("Configuring TPCarriers. Total number: " << m_tpcarriers.size() );
    for (const auto& kv : m_tpcarriers) {
        auto configuration = m_tpcarriers.at(kv.first);
        if (!m_simulation) {
            m_sender->sendTPCarrierConfig(configuration);
        }
        ERS_LOG("Finished config to: " << kv.first);
    }
}

void nsw::NSWConfig::configureVMMs() {
    ERS_INFO("Configuring VMMs");
    for (auto fe : m_frontends) {
        auto name = fe.first;
        auto configuration = fe.second;
        if (!m_simulation) {
            m_sender->sendVmmConfig(configuration);
        }
        ERS_LOG("Sending VMM config to: " << name);
    }
}

void nsw::NSWConfig::configureROCs() {
    ERS_INFO("Configuring ROCs");
    for (auto fe : m_frontends) {
        auto name = fe.first;
        auto configuration = fe.second;
        if (!m_simulation) {
            m_sender->sendRocConfig(configuration);
        }
        ERS_LOG("Sending ROC config to: " << name);
    }
}

size_t nsw::NSWConfig::active_threads() {
    size_t nfinished = 0;
    for (auto& thread : *m_threads)
        if (thread.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            nfinished++;
    return m_threads->size() - nfinished;
}

bool nsw::NSWConfig::too_many_threads() {
    size_t nthreads = active_threads();
    bool decision = (nthreads >= m_max_threads);
    if (decision) {
        std::cout << "Too many active threads ("
                  << nthreads
                  << "), waiting for fewer than "
                  << m_max_threads << std::endl;
    }
    return decision;
}

void nsw::NSWConfig::enableVmmCaptureInputs() {
    const auto func = [this] (const std::string& t_name) {
        const auto configuration = m_frontends.at(t_name);
        nsw::ConfigSender configSender;
        if (!m_simulation) {
          configSender.enableVmmCaptureInputs(configuration);
        }
    };
    m_threads->clear();
    for (const auto& kv : m_frontends) {
        while (too_many_threads())
            usleep(500000);
        m_threads->push_back(std::async(std::launch::async,
                                        func,
                                        kv.first));
    }
    // wait
    for (auto& thread : *m_threads) {
        try {
            thread.get();
        } catch (std::exception & ex) {
            nsw::NSWConfigIssue issue(ERS_HERE, "Enabling of VMMs failed due to : " + std::string(ex.what()));
            ers::fatal(issue);
        }
    }
}

void nsw::NSWConfig::disableVmmCaptureInputs() {
    const auto func = [this] (const std::string& t_name) {
        const auto configuration = m_frontends.at(t_name);
        nsw::ConfigSender configSender;
        if (!m_simulation) {
          configSender.disableVmmCaptureInputs(configuration);
        }
    };
    m_threads->clear();
    for (const auto& kv : m_frontends) {
        while (too_many_threads())
            usleep(500000);
        m_threads->push_back(std::async(std::launch::async,
                                        func,
                                        kv.first));
    }
    // wait
    for (auto& thread : *m_threads) {
        try {
            thread.get();
        } catch (std::exception & ex) {
            nsw::NSWConfigIssue issue(ERS_HERE, "Disabling of VMMs failed due to : " + std::string(ex.what()));
            ers::warning(issue);
        }
    }
}

void nsw::NSWConfig::enableMmtpChannelRates(bool enable) const {
  ERS_LOG("Enabling MMTP channel rates reporting: " << enable);
  bool quiet = true;
  nsw::ConfigSender configSender;
  for (const auto& kv : m_tps) {
    const auto config = m_tps.at(kv.first);
    try {
      if (config.EnableChannelRates()) {
        if (!m_simulation) {
          configSender.sendSCAXRegister(config, nsw::mmtp::REG_CHAN_RATE_ENABLE,
                                        static_cast<uint32_t>(enable), quiet);
        }
      } else {
        ERS_LOG("enableMmtpChannelRates skipped for " << kv.first);
      }
    } catch (std::exception & ex) {
      nsw::NSWConfigIssue issue(ERS_HERE, "enableMmtpChannelRates failed: " + std::string(ex.what()));
      ers::warning(issue);
    }
    ERS_LOG("Finished enabling " << kv.first);
  }
}

void nsw::NSWConfig::startRc() {
    enableMmtpChannelRates(true);
}

void nsw::NSWConfig::stopRc() {
    disableVmmCaptureInputs();
    enableMmtpChannelRates(false);
}

boost::property_tree::ptree nsw::NSWConfig::parseDeviceHierarchy(const std::vector<const daq::core::ResourceBase*>& contains, std::ostringstream& stream, const int level) const {
    ERS_INFO("STARTING parseDeviceHierarchy");
    for (const auto * element: contains) {
        ERS_INFO("LOOP " << element);
        element->print(level, true, stream);
        ERS_INFO("AFTER PRINT");
        auto* tmp = element->cast<daq::core::ResourceSet>();
        ERS_INFO("AFTER DYN CAST " << tmp);
        if (tmp) {
            const auto asd = tmp->get_Contains();
            ERS_INFO("CONTAINS SIZE  " << asd.size());
            [[maybe_unused]] const auto tmp2 = parseDeviceHierarchy(asd, stream, level + 2);
        }
    }
    ERS_INFO("DONE");
    return {};
}
