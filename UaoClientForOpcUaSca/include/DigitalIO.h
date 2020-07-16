

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class DigitalIO
{

public:

    DigitalIO(
        UaSession* session,
        UaNodeId objId
    );

// getters, setters for all variables
    OpcUa_Boolean readValue (
        UaStatus     *out_status=nullptr,
        UaDateTime   *sourceTimeStamp=nullptr,
        UaDateTime   *serverTimeStamp=nullptr);

    void writeValue (
        OpcUa_Boolean & data,
        UaStatus                                 *out_status=nullptr);


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

