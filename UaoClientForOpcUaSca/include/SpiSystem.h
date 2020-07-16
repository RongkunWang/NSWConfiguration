

#include <iostream>
#include <uaclient/uaclientsdk.h>

namespace UaoClientForOpcUaSca
{

using namespace UaClientSdk;



class SpiSystem
{

public:

    SpiSystem(
        UaSession* session,
        UaNodeId objId
    );

// getters, setters for all variables


private:

    UaSession  * m_session;
    UaNodeId     m_objId;

};



}

