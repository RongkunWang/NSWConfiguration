

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class ScaSupervisor
{

public:

    ScaSupervisor(
        UaSession* session,
        UaNodeId objId
    );

// getters, setters for all variables
    OpcUa_UInt16 readNumberOffline (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

