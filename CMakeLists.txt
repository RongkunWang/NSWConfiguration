cmake_minimum_required(VERSION 3.14.0)

### compile_commands.json DB
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(CheckIPOSupported)
set(IPO_SUPPORTED)
check_ipo_supported(RESULT IPO_SUPPORTED)
message(STATUS "  IPO_SUPPORTED::${IPO_SUPPORTED}")

add_subdirectory(extern)

tdaq_package() #This will make a package called NSWConfiguration
set(TDAQ_DB_PROJECT muons)

# set(OPC_OPEN62541_PATH ${open62541-compat_SOURCE_DIR} "")
# set(UAOCLIENTFOROPCUASCA_PATH ${uaoclientforopcuasca_SOURCE_DIR} "")

remove_definitions(-DERS_NO_DEBUG)

tdaq_generate_dal(schema/NSWConfig.schema.xml
  NAMESPACE nsw::dal
  INCLUDE_DIRECTORIES DFConfiguration dal
  INCLUDE NSWConfigurationDal
  CPP_OUTPUT dal_cpp_srcs)

# Build a C++ library out of the generated files
tdaq_add_library(nswconfigdal DAL
  ${dal_cpp_srcs}
  LINK_LIBRARIES tdaq::config tdaq::daq-core-dal tdaq::daq-df-dal)

tdaq_add_library(NSWConfig src/NSWConfig.cpp
  LINK_LIBRARIES nswconfig tdaq::daq-df-dal tdaq::rc_ItemCtrl
                 tdaq-common::ers Boost::program_options nswconfigdal nswhwinterface
)

## Executables linked against Boost::boost
set(NSWCONFIG_EXE_WITH_BOOST generate_config_json)

foreach(appsrc IN LISTS NSWCONFIG_EXE_WITH_BOOST)
  tdaq_add_executable(${appsrc} app/${appsrc}.cpp
    LINK_LIBRARIES Boost::boost tdaq-common::ers
  )
endforeach()

## Executables linked against nswconfig, nswhwinterface, and Boost::program_options
set(NSWCONFIG_EXE_WITH_BOOST_PO
  configure_frontend
  configure_gbtx
  read_sca_info
  roc_register_readout
  vmm_capture_status
  read_tds_continous
  read_vmm_adc
  read_baseline_with_scope
  calibrate_trimmers
  read_channel_trimmers
  read_channel_thresholds
  check_gpio
  roc_loop_over_phase
  roc_select_phase
  addc_skeleton
  tp_comm_test
  mmtp_loopback_test
  configure_tp
  configure_carrier
  configure_rim_l1ddc
  )

## Executables linked against nswconfig and Boost::program_options, but renamed to nsw_<X>
set(NSWCONFIG_EXE_WITH_BOOST_PO_RENAMED
  pad_trigger       # nsw_pad_trigger
  pad_repeaters     # nsw_pad_repeaters
  router            # nsw_router
  tds_frame2Router  # nsw_tds_frame2Router
  )

foreach(appsrc IN LISTS NSWCONFIG_EXE_WITH_BOOST_PO)
  tdaq_add_executable(${appsrc} app/${appsrc}.cpp
    LINK_LIBRARIES nswconfig
    LINK_LIBRARIES nswhwinterface
    LINK_LIBRARIES Boost::program_options
  )
endforeach()

foreach(appsrc IN LISTS NSWCONFIG_EXE_WITH_BOOST_PO_RENAMED)
  tdaq_add_executable(nsw_${appsrc} app/${appsrc}.cpp
    LINK_LIBRARIES nswconfig
    LINK_LIBRARIES nswhwinterface
    LINK_LIBRARIES Boost::program_options
  )
endforeach()

## Executables linked against nswconfig, Boost::program_options, tdaq::Alti, and tdaq::RCDCmemSegment
set(NSWCONFIG_EXE_WITH_BOOST_PO_ALTI
  trigger_loop_mm
  )

foreach(appsrc IN LISTS NSWCONFIG_EXE_WITH_BOOST_PO_ALTI)
  tdaq_add_executable(nsw_${appsrc} "app/${appsrc}.cpp"
    LINK_LIBRARIES nswconfig
    LINK_LIBRARIES Boost::program_options
    LINK_LIBRARIES tdaq::AltiModule tdaq::AltiConfiguration tdaq::RCDCmemSegment
    )
endforeach()

