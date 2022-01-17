#include "NSWConfiguration/NSWConfig.h"
#include "NSWConfiguration/hw/PadTrigger.h"

#include <utility>
#include <string>
#include <memory>

// Header to the RC online services
#include "NSWConfiguration/hw/DeviceManager.h"
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
    const auto frontend_names = m_reader->getAllElementNames();

    ERS_LOG("The following front ends will be configured now:\n");
    for (const auto& name : frontend_names) {
      try {
        const auto element = nsw::getElementType(name);
        ERS_LOG(name << ", an instance of " << element);
        auto this_pair = std::make_pair(name, m_reader->readConfig(name));
        if      (element == "ADDC")          { m_addcs.emplace(this_pair); }
        else if (element == "Router")        { m_deviceManager.add(RouterConfig{this_pair.second}); }
        else if (element == "PadTrigger")    { m_deviceManager.add(nsw::hw::PadTrigger{this_pair.second}); }
        else if (element == "TP")            { m_tps.emplace(this_pair); }
        else if (element == "TPCarrier")     { m_deviceManager.add(TPCarrierConfig{this_pair.second}); }
        else if (element == "L1DDC")         { m_l1ddcs.emplace(this_pair); }
        else {
          m_deviceManager.add(FEBConfig{this_pair.second});
        }
      } catch (const std::exception& e) {
        nsw::NSWConfigIssue issue(ERS_HERE, fmt::format("Problem constructing configuration due to : {}", e.what()));
        ers::fatal(issue);
      }
    }

    configureL1DDCs();        // Configure all l1ddc's
    if (!m_simulation) {
        std::vector<nsw::hw::DeviceManager::Options> result{hw::DeviceManager::Options::DISABLE_VMM_CAPTURE_INPUTS};
        if (m_resetvmm) {
            result.push_back(hw::DeviceManager::Options::RESET_VMM);
        }
        if (m_resettds) {
            result.push_back(hw::DeviceManager::Options::RESET_TDS);
        }
        m_deviceManager.configure(result);  // Configure all FEBs, Routers, and TPCarriers
    }
    configureADDCs();         // Configure all ADDCs
    configureTPs();           // Configure all Trigger processors
    alignADDCsToTP();         // Adjust MMTP serializers based on ART data
    ERS_LOG("End");
}

void nsw::NSWConfig::unconfigureRc() {
    ERS_INFO("Start");
    m_addcs.clear();
    m_l1ddcs.clear();
    m_pts.clear();
    m_tps.clear();
    // m_reader.reset();
    ERS_INFO("End");
}

void nsw::NSWConfig::configureL1DDCs() {
    ERS_INFO("Configuring all L1DDCs");
    m_threads->clear();
    for (const auto& [name,l1ddc] : m_l1ddcs) {
        while (too_many_threads()) usleep(500000);
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
    ERS_INFO("Configuring L1DDC " + l1ddc.getName());
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
      if(configuration.getART(iart).SkipConfigure()){
          nsw::NSWConfigIssue issue(ERS_HERE, fmt::format("Skipping config: {0}.art{1}", name, iart));
          ers::warning(issue);
         continue;
      }

      try {
        if (!m_simulation) {
          local_sender->sendAddcConfig(configuration, static_cast<int>(iart));
        }
        usleep(5000);
      } catch (std::exception & ex) {
        if (configuration.getART(iart).MustConfigure()) {
          throw;
        } else {
          nsw::NSWConfigIssue issue(ERS_HERE, fmt::format("Allowed to fail: {0}.art{1}: {2}", name, iart, ex.what()));
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
    m_deviceManager.enableVmmCaptureInputs();
}

void nsw::NSWConfig::disableVmmCaptureInputs() {
    m_deviceManager.disableVmmCaptureInputs();
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
