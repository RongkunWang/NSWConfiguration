

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class DacOutput
{

public:

    DacOutput(
        UaSession* session,
        UaNodeId objId
    );

// getters, setters for all variables
    OpcUa_Double readVoltage (
        UaStatus     *out_status=nullptr,
        UaDateTime   *sourceTimeStamp=nullptr,
        UaDateTime   *serverTimeStamp=nullptr);

    void writeVoltage (
        OpcUa_Double & data,
        UaStatus                                 *out_status=nullptr);


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

