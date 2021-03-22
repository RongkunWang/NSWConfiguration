#include <string>

#include "boost/optional.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/json_parser.hpp"

#include "NSWConfiguration/TPConfig.h"


nsw::TPConfig::TPConfig(const ptree& config) :
SCAConfig(config),
m_numMasters(NUM_REGISTER_FILES) {
    // restructureConfig();

    for (int i = 0; i < m_numMasters; i++) {
        std::string mastername = registerFilesNamesArr[i];
        if (config.find(mastername) != config.not_found()) {
            // make map pair or register file object (alloc memory first) and it's index name
            ERS_DEBUG(3, "creating object: m_registerFiles[" << i << "] : " << registerFilesNamesArr[i]);
            m_registerFiles.insert(std::make_pair(registerFilesNamesArr[i],
                new I2cMasterConfig(config.get_child(registerFilesNamesArr[i]),
                    registerFilesNamesArr[i], registerFilesArr[i]) ));
        } else {
            // register file name was declred but not found in ptree
            ERS_DEBUG(3, "mastername[" << mastername << "] : " <<
                registerFilesNamesArr[i] + ".bus" + std::to_string(i) << " not found!!");
        }
    }
}

void nsw::TPConfig::setRegisterValue(const std::string& master, const std::string& slave, uint32_t value,
    const std::string& register_name) {
    // set value of a register which salve of is on a register file (master)
    m_registerFiles[master]->setRegisterValue(slave, register_name, value);

    return;
}

uint32_t nsw::TPConfig::getRegisterValue(const std::string& master, const std::string& slave,
    const std::string& register_name) const {
    // get value of a register which salve of is on a register file (master)
    if (m_registerFiles.find(master) == m_registerFiles.end()) {
        // not sure if this is the right error.
        nsw::MissingI2cAddress issue(ERS_HERE, master.c_str());
        ers::error(issue);
        throw issue;
    }
    return m_registerFiles.at(master)->getRegisterValue(slave, register_name);
}

void nsw::TPConfig::dump() const {
    for (int i = 0; i < m_numMasters; i++) {
        if (m_registerFiles.find(registerFilesNamesArr[i]) == m_registerFiles.end() ||
            !m_registerFiles.at(registerFilesNamesArr[i])) {
            ERS_DEBUG(3, "Nothing found in register file: " << registerFilesNamesArr[i]);
        }
        else {
            m_registerFiles.at(registerFilesNamesArr[i])->dump();
        }
    }
    boost::property_tree::write_json(std::cout, m_config);

    return;
}

void nsw::TPConfig::restructureConfig() {
    ptree m_config_copy;
    m_config_copy.put("OpcServerIp", m_config.get_child("OpcServerIp").data());
    m_config_copy.put("OpcNodeId", m_config.get_child("OpcNodeId").data());

    ptree master_tree;
    // loop over masters
    for (int i = 0; i < getNumMasters(); i++) {
        master_tree = m_config.get_child(registerFilesNamesArr[i]);
        std::string registerName;
        std::string value;
        // loop over registers in master
        for (ptree::iterator pos = master_tree.begin(); pos != master_tree.end(); pos++) {
            if (master_tree.empty()) {
                ERS_DEBUG(3, "Nothing found in register file: " << registerFilesNamesArr[i]);
            } else {
                registerName = pos->first;
                value = pos->second.data();
                m_config_copy.put(registerFilesNamesArr[i] + ".bus"+std::to_string(i)+"." +
                    registerName + ".register", value);
            }
        }
    }
    m_config = m_config_copy;
    boost::property_tree::write_json(std::cout, m_config);
}


int nsw::TPConfig::ARTWindowCenter() const {
    return m_config.get<int>("ARTWindowCenter");
}
int nsw::TPConfig::ARTWindowLeft() const {
    return m_config.get<int>("ARTWindowLeft");
}
int nsw::TPConfig::ARTWindowRight() const {
    return m_config.get<int>("ARTWindowRight");
}
uint32_t nsw::TPConfig::FiberBCOffset() const {
    return m_config.get<uint32_t>("FiberBCOffset");
}
int nsw::TPConfig::GlobalInputPhase() const {
    return m_config.get<int>("GlobalInputPhase");
}
int nsw::TPConfig::GlobalInputOffset() const {
    return m_config.get<int>("GlobalInputOffset");
}
int nsw::TPConfig::SelfTriggerDelay() const {
    return m_config.get<int>("SelfTriggerDelay");
}
bool nsw::TPConfig::EnableVmmMasking() const {
    return m_config.get<bool>("EnableVmmMasking");
}

void nsw::TPConfig::setARTWindowCenter(int val) {
  m_config.put("ARTWindowCenter", val);
}
void nsw::TPConfig::setARTWindowLeft(int val) {
  m_config.put("ARTWindowLeft", val);
}
void nsw::TPConfig::setARTWindowRight(int val) {
  m_config.put("ARTWindowRight", val);
}
void nsw::TPConfig::setFiberBCOffset(uint32_t val) {
  m_config.put("FiberBCOffset", val);
}
void nsw::TPConfig::setGlobalInputPhase(int val) {
  m_config.put("GlobalInputPhase", val);
}
void nsw::TPConfig::setGlobalInputOffset(int val) {
  m_config.put("GlobalInputOffset", val);
}
void nsw::TPConfig::setSelfTriggerDelay(int val) {
  m_config.put("SelfTriggerDelay", val);
}
void nsw::TPConfig::setEnableVmmMasking(bool val) {
  m_config.put("EnableVmmMasking", val);
}

nsw::TPConfig::~TPConfig() {
    // deallocate all memory taken in the constructor
    for (int i = 0; i < m_numMasters; i++) {
        ERS_DEBUG(3, "deallocate object: m_registerFiles[" << i << "] : " << registerFilesNamesArr[i]  <<
            " object address = " << static_cast<void*>(m_registerFiles[registerFilesNamesArr[i]]));
        delete m_registerFiles[registerFilesNamesArr[i]];
    }

    return;
}

