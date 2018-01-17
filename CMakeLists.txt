tdaq_package() #This will make a package called NSWConfiguration

# This may be useful for compatibility with external packages such as OPC client
SET(LCG_TAG $ENV{LCGPLAT} )

MESSAGE(STATUS "LCG Software Tag: ${LCG_TAG}")

tdaq_add_executable(read_roc app/read_roc.cpp
  LINK_LIBRARIES Boost::boost
)

tdaq_add_library(nswconfig src/ConfigReader.cpp src/ConfigReaderApi.cpp
                 src/Utility.cpp src/ConfigSender.cpp
                 src/FEConfig.cpp
                 src/I2cFECodec.cpp src/I2cFEConfig.cpp
                 src/VMMCodec.cpp src/VMMConfig.cpp
                 src/ROCCodec.cpp src/ROCConfig.cpp
  LINK_LIBRARIES Boost::boost
                 nswopcclient
)

tdaq_add_executable(nsw_read_config app/read_config.cpp
  LINK_LIBRARIES nswconfig
)

tdaq_add_executable(NSWConfigRc_main app/NSWConfigRc_main.cpp src/NSWConfigRc.cpp
  LINK_LIBRARIES nswconfig tdaq::daq-df-dal tdaq::rc_ItemCtrl
                 tdaq-common::ers Boost::program_options
)

tdaq_add_schema(schema/NSWConfig.schema.xml)

# Testing

tdaq_add_executable(nsw_test_vmmcodec test/test_vmmcodec.cpp
      LINK_LIBRARIES Boost::unit_test_framework)

add_test(NAME vmmcodec COMMAND nsw_test_vmmcodec)

tdaq_add_executable(nsw_test_utility test/test_utility.cpp src/Utility.cpp
      LINK_LIBRARIES Boost::unit_test_framework)

add_test(NAME utility COMMAND nsw_test_utility)

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

file(GLOB OPC_GENERATED_CLIENT_SRCS UaoClientForOpcUaSca/src/*.cpp)

tdaq_add_library(nswopcclient
        src/OpcClient.cpp
        UaoClientForOpcUaSca/src/ClientSessionFactory.cpp
        ${OPC_GENERATED_CLIENT_SRCS}
  LINK_LIBRARIES
        ${OPC_OPEN62541_PATH}/build/libopen62541-compat.so
        ${OPC_OPEN62541_PATH}/open62541/build/libopen62541.a
        rt
)

tdaq_add_executable(demo_opc_client
        app/demo_opc_client.cpp
        UaoClientForOpcUaSca/src/ClientSessionFactory.cpp
        ${OPC_GENERATED_CLIENT_SRCS}
    LINK_LIBRARIES
        ${OPC_OPEN62541_PATH}/build/libopen62541-compat.so
        ${OPC_OPEN62541_PATH}/open62541/build/libopen62541.a
        rt
)

tdaq_add_executable(demo_opc_client_class app/demo_opc_client_class.cpp
    LINK_LIBRARIES nswopcclient
)
