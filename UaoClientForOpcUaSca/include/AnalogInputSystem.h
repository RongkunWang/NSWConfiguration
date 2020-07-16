

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class AnalogInputSystem
{

public:

    AnalogInputSystem(
        UaSession* session,
        UaNodeId objId
    );

// getters, setters for all variables
    OpcUa_Double readGeneralRefreshRate (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);

    void writeGeneralRefreshRate (
        OpcUa_Double & data,
        UaStatus                                 *out_status=nullptr);
    UaString readDiagnostics (
        UaStatus     *out_status=nullptr,
        UaDateTime   *sourceTimeStamp=nullptr,
        UaDateTime   *serverTimeStamp=nullptr);


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

