tdaq_package() #This will make a package called NSWConfiguration

tdaq_generate_dal(schema/NSWConfig.schema.xml
  NAMESPACE nsw::dal
  INCLUDE_DIRECTORIES DFConfiguration dal 
  INCLUDE NSWConfigurationDal
  CPP_OUTPUT dal_cpp_srcs)

# Build a C++ library out of the generated files
tdaq_add_library(nswconfigdal DAL
  ${dal_cpp_srcs}
  LINK_LIBRARIES tdaq::config tdaq::daq-core-dal tdaq::daq-df-dal)

tdaq_add_executable(generate_config_json app/generate_config_json.cpp
  LINK_LIBRARIES Boost::boost
)

tdaq_add_executable(configure_frontend app/configure_frontend.cpp
  LINK_LIBRARIES nswconfig Boost::program_options
)

tdaq_add_executable(nsw_integration_pfeb_config app/integration_pfeb_config.cpp
  LINK_LIBRARIES nswconfig Boost::program_options
)

tdaq_add_executable(check_gpio app/check_gpio.cpp
  LINK_LIBRARIES nswconfig Boost::program_options
)

tdaq_add_executable(roc_loop_over_phase app/roc_loop_over_phase.cpp
  LINK_LIBRARIES nswconfig Boost::program_options
)

tdaq_add_executable(roc_select_phase app/roc_select_phase.cpp
  LINK_LIBRARIES nswconfig Boost::program_options
)

tdaq_add_library(nswconfig src/ConfigReader.cpp src/ConfigReaderApi.cpp
                 src/Utility.cpp src/ConfigSender.cpp
                 src/FEConfig.cpp
                 src/I2cMasterConfig.cpp
                 src/VMMCodec.cpp src/VMMConfig.cpp
                 src/FEBConfig.cpp
  LINK_LIBRARIES Boost::boost
                 nswopcclient
                 tdaq-common::ers
)

tdaq_add_executable(nsw_read_config app/read_config.cpp
  LINK_LIBRARIES nswconfig
)

tdaq_add_executable(nsw_integration_roc_config app/integration_roc_config.cpp
  LINK_LIBRARIES nswconfig
)

tdaq_add_executable(nsw_integration_addc_config app/integration_addc_config.cpp
  LINK_LIBRARIES nswconfig
)

tdaq_add_executable(nsw_roc_tests app/roc_tests.cpp
  LINK_LIBRARIES nswconfig
)

tdaq_add_executable(NSWConfigRc_main app/NSWConfigRc_main.cpp src/NSWConfigRc.cpp
  LINK_LIBRARIES nswconfig tdaq::daq-df-dal tdaq::rc_ItemCtrl
                 tdaq-common::ers Boost::program_options nswconfigdal
)

tdaq_add_schema(schema/NSWConfig.schema.xml)

# Testing

configure_file(test/test_vmm.json test_vmm.json COPYONLY)
configure_file(test/test_configreader.json test_configreader.json COPYONLY)

tdaq_add_executable(test_vmmconfig test/test_vmmconfig.cpp
      LINK_LIBRARIES Boost::unit_test_framework  tdaq-common::ers nswconfig)

tdaq_add_executable(test_configreader test/test_configreader.cpp
      LINK_LIBRARIES Boost::unit_test_framework tdaq-common::ers nswconfig)


add_test(NAME vmmconfig COMMAND test_vmmconfig)

add_test(NAME configreader COMMAND test_configreader)

tdaq_add_executable(nsw_test_utility test/test_utility.cpp src/Utility.cpp
      LINK_LIBRARIES Boost::unit_test_framework  tdaq-common::ers)

add_test(NAME utility COMMAND nsw_test_utility)

tdaq_add_executable(test_i2cmasterconfig test/test_i2cmasterconfig.cpp src/I2cMasterConfig.cpp src/Utility.cpp
      LINK_LIBRARIES Boost::unit_test_framework tdaq-common::ers)

add_test(NAME test_i2cmasterconfig COMMAND test_i2cmasterconfig)

# Generated headers and ClientSessionFactory.h from submodule
include_directories(UaoClientForOpcUaSca/include)

# Directory of open source OPCUA SDK
# https://github.com/quasar-team/open62541-compat.git
# branch: origin/OPCUA-887a_client_extensions
if (DEFINED ENV{OPC_OPEN62541_PATH})
    set(OPC_OPEN62541_PATH $ENV{OPC_OPEN62541_PATH})
    message("OPC_OPEN62541_PATH: " ${OPC_OPEN62541_PATH})
else()
    message( FATAL_ERROR "OPC_OPEN62541_PATH environment variable not defined!" )
endif()

include_directories(${OPC_OPEN62541_PATH}/include)
include_directories(${OPC_OPEN62541_PATH}/build/open62541/)

file(GLOB OPC_GENERATED_CLIENT_SRCS UaoClientForOpcUaSca/src/*.cpp)

tdaq_add_library(nswopcclient
        src/OpcClient.cpp
        UaoClientForOpcUaSca/src/ClientSessionFactory.cpp
        ${OPC_GENERATED_CLIENT_SRCS}
  LINK_LIBRARIES
        Boost::boost
        tdaq-common::ers
        ${OPC_OPEN62541_PATH}/build/libopen62541-compat.so
        ${OPC_OPEN62541_PATH}/build/open62541/libopen62541.a
        rt
)

tdaq_add_executable(demo_opc_client
        app/demo_opc_client.cpp
        UaoClientForOpcUaSca/src/ClientSessionFactory.cpp
        ${OPC_GENERATED_CLIENT_SRCS}
    LINK_LIBRARIES
        ${OPC_OPEN62541_PATH}/build/libopen62541-compat.so
        ${OPC_OPEN62541_PATH}/build/open62541/libopen62541.a
        rt
)

tdaq_add_executable(demo_opc_client_class app/demo_opc_client_class.cpp
    LINK_LIBRARIES nswopcclient
)
