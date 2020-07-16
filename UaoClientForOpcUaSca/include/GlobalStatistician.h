

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class GlobalStatistician
{

public:

    GlobalStatistician(
        UaSession* session,
        UaNodeId objId
    );

// getters, setters for all variables
    OpcUa_Float readRequestRate (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

