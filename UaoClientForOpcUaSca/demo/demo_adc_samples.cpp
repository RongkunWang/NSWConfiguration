/*
 * demo_adc_samples.cpp
 *
 *  Created on: 6 Jun 2018
 *      Author: pnikiel
 *
 *  This small program illustrates how to get ADC consecutive samples, e.g. for NSW ADC calibration.
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

#include <algorithm>

#include <boost/program_options.hpp>

using namespace boost::program_options;
using namespace UaoClientForOpcUaSca;

int main( int argc, char* argv[])
{
    UaPlatformLayer::init();



    options_description options;
    std::string opcUaEndpoint;
    std::string aiAddress;
    unsigned int numberSamples;
    std::string logLevelStr;
    Log::LOG_LEVEL       logLevel;
    options.add_options()
            ("help", "show help")
            ("endpoint,e",     value<std::string>(&opcUaEndpoint)->default_value("opc.tcp://127.0.0.1:48020"),  "OPC-UA Endpoint, e.g. opc.tcp://127.0.0.1:48020" )
            ("address,a",      value<std::string>(&aiAddress)->default_value("simulatedSca.ai.voltage"),       "Address of the Voltage AI")
            ("trace_level,t", value<std::string>(&logLevelStr)->default_value("INF"),                         "Trace level, one of: ERR,WRN,INF,DBG,TRC")
            ("iterations,i" , value<unsigned int>(&numberSamples)->default_value(1000),                       "Number of samples" )
            ;


    variables_map vm;
    store( parse_command_line (argc, argv, options), vm );
    notify (vm);
    if (vm.count("help"))
    {
        std::cout << options << std::endl;
        exit(1);
    }
    if (! Log::logLevelFromString( logLevelStr, logLevel ) )
    {
        std::cout << "Log level not recognized: '" << logLevelStr << "'" << std::endl;
        exit(1);
    }

    Log::initializeLogging(Log::INF);

    UaClientSdk::UaSession* session = ClientSessionFactory::connect(opcUaEndpoint.c_str());
    if (!session)
        return -1;

    AnalogInput ai (session, UaNodeId(aiAddress.c_str(), 2));

    std::vector<OpcUa_UInt16> samples;
    ai.getConsecutiveRawSamples(numberSamples, samples);

    std::for_each(samples.begin(), samples.end(), [](OpcUa_UInt16 f){std::cout << f << std::endl;});

    ServiceSettings sessset = ServiceSettings();
    session->disconnect(sessset, OpcUa_True);
    delete session;
}






