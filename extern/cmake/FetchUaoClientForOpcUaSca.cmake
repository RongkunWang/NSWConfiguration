option(BUILD_UAOCLIENTFOROPCUASCA "Build UaoClientForOpcUaSca from source" ON)
set(UAOCLIENTFOROPCUASCA_VERSION 1.5.2 CACHE STRING "Select version of UaoClientForOpcUaSca to build")

set(UAOCLIENTFOROPCUASCA_DIR ${CMAKE_CURRENT_BINARY_DIR}/UaoClientForOpcUaSca)
include(FetchContent)

function(fetch_UaoClientForOpcUaSca)
  message(STATUS "  Fetching UaoClientForOpcUaSca from CERN GitLab.")
  message(STATUS "  *NOTE* fetching version [${UAOCLIENTFOROPCUASCA_VERSION}]")
  message(STATUS "  *Patching* UaoClientForOpcUaSca")
  set(patchCommand PATCH_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/patch-UaoClientForOpcUaSca.sh")

  FetchContent_Declare(
    UaoClientForOpcUaSca
    GIT_REPOSITORY ${GIT_URL_ENDPOINT}/atlas-dcs-opcua-servers/UaoClientForOpcUaSca.git
    GIT_TAG        ${UAOCLIENTFOROPCUASCA_VERSION}
    GIT_SHALLOW    "1"
    SOURCE_DIR	   ${UAOCLIENTFOROPCUASCA_DIR}
    BINARY_DIR     ${UAOCLIENTFOROPCUASCA_DIR}
    UPDATE_DISCONNECTED ON
    ${patchCommand}
  )
endfunction()

macro(build_UaoClientForOpcUaSca)
  set(BUILD_CONFIG open62541_config.cmake CACHE STRING "")
  if(Open62541Compat_INCLUDE_DIR)
    get_filename_component(Open62541Compat_PREFIX_DIR "${Open62541Compat_INCLUDE_DIR}" PATH)
    set(OPEN62541_COMPAT_DIR  ${Open62541Compat_PREFIX_DIR} CACHE STRING "")
  else()
    set(OPEN62541_COMPAT_DIR  ${open62541-compat_BINARY_DIR} CACHE STRING "")
  endif()

  if(LogIt_INCLUDE_DIR)
    set(LOGIT_INCLUDE_DIR     ${LogIt_INCLUDE_DIR} CACHE STRING "")
  elseif(logit_BINARY_DIR)
    set(LOGIT_INCLUDE_DIR     ${logit_BINARY_DIR}/include CACHE STRING "")
  else()
    set(LOGIT_INCLUDE_DIR     ${open62541-compat_BINARY_DIR}/LogIt/include CACHE STRING "")
  endif()
  set(CMAKE_INSTALL_PREFIX  ${CMAKE_PROJECT_BINARY_DIR}/install/UaoClientForOpcUaSca CACHE STRING "")
  option(BUILD_STANDALONE CACHE ON)

  ### compile_commands.json DB
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

  fetch_UaoClientForOpcUaSca()

  add_compile_options(-Wno-error -Wno-pedantic -Wno-cast-qual)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-discarded-qualifiers>)

  if(NOT NSW_EXTERN_WARN)
    add_compile_options(-w)
  endif()

  message(CHECK_START "  Looking for UaoClientForOpcUaSca protobuf dependencies")
  if(EXISTS "/sw/atlas/sw/lcg")
    set(UAO_PROTOBUF_LCG_DIR "/sw/atlas/sw/lcg" CACHE INTERNAL "")
  elseif(EXISTS "/cvmfs/sft.cern.ch/lcg")
    set(UAO_PROTOBUF_LCG_DIR "/cvmfs/sft.cern.ch/lcg" CACHE INTERNAL "")
  endif()
  set(UAO_PROTOBUF_VERSION "2.5.0-aa8bd" CACHE INTERNAL "")
  set(UAO_PROTOBUF_RELEASE $ENV{CMTCONFIG} CACHE INTERNAL "")
  set(PROTOBUF_ROOT "${UAO_PROTOBUF_LCG_DIR}/releases/protobuf/${UAO_PROTOBUF_VERSION}/${UAO_PROTOBUF_RELEASE}" CACHE INTERNAL "")
  if(EXISTS ${PROTOBUF_ROOT})
    message(CHECK_PASS "    using PROTOBUF_ROOT ${PROTOBUF_ROOT}")
  else()
    message(CHECK_FAIL "    Unable to find a compatible protobuf release")
  endif()
  find_package(Protobuf 2.5.0 EXACT REQUIRED COMPONENTS libprotobuf)

  # FetchContent_MakeAvailable(UaoClientForOpcUaSca)
  ## Done to disable the default header installation location
  ## otherwise, use the above FetchContent_MakeAvailable
  FetchContent_GetProperties(UaoClientForOpcUaSca)
  if(NOT uaoclientforopcuasca_POPULATED)
    FetchContent_Populate(UaoClientForOpcUaSca)
    add_subdirectory(${uaoclientforopcuasca_SOURCE_DIR} ${uaoclientforopcuasca_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()

  add_library(UaoClient INTERFACE)
  add_library(UaoClient::UaoClientForOpcUaSca ALIAS UaoClientForOpcUaSca)

  ## Add -flto, if supported
  if(IPO_SUPPORTED)
    message(STATUS "  Enabling IPO for UaoClientForOpcUaSca")
    set_target_properties(UaoClientForOpcUaSca PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
  endif()

  ## Add -fPIC for inclusion in shared libs
  set_target_properties(UaoClientForOpcUaSca
    PROPERTIES
      EXCLUDE_FROM_ALL FALSE
      POSITION_INDEPENDENT_CODE ON
      C_CLANG_TIDY ""
      CXX_CLANG_TIDY ""
      C_CPPCHECK ""
      CXX_CPPCHECK ""
      C_INCLUDE_WHAT_YOU_USE ""
      CXX_INCLUDE_WHAT_YOU_USE ""
    )

  target_sources(UaoClientForOpcUaSca PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/UaoClientForOpcUaSca/src/BitBangProtocol.pb.cc
  )

  target_include_directories(UaoClientForOpcUaSca SYSTEM BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/UaoClientForOpcUaSca/include>
    $<INSTALL_INTERFACE:extern/include>
  )

  target_link_directories(UaoClientForOpcUaSca BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/UaoClientForOpcUaSca>
    $<INSTALL_INTERFACE:extern/lib>
  )

 target_link_libraries(UaoClientForOpcUaSca PRIVATE
    Open62541Compat::open62541-compat
  )

  install(TARGETS UaoClientForOpcUaSca
    DESTINATION extern/lib
  )

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/UaoClientForOpcUaSca/include/
    DESTINATION extern/include/UaoClient
    FILES_MATCHING PATTERN "*.h*"
  )

  install(TARGETS UaoClientForOpcUaSca
    EXPORT UaoClient
    ARCHIVE DESTINATION extern/lib
    LIBRARY DESTINATION extern/lib
    RUNTIME DESTINATION extern/bin
    PUBLIC_HEADER DESTINATION extern/include/UaoClient
    PRIVATE_HEADER DESTINATION extern/include/UaoClient
    INCLUDES DESTINATION extern/include/UaoClient
  )

  install(EXPORT UaoClient
    FILE UaoClientTargets.cmake
    NAMESPACE UaoClient::
    DESTINATION extern/lib/cmake/UaoClientForOpcUaSca
  )

  set(uaoclientforopcuasca_SOURCE_DIR  ${uaoclientforopcuasca_SOURCE_DIR}  PARENT_SCOPE)
  set(uaoclientforopcuasca_BINARY_DIR  ${uaoclientforopcuasca_BINARY_DIR}  PARENT_SCOPE)
  set(uaoclientforopcuasca_INSTALL_DIR ${uaoclientforopcuasca_INSTALL_DIR} PARENT_SCOPE)
endmacro()
