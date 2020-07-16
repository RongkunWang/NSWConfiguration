

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class SpiSlave
{

public:

    SpiSlave(
        UaSession* session,
        UaNodeId objId
    );

// getters, setters for all variables
    UaByteString readValue (
        UaStatus     *out_status=nullptr,
        UaDateTime   *sourceTimeStamp=nullptr,
        UaDateTime   *serverTimeStamp=nullptr);

    void writeValue (
        UaByteString & data,
        UaStatus                                 *out_status=nullptr);

    void readSlave(
        OpcUa_UInt32  in_numberOfChunks,UaByteString & out_reply

    );

    void writeSlave(
        const UaByteString &   in_payload

    );


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

