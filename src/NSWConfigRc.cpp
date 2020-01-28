#include <utility>  // make_pair
#include <string>
#include <memory>

// Header to the RC online services
#include "RunControl/Common/OnlineServices.h"

#include "NSWConfiguration/NSWConfigRc.h"
#include "NSWConfigurationDal/NSWConfigApplication.h"

nsw::NSWConfigRc::NSWConfigRc(bool simulation):m_simulation {simulation} {
    ERS_LOG("Constructing NSWConfigRc instance");
    if (m_simulation) {
        ERS_INFO("Running in simulation mode, no configuration will be sent");
    }
}

void nsw::NSWConfigRc::configure(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("Start");

    daq::rc::OnlineServices& rcSvc = daq::rc::OnlineServices::instance();

    try {
      const daq::core::RunControlApplicationBase& rcBase = rcSvc.getApplication();
      const nsw::dal::NSWConfigApplication* nswConfigApp = rcBase.cast<nsw::dal::NSWConfigApplication>();
      m_dbcon = nswConfigApp->get_dbConnection();
      m_resetvmm = nswConfigApp->get_resetVMM();
      m_max_threads = nswConfigApp->get_maxThreads();
      ERS_INFO("DB Connection: " << m_dbcon);
    } catch(std::exception& ex) {
        std::stringstream ss;
        ss << "Problem reading OKS configuration of NSWConfigRc: " << ex.what();
        nsw::NSWConfigIssue issue(ERS_HERE, ss.str());
        ers::fatal(issue);
    }

    m_reader  = std::make_unique<nsw::ConfigReader>(m_dbcon);
    m_sender  = std::make_unique<nsw::ConfigSender>();
    m_threads = std::make_unique<std::vector< std::future<void> > >();

    auto config = m_reader->readConfig();

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

    configureFEBs();        // Configure all front-ends
    configureADDCs();       // Configure all ADDCs
    configureRouters();     // Configure all Routers
    configurePadTriggers(); // Configure all Pad Triggers
    configureTPs();         // Configure all Trigger processors
    alignADDCsToTP();
    ERS_LOG("End");
}

void nsw::NSWConfigRc::unconfigure(const daq::rc::TransitionCmd& cmd) {
    ERS_INFO("Start");
    m_frontends.clear();
    m_addcs.clear();
    m_routers.clear();
    m_ptscas.clear();
    m_tps.clear();
    // m_reader.reset();
    ERS_INFO("End");
}


void nsw::NSWConfigRc::prepareForRun(const daq::rc::TransitionCmd& cmd) {
    ERS_LOG("Start");
    ERS_LOG("End");
}

void nsw::NSWConfigRc::stopRecording(const daq::rc::TransitionCmd& cmd) {
    ERS_LOG("Start");
    ERS_LOG("End");
}

void nsw::NSWConfigRc::user(const daq::rc::UserCmd& usrCmd) {
  ERS_LOG("User command received: " << usrCmd.commandName());
}

void nsw::NSWConfigRc::subTransition(const daq::rc::SubTransitionCmd& cmd) {
    auto main_transition = cmd.mainTransitionCmd();
    auto sub_transition = cmd.subTransition();

    ERS_LOG("Sub transition received: " << sub_transition << " (mainTransition: " << main_transition << ")");

    // This part should be in sync with NSWTTCConfig application. Some of this steps can also be a regular
    // state transition instead of a subTransition. SubTransitions are called before the main transition
    // This is not used in current software version, it may be used if one requires to configure different
    // boards at different times, instead of configuring everything at "configure" step.
    /*if (sub_transition == "CONFIGURE_ROC") {
      // configureROCs();
    } else if (sub_transition == "CONFIGURE_VMM") {
      // configureVMMs();
    } else {
      ERS_LOG("Nothing to do for subTransition" << sub_transition);
    }*/
}

void nsw::NSWConfigRc::configureFEBs() {
    m_threads->clear();
    for (const auto& kv: m_frontends) {
        while(too_many_threads())
            usleep(500000);
        m_threads->push_back(std::async(std::launch::async,
                                        &nsw::NSWConfigRc::configureFEB,
                                        this,
                                        kv.first));
    }
    // wait
    for (auto& thread: *m_threads) {
        try {  // If configureFEB throws exception, it will be caught here
            thread.get();
        } catch (std::exception & ex) {
            nsw::NSWConfigIssue issue(ERS_HERE, "Configuration of FEB failed due to : " + std::string(ex.what()));
            ers::warning(issue);
        }
    }
}

void nsw::NSWConfigRc::configureFEB(std::string name) {

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

        if (m_resetvmm)
        {
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

        local_sender->sendTdsConfig(configuration);
    }
    usleep(100000);
    ERS_LOG("Sending config to: " << name);
}

void nsw::NSWConfigRc::configureADDCs() {
    ERS_LOG("Configuring all ADDCs");
    m_threads->clear();
    for (const auto& kv: m_addcs) {
        while(too_many_threads())
            usleep(500000);
        m_threads->push_back(std::async(std::launch::async,
                                        &nsw::NSWConfigRc::configureADDC,
                                        this,
                                        kv.first));
    }
    for (auto& thread: *m_threads) {
        try {  // If configureADDC throws exception, it will be caught here
            thread.get();
        } catch (std::exception & ex) {
            std::string message = "Skipping ADDC due to : " + std::string(ex.what());
            nsw::NSWConfigIssue issue(ERS_HERE, message);
            ers::warning(issue);
        }
    }
}

void nsw::NSWConfigRc::configureADDC(std::string name) {
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

void nsw::NSWConfigRc::alignADDCsToTP() {
    ERS_LOG("Checking alignment of ADDCs to TP");
    m_sender->alignAddcGbtxTp(m_addcs);
    ERS_LOG("Finished checking alignment of ADDCs to TP");
}

void nsw::NSWConfigRc::configureRouters() {
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

void nsw::NSWConfigRc::configurePadTriggers() {
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

void nsw::NSWConfigRc::configureTPs() {
    ERS_LOG("Configuring TPs. Total number: " << m_tps.size() );
    for (const auto& kv: m_tps) {
        auto configuration = m_tps.at(kv.first);
        m_sender->sendTpConfig(configuration);
        ERS_LOG("Finished config to: " << kv.first);
    }
}

void nsw::NSWConfigRc::configureVMMs() {
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

void nsw::NSWConfigRc::configureROCs() {
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

size_t nsw::NSWConfigRc::active_threads() {
    size_t nfinished = 0;
    for (auto& thread: *m_threads)
        if (thread.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
            nfinished++;
    return m_threads->size() - nfinished;
}

bool nsw::NSWConfigRc::too_many_threads() {
    size_t nthreads = active_threads();
    bool decision = (nthreads >= m_max_threads);
    if(decision){
        std::cout << "Too many active threads ("
                  << nthreads
                  << "), waiting for fewer than "
                  << m_max_threads << std::endl;
    }
    return decision;
}
