
#ifndef NSWCONFIGURATION_NSWCONFIGRC_H_
#define NSWCONFIGURATION_NSWCONFIGRC_H_

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <future>

#include "RunControl/RunControl.h"
#include "RunControl/Common/RunControlCommands.h"

#include "NSWConfiguration/ConfigSender.h"
#include "NSWConfiguration/ConfigReader.h"

using boost::property_tree::ptree;

namespace nsw {
class FEBConfig;
}

namespace nsw {
class NSWConfigRc: public daq::rc::Controllable {
 public:
    // override only the needed methods
    explicit NSWConfigRc(bool simulation = false);
    virtual ~NSWConfigRc() noexcept {}

    //! Connects to configuration database/ or reads file based config database
    //! Reads the names of front ends that should be configured and constructs
    //! FEBConfig objects in the map m_frontends
    void configure(const daq::rc::TransitionCmd& cmd) override;

    void prepareForRun(const daq::rc::TransitionCmd& cmd) override;

    void stopRecording(const daq::rc::TransitionCmd& cmd) override;

    void unconfigure(const daq::rc::TransitionCmd& cmd) override;

    void user(const daq::rc::UserCmd& cmd) override;

    // void onExit(daq::rc::FSM_STATE) noexcept override;

    //! Used to syncronize ROC/VMM configuration
    void subTransition(const daq::rc::SubTransitionCmd&) override;

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

    // Run the program in simulation mode, don't send any configuration
    bool m_simulation;

    // Database connection string
    std::string m_dbcon;

    // reset the vmms before config
    bool m_resetvmm;
    // reset the tds SER, logic, ePLL after configuration
    bool m_resettds;

    // thread management
    size_t m_max_threads;
    std::unique_ptr< std::vector< std::future<void> > > m_threads;

};
}  // namespace nsw
#endif  // NSWCONFIGURATION_NSWCONFIGRC_H_
