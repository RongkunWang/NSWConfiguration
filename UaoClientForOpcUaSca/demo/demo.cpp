/*
 * demo.cpp
 *
 *  Created on: 25 Oct 2017
 *      Author: pnikiel
 *
 *    This is a small demo program
 *    that illustrates how you can use your
 *    generated client.
 */
#include <unistd.h>

// the following file is provided with UaoForQuasar
#include <ClientSessionFactory.h>

// the following comes from generation
// you might need to use another class
#include <AnalogInput.h>
#include <SpiSlave.h>

#include <uaplatformlayer.h>
#include <iostream>

#include <LogIt.h>

using namespace UaoClientForOpcUaSca;

int main()
{
    UaPlatformLayer::init();

    Log::initializeLogging(Log::INF);

    UaClientSdk::UaSession* session = ClientSessionFactory::connect("opc.tcp://127.0.0.1:48020");
    if (!session)
        return -1;

    AnalogInput ai (session, UaNodeId("fscaFelix1.ai.voltage", 2));
    SpiSlave ss (session, UaNodeId("scaFelix1.spi.vmm0x", 2));

    UaByteString bs;
    OpcUa_Byte data[] = {0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x03, 0x03, 0x03};

    bs.setByteString( sizeof data, data);



    for (unsigned int i=0; i<100; ++i)
    {
        UaDateTime sourceTime;
        UaStatus s;
        float value;
        //value = ai.readValue(/*status*/&s, /*sourcetime*/nullptr, /*servertime*/ &sourceTime);
        std::cout << "value=" << value << ", ts=" << sourceTime.toString().toUtf8() << std::endl;
        ss.writeValue(bs, nullptr);
        //usleep(1000000);
    }

    ServiceSettings sessset = ServiceSettings();
    session->disconnect(sessset, OpcUa_True);
    delete session;
}




