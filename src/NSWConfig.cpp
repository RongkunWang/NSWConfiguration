#include <utility>  // make_pair
#include <string>
#include <memory>

// Header to the RC online services
#include "RunControl/Common/OnlineServices.h"
#include "NSWConfiguration/NSWConfig.h"

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

    ERS_LOG("\nFollowing front ends will be configured:\n"
          <<"========================================");
    for (auto & name : frontend_names) {
      try {
        auto this_pair = std::make_pair(name, m_reader->readConfig(name));
        if      (nsw::getElementType(name) == "ADDC")          m_addcs  .emplace(this_pair);
        else if (nsw::getElementType(name) == "Router")        m_routers.emplace(this_pair);
        else if (nsw::getElementType(name) == "PadTriggerSCA") m_ptscas .emplace(this_pair);
        else if (nsw::getElementType(name) == "TP")            m_tps    .emplace(this_pair);
        else
          m_frontends.emplace(this_pair);
        std::cout << name << std::endl;
      } catch (std::exception & e) {
        std::stringstream ss;
        ss << " Skipping FE: " << name
                  << " - Problem constructing configuration due to : " << e.what() << std::endl;
        nsw::NSWConfigIssue issue(ERS_HERE, ss.str());
        ers::warning(issue);
        ERS_LOG(ss.str());
      }
    }

    configureFEBs();          // Configure all front-ends
    configureADDCs();         // Configure all ADDCs
    configureRouters();       // Configure all Routers
    configurePadTriggers();   // Configure all Pad Triggers
    configureTPs();           // Configure all Trigger processors
    alignADDCsToTP();
    ERS_LOG("End");
}

void nsw::NSWConfig::unconfigureRc() {
    ERS_INFO("Start");
    m_frontends.clear();
    m_addcs.clear();
    m_routers.clear();
    m_ptscas.clear();
    m_tps.clear();
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
        try {  // If configureFEB throws exception, it will be caught here
            thread.get();
        } catch (std::exception & ex) {
            nsw::NSWConfigIssue issue(ERS_HERE, "Configuration of FEB failed due to : " + std::string(ex.what()));
            ers::warning(issue);
        }
    }
}

void nsw::NSWConfig::configureFEB(const std::string& name) {
    // how can we avoid this?
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
        try {  // If configureADDC throws exception, it will be caught here
            thread.get();
        } catch (std::exception & ex) {
            std::string message = "Skipping ADDC due to : " + std::string(ex.what());
            nsw::NSWConfigIssue issue(ERS_HERE, message);
            ers::warning(issue);
        }
    }
}

void nsw::NSWConfig::configureADDC(const std::string& name) {
    ERS_LOG("Configuring ADDC: " + name);
    if (m_addcs.count(name) == 0) {
        std::string err = "ADDC has bad name: " + name;
        nsw::NSWConfigIssue issue(ERS_HERE, err);
        ers::error(issue);
    }
    auto local_sender = std::make_unique<nsw::ConfigSender>();
    auto configuration = m_addcs.at(name);
    if (!m_simulation)
        local_sender->sendAddcConfig(configuration);
    usleep(10000);
    ERS_LOG("Finished config to: " << name);
}

void nsw::NSWConfig::alignADDCsToTP() {
    ERS_LOG("Checking alignment of ADDCs to TP");
    m_sender->alignAddcGbtxTp(m_addcs);
    ERS_LOG("Finished checking alignment of ADDCs to TP");
}

void nsw::NSWConfig::configureRouters() {
    ERS_LOG("Configuring all Routers");
    for (auto obj : m_routers) {
        auto name = obj.first;
        auto configuration = obj.second;
        ERS_LOG("Sending config to: " << name);
        if (!m_simulation)
            m_sender->sendRouterConfig(configuration);
    }
    ERS_LOG("Finished configuration of Routers");
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
        m_sender->sendTpConfig(configuration);
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
