

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class DigitalIOSystem
{

public:

    DigitalIOSystem(
        UaSession* session,
        UaNodeId objId
    );

// getters, setters for all variables
    UaString readDiagnostics (
        UaStatus     *out_status=nullptr,
        UaDateTime   *sourceTimeStamp=nullptr,
        UaDateTime   *serverTimeStamp=nullptr);


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

