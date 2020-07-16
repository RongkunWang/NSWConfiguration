/*
 * demo_adc_samples.cpp
 *
 *  Created on: 6 Jun 2018
 *      Author: pnikiel
 *
 *  This small program illustrates how to get ADC consecutive samples, e.g. for NSW ADC calibration.
 */

#include <unistd.h>
#include <fstream>

// the following file is provided with UaoForQuasar
#include <ClientSessionFactory.h>

#include <XilinxFpga.h>

#include <uaplatformlayer.h>
#include <iostream>

#include <LogIt.h>

#include <algorithm>

#include <boost/program_options.hpp>

using namespace boost::program_options;
using namespace UaoClientForOpcUaSca;

std::vector<unsigned char> getFileContent(const std::string& fn)
{
    std::ifstream f(fn);
    if (!f)
    {
        std::cout << "Failed to open the file." << std::endl;
        throw std::runtime_error("file opening problem");
    }

    // how do we get the size?
    f.seekg(0, f.end);
    size_t sz = f.tellg();
    f.seekg(0, f.beg);

    std::vector<unsigned char> data (sz, 0);
    f.read(reinterpret_cast<char*>(&data[0]), sz);
    f.close();

    LOG(Log::INF) << "Read a file of " << sz << " bytes";
    return data;
}

int main( int argc, char* argv[])
{
    try
    {
        UaPlatformLayer::init();
        options_description options;
        std::string opcUaEndpoint;
        std::string xilinxObjectAddress;
        std::string logLevelStr;
        std::string bitFileName;
        Log::LOG_LEVEL       logLevel;
        options.add_options()
                    ("help,h", "show help")
                    ("endpoint,e",     value<std::string>(&opcUaEndpoint)->default_value("opc.tcp://127.0.0.1:48020"),        "OPC-UA Endpoint, e.g. opc.tcp://127.0.0.1:48020" )
                    ("address,a",      value<std::string>(&xilinxObjectAddress)->default_value("scaFelix1.jtag.fpga"),       "Address of the Voltage AI")
                    ("trace_level,t",  value<std::string>(&logLevelStr)->default_value("INF"),                               "Trace level, one of: ERR,WRN,INF,DBG,TRC")
                    ("bitfile,b",      value<std::string>(&bitFileName),                                                     "Bitfile to program"   )
                    ;

        variables_map vm;
        store( parse_command_line (argc, argv, options), vm );
        notify (vm);
        if (vm.count("help"))
        {
            std::cout << options << std::endl;
            return 0;
        }

        if (! Log::logLevelFromString( logLevelStr, logLevel ) )
        {
            std::cout << "Log level not recognized: '" << logLevelStr << "'" << std::endl;
            exit(1);
        }

        Log::initializeLogging(logLevel);

        if (bitFileName.empty())
        {
            LOG(Log::INF) << "File name not provided, exiting.";
            return -1;
        }


        UaClientSdk::UaSession session;
        UaClientSdk::SessionSecurityInfo security;

        UaClientSdk::SessionConnectInfo sessionConnectInfo;
        sessionConnectInfo.internalServiceCallTimeout = 60000; // this is important as programming might take time

        UaStatus status = session.connect(
                opcUaEndpoint.c_str(),
                sessionConnectInfo,
                security,
                new MyCallBack ()
        );

        if (status.isBad())
        {
            LOG(Log::ERR) << "Failed to connect: " << status.toString().toUtf8();
            return -1;
        }

        XilinxFpga fpga (&session, UaNodeId(xilinxObjectAddress.c_str(), 2));

        std::vector<unsigned char> data = getFileContent(bitFileName);
        UaByteString bs;
        bs.setByteString(data.size(), &data[0]);

        LOG(Log::INF) << "Starting programming - this will certainly take some time! Stay tuned.";

        fpga.program(bs);
        LOG(Log::INF) << "Finished programming";
    }
    catch (std::exception &e)
    {
        LOG(Log::ERR) << "Caught exception: " << e.what();
    }



}






