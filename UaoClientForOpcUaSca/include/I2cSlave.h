

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class I2cSlave
{

public:

    I2cSlave(
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
        OpcUa_Byte  in_numberOfBytes,UaByteString & out_reply

    );

    void writeSlave(
        const UaByteString &   in_payload

    );


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

