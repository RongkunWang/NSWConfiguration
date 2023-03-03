cmake_minimum_required(VERSION 3.14.0)

### compile_commands.json DB
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(CheckIPOSupported)
set(IPO_SUPPORTED)
check_ipo_supported(RESULT IPO_SUPPORTED)
message(STATUS "  IPO_SUPPORTED::${IPO_SUPPORTED}")

add_subdirectory(extern)

tdaq_package()
set(TDAQ_DB_PROJECT muons)

find_package(ROOT REQUIRED COMPONENTS Core Tree RIO)

find_package(libgit2 QUIET)
if(NOT libgit2_FOUND)
  find_package(PkgConfig)
  find_path(LIBGIT2_PC_DIR NAMES lib64
    HINTS
      "${LCG_RELEASE_DIR}/libgit2/*/${BINARY_TAG}"
  )

  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${LIBGIT2_PC_DIR}")
  set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${LIBGIT2_PC_DIR}")

  find_package(PkgConfig)
  if(PkgConfig_FOUND)
    set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH ON)
    pkg_search_module(libgit2 REQUIRED IMPORTED_TARGET libgit2)
    set(LIBGIT_PATH ${LIBGIT2_PREFIX})

  endif()
endif()

if(NOT libgit2_FOUND AND NOT DEFINED libgit2_QUIET)
  message(FATAL_ERROR "Could NOT find libgit2")
endif()

option(ENABLE_NSW_DEBUG "Compile with all DEBUG messages" OFF)
if(ENABLE_NSW_DEBUG)
  remove_definitions(-DERS_NO_DEBUG)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

tdaq_generate_isinfo(nswmonis
    schema/NswMonitoring_is_info.xml
  CPP_OUTPUT is_cpp_srcs
  SIMPLE
  NAMED
  OUTPUT_DIRECTORY ${TDAQ_PACKAGE_NAME}Is
  NAMESPACE nsw::mon::is)

tdaq_add_is_schema(schema/NswMonitoring_is_info.xml)

tdaq_generate_dal(schema/NSWConfig.schema.xml
                  schema/NSWSCAService.schema.xml
                  schema/NSWConfigurationController.schema.xml
                  schema/NSWMonitoringController.schema.xml
                  schema/NSWSectorController.schema.xml
  NAMESPACE nsw::dal
  INCLUDE_DIRECTORIES DFConfiguration dal
  INCLUDE NSWConfigurationDal
  CPP_OUTPUT dal_cpp_srcs)

# Build a C++ library out of the generated files
tdaq_add_library(nswconfigdal DAL
  ${dal_cpp_srcs}
  LINK_LIBRARIES tdaq::config tdaq::daq-core-dal tdaq::daq-df-dal)

tdaq_add_library(NSWConfig src/NSWConfig.cpp
  INCLUDE_DIRECTORIES
                 $<$<VERSION_EQUAL:${TDAQ_VERSION},9.4.0>:$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extern/omni/include>>
  LINK_LIBRARIES nswconfig tdaq::daq-df-dal tdaq::rc_ItemCtrl
                 tdaq-common::ers Boost::program_options nswconfigdal nswhwinterface nswmonitoring
)

## Executables linked against Boost::boost
set(NSWCONFIG_EXE_WITH_BOOST generate_config_json)

foreach(appsrc IN LISTS NSWCONFIG_EXE_WITH_BOOST)
  tdaq_add_executable(${appsrc} app/${appsrc}.cpp
    LINK_LIBRARIES Boost::boost tdaq-common::ers
  )
endforeach()

## Executables linked against nswconfig, nswhwinterface, and Boost::program_options
## and ROOOOT
set(NSWCONFIG_EXE_WITH_BOOST_PO_AND_ROOT
  tp_mon_env
    )

