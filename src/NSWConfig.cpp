#include "NSWConfiguration/NSWConfig.h"
#include "NSWConfiguration/OpcClient.h"
#include "NSWConfiguration/TPConstants.h"
#include "NSWConfiguration/hw/FEB.h"
#include "NSWConfiguration/hw/PadTrigger.h"

#include <thread>
#include <utility>
#include <string>
#include <memory>
#include <chrono>
#include <variant>

// Header to the RC online services
#include "NSWConfiguration/hw/DeviceManager.h"
#include "NSWConfiguration/monitoring/RocStatusRegisters.h"
#include "NSWConfiguration/monitoring/RocConfigurationRegisters.h"
#include "RunControl/Common/OnlineServices.h"

#include "dal/ResourceSet.h"
#include "config/Configuration.h"
#include "ers/ers.h"

using boost::property_tree::ptree;
using namespace std::chrono_literals;

nsw::NSWConfig::NSWConfig(bool simulation):m_simulation {simulation} {
    ERS_LOG("Constructing NSWConfig instance");
    if (m_simulation) {
        ERS_INFO("Running in simulation mode, no configuration will be sent");
    }
}

void nsw::NSWConfig::substituteConf(const ptree& tree) {
  m_reader  = std::make_unique<nsw::ConfigReader>(tree);
}

void nsw::NSWConfig::readConfigurationResource() {
    // TODO(cyildiz): Instead of reading all front ends from the database,
    // we should find the ones that are at the same links with the swROD
    const auto frontend_names = m_reader->getAllElementNames();

    ERS_LOG("The following front ends will be configured now:\n");
    for (const auto& name : frontend_names) {
      try {
        const auto element = nsw::getElementType(name);
        // Skip L1DDC for now. TODO: Remove this once implemented properly
        if (element == "L1DDC" or element == "RimL1DDC") {
          ERS_LOG(fmt::format("Skipping {}, an instance of {}", name, element));
          continue;
        }
        ERS_LOG(name << ", an instance of " << element);
        auto this_pair = std::make_pair(name, m_reader->readConfig(name));
        if      (element == "ADDC")          { m_addcs.emplace(this_pair); }
        else if (element == "Router")        { m_deviceManager.add(this_pair.second); }
        else if (element == "PadTrigger")    { m_deviceManager.add(this_pair.second); }
        else if (element == "MMTP")          { m_deviceManager.add(this_pair.second); }
        else if (element == "STGCTP")        { m_deviceManager.add(this_pair.second); }
        else if (element == "TPCarrier")     { m_deviceManager.add(this_pair.second); }
        else if (element == "L1DDC")         { m_l1ddcs.emplace(this_pair); }
        else {
          m_deviceManager.add(this_pair.second);
        }
      } catch (const std::exception& e) {
        nsw::NSWConfigIssue issue(ERS_HERE, fmt::format("Problem constructing configuration ({}) due to : {}", name, e.what()));
        ers::fatal(issue);
      }
    }
    m_monitoringMap.try_emplace(std::string{nsw::mon::RocStatusRegisters::NAME},
                                std::in_place_type<nsw::mon::RocStatusRegisters>,
                                m_deviceManager);
    m_monitoringMap.try_emplace(std::string{nsw::mon::RocConfigurationRegisters::NAME},
                                std::in_place_type<nsw::mon::RocConfigurationRegisters>,
                                m_deviceManager);
    m_monitoringMap.try_emplace(std::string{nsw::mon::MmtpInRunStatusRegisters::NAME},
                                std::in_place_type<nsw::mon::MmtpInRunStatusRegisters>,
                                m_deviceManager);
    m_monitoringMap.try_emplace(std::string{nsw::mon::MmtpOutRunStatusRegisters::NAME},
                                std::in_place_type<nsw::mon::MmtpOutRunStatusRegisters>,
                                m_deviceManager);
    m_monitoringMap.try_emplace(std::string{nsw::mon::StgctpInRunStatusRegisters::NAME},
                                std::in_place_type<nsw::mon::StgctpInRunStatusRegisters>,
                                m_deviceManager);
    m_monitoringMap.try_emplace(std::string{nsw::mon::StgctpOutRunStatusRegisters::NAME},
                                std::in_place_type<nsw::mon::StgctpOutRunStatusRegisters>,
                                m_deviceManager);
    m_monitoringMap.try_emplace(std::string{nsw::mon::PadTriggerRegisters::NAME},
                                std::in_place_type<nsw::mon::PadTriggerRegisters>,
                                m_deviceManager);
    m_monitoringMap.try_emplace(std::string{nsw::mon::CarriertpInRunStatusRegisters::NAME},
                                std::in_place_type<nsw::mon::CarriertpInRunStatusRegisters>,
                                m_deviceManager);
}

void nsw::NSWConfig::configureRc() {
    configureL1DDCs();        // Configure all l1ddc's
    configureADDCs();         // Configure all ADDCs with ConfigSender
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
    configureTPs();           // Configure all Trigger processors
    alignADDCsToTP();         // Adjust MMTP serializers based on ART data
    ERS_LOG("End");
}

void nsw::NSWConfig::connectRc() {
  ERS_INFO("Start");
  m_deviceManager.connect();
  ERS_INFO("End");
}

void nsw::NSWConfig::unconfigureRc() {
    ERS_INFO("Start");
    m_addcs.clear();
    m_l1ddcs.clear();
    m_tps.clear();
    m_deviceManager.clear();
    m_monitoringMap.clear();
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
            ers::error(issue);
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


void nsw::NSWConfig::startRc() {
  m_deviceManager.enableMmtpChannelRates(true);
}

void nsw::NSWConfig::stopRc() {
    disableVmmCaptureInputs();
    m_deviceManager.enableMmtpChannelRates(false); 
    m_deviceManager.toggleIdleStateHigh();
}

bool nsw::NSWConfig::recoverOpc() {
  ERS_INFO("Trying to reconnect...");
  m_deviceManager.clearOpc();
  const auto pingServer = [this] () {
    const auto ping = [] (const auto& device) {
      return device.ping() == nsw::hw::ScaStatus::REACHABLE;
    };
    if (std::size(m_deviceManager.getFebs()) != 0) {
      return ping(m_deviceManager.getFebs().at(0).getRoc());
    }
    if (std::size(m_deviceManager.getArts()) != 0) {
      return ping(m_deviceManager.getArts().at(0));
    }
    // if (std::size(m_deviceManager.getTps()) != 0) {
    //   return ping(m_deviceManager.getTps().at(0));
    // }
    if (std::size(m_deviceManager.getRouters()) != 0) {
      return ping(m_deviceManager.getRouters().at(0));
    }
    if (std::size(m_deviceManager.getPadTriggers()) != 0) {
      return ping(m_deviceManager.getPadTriggers().at(0));
    }
    if (std::size(m_deviceManager.getTpCarriers()) != 0) {
      return ping(m_deviceManager.getTpCarriers().at(0));
    }
    return true;
  };

  return pingServer();
}

void nsw::NSWConfig::monitor(const std::string& name, ISInfoDictionary* isDict, const std::string_view serverName)
{
  std::visit([&isDict, &serverName] (auto& mon) mutable { mon.monitor(isDict, serverName); }, m_monitoringMap.at(name));
}

void nsw::NSWConfig::publish(const ISInfoDictionary* isDict, std::string_view isServer) const
{
  m_deviceManager.publishConfigurationErrors(isDict, isServer);
}
