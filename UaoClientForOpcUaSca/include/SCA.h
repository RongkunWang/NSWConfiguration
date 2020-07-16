

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class SCA
{

public:

    SCA(
        UaSession* session,
        UaNodeId objId
    );

// getters, setters for all variables
    UaString readAddress (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);
    OpcUa_Boolean readOnline (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);
    OpcUa_UInt32 readId (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);
    OpcUa_UInt64 readNumberRequests (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);
    OpcUa_UInt64 readNumberReplies (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);
    OpcUa_UInt64 readLastReplySecondsAgo (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);
    OpcUa_Double readRequestRate (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);
    OpcUa_UInt64 readNumberLostReplies (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);
    OpcUa_Float readLostRepliesRate (
        UaStatus      *out_status=nullptr,
        UaDateTime    *sourceTimeStamp=nullptr,
        UaDateTime    *serverTimeStamp=nullptr);

    void reset(


    );

    void ping(
        OpcUa_Boolean & out_pong

    );


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

