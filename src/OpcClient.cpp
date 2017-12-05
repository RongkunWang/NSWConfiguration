#include <memory>

#include "NSWConfiguration/OpcClient.h"

nsw::OpcClient::OpcClient(std::string server_ip_port): m_server_ipport(server_ip_port) {

    // TODO(cyildiz): Does this need to be moved to a higher level?
    // Can we have multiple init() in the same application?
    UaPlatformLayer::init();

    std::string opc_connection = "opc.tcp://" + server_ip_port;
    
     m_session = ClientSessionFactory::connect(opc_connection.c_str());
     //UaClientSdk::UaSession* session = ClientSessionFactory::connect("opc.tcp://pcatlnswdev01.cern.ch:4841");
     //m_session = std::make_shared<UaClientSdk::UaSession> (session);
     //std::unique_ptr<UaClientSdk::UaSession> unq = 

}

// vector may not be the best option
void nsw::OpcClient::writeSpiSlave(std::string node_address, std::vector<uint8_t> vdata){
    SpiSlave ss(m_session, UaNodeId(node_address.c_str(), 2));

    auto data = &vdata[0]; // get pointer to array
    UaByteString bs;
    bs.setByteString(vdata.size(), data);

    try {
        ss.writeWrite(bs);
        std::cout << "Written value to vmm" << std::endl;
    } catch (const std::exception& e) {
        // TODO(cyildiz) handle exception properly
        std::cout << "Can't write SpiSlave: " <<  e.what() << std::endl;
        return;
    }
    
}
