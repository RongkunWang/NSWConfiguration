// Opc Client implementation

#ifndef NSWCONFIGURATION_OPCCLIENT_H_
#define NSWCONFIGURATION_OPCCLIENT_H_

#include <unistd.h>
#include <time.h>

#include <iostream>
#include <string>
#include <memory>
#include <vector>

// TODO(cyildiz): Include headers with directories to avoid confusion
// From UaoForQuasar
#include "ClientSessionFactory.h"

// Generated  files
#include "SpiSlave.h"

// From: open62541-compat
#include "uaplatformlayer.h"

#include "NSWConfiguration/OpcClient.h"

namespace nsw {

class OpcClient {

 private:
    std::string m_server_ipport;
    //std::unique_ptr<UaClientSdk::UaSession> m_session;
    //std::shared_ptr<UaClientSdk::UaSession> m_session;
    UaClientSdk::UaSession* m_session;

 public:
    /// Initialize Opc Platform Layer and creates a UaSession
    OpcClient(std::string server_ip_port);
    ~OpcClient() {};

    void writeSpiSlave(std::string node_address, std::vector<uint8_t> data);
};

}

#endif  // NSWCONFIGURATION_OPCCLIENT_H_

