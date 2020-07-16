

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class AnalogInput
{

public:

    AnalogInput(
        UaSession* session,
        UaNodeId objId
    );

// getters, setters for all variables
    OpcUa_Float readValue (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);
    OpcUa_UInt16 readRawValue (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);

    void getConsecutiveRawSamples(
        OpcUa_UInt16  in_number,std::vector<OpcUa_UInt16> & out_samples

    );


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

