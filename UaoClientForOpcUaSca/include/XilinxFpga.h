

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class XilinxFpga
{

public:

    XilinxFpga(
        UaSession* session,
        UaNodeId objId
    );

// getters, setters for all variables
    OpcUa_UInt32 readIdcodeNumeric (
        UaStatus     *out_status=nullptr,
        UaDateTime   *sourceTimeStamp=nullptr,
        UaDateTime   *serverTimeStamp=nullptr);
    UaString readIdcodeString (
        UaStatus     *out_status=nullptr,
        UaDateTime   *sourceTimeStamp=nullptr,
        UaDateTime   *serverTimeStamp=nullptr);

    void program(
        const UaByteString &   in_bitstring

    );


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