## Executables linked against nswconfig, nswhwinterface, and Boost::program_options
set(NSWCONFIG_EXE_WITH_BOOST_PO
  configure_frontend
  configure_gbtx
  read_sca_info
  roc_register_readout
  vmm_capture_status
  read_tds_continous
  read_vmm_adc
  check_gpio
  roc_loop_over_phase
  roc_select_phase
  configure_addc
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

foreach(appsrc IN LISTS NSWCONFIG_EXE_WITH_BOOST_PO_AND_ROOT)
  tdaq_add_executable(${appsrc} app/${appsrc}.cpp
    LINK_LIBRARIES nswconfig
    LINK_LIBRARIES nswhwinterface
    LINK_LIBRARIES Boost::program_options
    ROOT::Core
    ROOT::Tree
    ROOT::RIO
  )
endforeach()

foreach(appsrc IN LISTS NSWCONFIG_EXE_WITH_BOOST_PO_RENAMED)
  tdaq_add_executable(nsw_${appsrc} app/${appsrc}.cpp
    LINK_LIBRARIES nswconfig
    LINK_LIBRARIES nswhwinterface
    LINK_LIBRARIES Boost::program_options
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
                 src/ADDCConfig.cpp
                 src/TPConfig.cpp src/TPCarrierConfig.cpp
                 src/RouterConfig.cpp
                 src/ConfigConverter.cpp
                 src/OKSDeviceHierarchy.cpp
                 src/SCAGeoIdentifier.cpp
                 src/RcUtility.cpp
                 src/IsUtility.cpp
  INCLUDE_DIRECTORIES
                $<$<VERSION_EQUAL:${TDAQ_VERSION},9.4.0>:$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extern/omni/include>>
  LINK_LIBRARIES Boost::boost
                 nswconfigdal
                 nswopcclient
                 tdaq-common::ers
                 tdaq::rc_ItemCtrl
                 tdaq::daq-df-dal
                 ichandler
                 $<BUILD_INTERFACE:fmt::fmt-header-only>
    PRIVATE
                 PkgConfig::libgit2
                 stdc++fs
)

tdaq_add_library(nswhwinterface
                 src/hw/DeviceManager.cpp # Placeholder for future
                 src/hw/OpcManager.cpp
                 src/hw/SCAInterface.cpp
                 src/hw/ScaAddressBase.cpp
                 src/hw/OpcConnectionBase.cpp
                 src/hw/FEB.cpp
                 src/hw/ROC.cpp
                 src/hw/VMM.cpp
                 src/hw/TDS.cpp
                 src/hw/ART.cpp
                 src/hw/PadTrigger.cpp
                 src/hw/Router.cpp
                 src/hw/SCAX.cpp
                 src/hw/STGCTP.cpp
                 src/hw/MMTP.cpp
                 src/hw/TPCarrier.cpp
                 src/hw/Helper.cpp
  INCLUDE_DIRECTORIES
                $<$<VERSION_EQUAL:${TDAQ_VERSION},9.4.0>:$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extern/omni/include>>
  LINK_LIBRARIES nswconfig
    PRIVATE
                 $<BUILD_INTERFACE:fmt::fmt-header-only>
)

tdaq_add_library(nswmonitoring
                 src/monitoring/RocStatusRegisters.cpp
                 src/monitoring/RocConfigurationRegisters.cpp
                 src/monitoring/MmtpInRunStatusRegisters.cpp
                 src/monitoring/MmtpOutRunStatusRegisters.cpp
                 src/monitoring/PadTriggerRegisters.cpp
                 src/monitoring/IsPublisher.cpp
  LINK_LIBRARIES nswconfig
                 nswmonis
                 nswhwinterface
    PRIVATE
                 $<BUILD_INTERFACE:fmt::fmt-header-only>
)

tdaq_add_executable(NSWConfigRc_main app/NSWConfigRc_main.cpp src/NSWConfigRc.cpp
  INCLUDE_DIRECTORIES
                 $<$<VERSION_EQUAL:${TDAQ_VERSION},9.4.0>:$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extern/omni/include>>
  LINK_LIBRARIES nswconfig tdaq::daq-df-dal tdaq::rc_ItemCtrl
                 tdaq-common::ers Boost::program_options nswconfigdal NSWConfig
)

tdaq_add_executable(NSWSCAServiceRc_main app/NSWSCAServiceRc_main.cpp src/NSWSCAServiceRc.cpp
  LINK_LIBRARIES nswconfig tdaq::daq-df-dal tdaq::rc_ItemCtrl
                 tdaq-common::ers Boost::program_options nswconfigdal NSWConfig nswconfig
)

tdaq_add_executable(NSWConfigurationControllerRc_main app/NSWConfigurationControllerRc_main.cpp src/NSWConfigurationControllerRc.cpp
  LINK_LIBRARIES nswconfig tdaq::daq-df-dal tdaq::rc_ItemCtrl
                 tdaq-common::ers Boost::program_options nswconfigdal NSWConfig
)

tdaq_add_executable(NSWMonitoringControllerRc_main app/NSWMonitoringControllerRc_main.cpp src/NSWMonitoringControllerRc.cpp
  LINK_LIBRARIES nswconfig tdaq::daq-df-dal tdaq::rc_ItemCtrl
                 tdaq-common::ers Boost::program_options nswconfigdal NSWConfig
)

tdaq_add_executable(NSWSectorControllerRc_main app/NSWSectorControllerRc_main.cpp src/NSWSectorControllerRc.cpp
  LINK_LIBRARIES nswconfig tdaq::daq-df-dal tdaq::rc_ItemCtrl
                 tdaq-common::ers Boost::program_options nswconfigdal NSWConfig nswconfig
)

tdaq_add_schema(schema/NSWConfig.schema.xml)
tdaq_add_schema(schema/NSWConfigurationController.schema.xml)
tdaq_add_schema(schema/NSWMonitoringController.schema.xml)
tdaq_add_schema(schema/NSWSectorController.schema.xml)
tdaq_add_schema(schema/NSWSCAService.schema.xml)

tdaq_add_is_schema(info/NswMonitoring.schema.xml)

# Testing

set(NSWCONFIG_TEST_DATA test_vmm.json test_padtrigger.json test_jsonapi.json TP_testRegisterConfig.json)
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

tdaq_add_executable(test_scageoidentifier test/test_scageoidentifier.cpp
  NOINSTALL
  LINK_LIBRARIES Boost::unit_test_framework tdaq-common::ers nswconfig
  PRIVATE $<BUILD_INTERFACE:fmt::fmt-header-only>)

tdaq_add_executable(test_constants test/test_constants.cpp
  NOINSTALL
  LINK_LIBRARIES Boost::unit_test_framework tdaq-common::ers nswconfig
  PRIVATE $<BUILD_INTERFACE:fmt::fmt-header-only>)

tdaq_add_executable(test_febhw test/test_febhw.cpp
  NOINSTALL
  LINK_LIBRARIES Boost::unit_test_framework tdaq-common::ers nswconfig nswhwinterface
  PRIVATE $<BUILD_INTERFACE:fmt::fmt-header-only>)

tdaq_add_executable(test_padtrigger test/test_padtrigger.cpp
  NOINSTALL
  LINK_LIBRARIES Boost::unit_test_framework tdaq-common::ers nswconfig nswhwinterface
  PRIVATE $<BUILD_INTERFACE:fmt::fmt-header-only>)

### Tests
set(NSWCONFIG_TESTS jsonapi configreader i2cmasterconfig utility vmmconfig configtranslation scageoidentifier constants febhw padtrigger)

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

option(SKIP_NSW_DOCS "Disable doxygen when dependent build" OFF)
if (NOT SKIP_NSW_DOCS)
if (BUILD_NSW_DOCS)
  message(STATUS "CMAKE_MODULE_PATH is ${CMAKE_MODULE_PATH}")
  include(NSWDoxygen)
  SetupNSWDoxygen()

  if (TARGET ${TDAQ_PACKAGE_NAME}.doxygen)
    add_dependencies(${TDAQ_PACKAGE_NAME}.doxygen nswconfigdal)
    add_dependencies(${TDAQ_PACKAGE_NAME}.doxygen nswmonis)
    SetupNSWDoxygenTargets(${TDAQ_PACKAGE_NAME}.doxygen)
  endif()
endif()
endif()