tdaq_add_library(nswconfig src/ConfigReader.cpp src/ConfigReaderApi.cpp
                 src/ConfigReaderJsonApi.cpp
                 src/Utility.cpp src/ConfigSender.cpp
                 src/SCAConfig.cpp
                 src/I2cMasterConfig.cpp
                 src/GBTxConfig.cpp
                 src/VMMCodec.cpp src/VMMConfig.cpp
                 src/L1DDCConfig.cpp
                 src/FEBConfig.cpp src/ARTConfig.cpp
                 src/ADDCConfig.cpp src/PadTriggerSCAConfig.cpp
                 src/TPConfig.cpp src/TPCarrierConfig.cpp
                 src/RouterConfig.cpp
                 src/ConfigConverter.cpp
                 src/OKSDeviceHierarchy.cpp
  LINK_LIBRARIES Boost::boost
                 nswconfigdal
                 nswopcclient
                 tdaq-common::ers
                 ic_comms
                 $<BUILD_INTERFACE:fmt::fmt-header-only>
)

tdaq_add_library(nswhwinterface
                #  src/hw/DeviceManager.cpp # Placeholder for future
                 src/hw/OpcManager.cpp
                 src/hw/SCAInterface.cpp
                 src/hw/ROC.cpp
                 src/hw/VMM.cpp
                 src/hw/TDS.cpp
                 src/hw/ART.cpp
                 src/hw/PadTrigger.cpp
                 src/hw/Router.cpp
                 src/hw/TP.cpp
                 src/hw/TPCarrier.cpp
  LINK_LIBRARIES nswconfig
    PRIVATE
                 $<BUILD_INTERFACE:fmt::fmt-header-only>
)

tdaq_add_executable(NSWConfigRc_main app/NSWConfigRc_main.cpp src/NSWConfigRc.cpp
  LINK_LIBRARIES nswconfig tdaq::daq-df-dal tdaq::rc_ItemCtrl
                 tdaq-common::ers Boost::program_options nswconfigdal NSWConfig
)

tdaq_add_schema(schema/NSWConfig.schema.xml)

# Testing

set(NSWCONFIG_TEST_DATA test_vmm.json test_jsonapi.json TP_testRegisterConfig.json)
foreach(datafile IN LISTS NSWCONFIG_TEST_DATA)
  configure_file(test/${datafile} ${datafile} COPYONLY)

  # ## goes to share/${TDAQ_PACKAGE_NAME}/
  # tdaq_add_data(test/${datafile})
  ## goes to share/${TDAQ_PACKAGE_NAME}/tests
  tdaq_add_data(test/${datafile} DESTINATION tests)
endforeach()

tdaq_add_executable(test_jsonapi test/test_jsonapi.cpp
  NOINSTALL
  LINK_LIBRARIES Boost::unit_test_framework tdaq-common::ers nswconfig)

tdaq_add_executable(test_vmmconfig test/test_vmmconfig.cpp
  NOINSTALL
  LINK_LIBRARIES Boost::unit_test_framework  tdaq-common::ers nswconfig)

tdaq_add_executable(test_configreader test/test_configreader.cpp
  NOINSTALL
  LINK_LIBRARIES Boost::unit_test_framework tdaq-common::ers nswconfig)

tdaq_add_executable(test_utility test/test_utility.cpp src/Utility.cpp
  NOINSTALL
  LINK_LIBRARIES Boost::unit_test_framework  tdaq-common::ers
  PRIVATE $<BUILD_INTERFACE:fmt::fmt-header-only>)

tdaq_add_executable(test_i2cmasterconfig test/test_i2cmasterconfig.cpp src/I2cMasterConfig.cpp src/Utility.cpp
  NOINSTALL
  LINK_LIBRARIES Boost::unit_test_framework tdaq-common::ers
  PRIVATE $<BUILD_INTERFACE:fmt::fmt-header-only>)

tdaq_add_executable(test_configtranslation test/test_configtranslation.cpp
  NOINSTALL
  LINK_LIBRARIES Boost::unit_test_framework tdaq-common::ers nswconfig
  PRIVATE $<BUILD_INTERFACE:fmt::fmt-header-only>)

### Tests
set(NSWCONFIG_TESTS jsonapi configreader i2cmasterconfig utility vmmconfig configtranslation)

foreach(testname IN LISTS NSWCONFIG_TESTS)
  message(STATUS "  Adding test::add_test(NAME ${testname} COMMAND test_${testname})")
  add_test(NAME ${testname} COMMAND test_${testname})
endforeach()

tdaq_add_library(nswopcclient
    src/OpcClient.cpp
  LINK_LIBRARIES
      tdaq-common::ers
      UaoClient::UaoClientForOpcUaSca
      Open62541Compat::open62541-compat
      rt
    PRIVATE
      $<BUILD_INTERFACE:fmt::fmt-header-only>
)

if(IPO_SUPPORTED)
  message(STATUS "  Enabling IPO for nswopcclient and nswconfig")
  set_target_properties(nswopcclient nswconfig
    PROPERTIES
      INTERPROCEDURAL_OPTIMIZATION ON
  )
endif()

tdaq_add_executable(demo_opc_client_class app/demo_opc_client_class.cpp
    LINK_LIBRARIES nswopcclient
)
