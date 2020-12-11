option(BUILD_UAOCLIENTFOROPCUASCA "Build UaoClientForOpcUaSca from source" ON)
set(UAOCLIENTFOROPCUASCA_VERSION OPCUA-1714_deployable_as_INSTALL_target CACHE STRING "Select version of UaoClientForOpcUaSca to build")

set(UAOCLIENTFOROPCUASCA_DIR ${CMAKE_CURRENT_BINARY_DIR}/UaoClientForOpcUaSca)
include(FetchContent)

function(fetch_UaoClientForOpcUaSca)
  message(STATUS "Fetching UaoClientForOpcUaSca from github. *NOTE* fetching version [${UAOCLIENTFOROPCUASCA_VERSION}]")
  FetchContent_Declare(
    UaoClientForOpcUaSca
    GIT_REPOSITORY https://:@gitlab.cern.ch:8443/atlas-dcs-opcua-servers/UaoClientForOpcUaSca.git
    GIT_TAG        ${UAOCLIENTFOROPCUASCA_VERSION}
    GIT_SHALLOW    "1"
    SOURCE_DIR	   ${UAOCLIENTFOROPCUASCA_DIR}
    BINARY_DIR     ${UAOCLIENTFOROPCUASCA_DIR}
    )
endfunction()

macro(build_UaoClientForOpcUaSca)
  set(BUILD_CONFIG open62541_config.cmake CACHE STRING "")
  set(OPEN62541_COMPAT_DIR  ${open62541-compat_BINARY_DIR} CACHE STRING "")
  set(LOGIT_INCLUDE_DIR     ${open62541-compat_BINARY_DIR}/LogIt/include CACHE STRING "")
  set(CMAKE_INSTALL_PREFIX  ${CMAKE_PROJECT_BINARY_DIR}/install/UaoClientForOpcUaSca CACHE STRING "")
  option(BUILD_STANDALONE CACHE ON)

  ### compile_commands.json DB
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

  fetch_UaoClientForOpcUaSca()

  add_compile_options(-Wno-error -Wno-pedantic -Wno-cast-qual)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-discarded-qualifiers>)

  FetchContent_MakeAvailable(UaoClientForOpcUaSca)

  add_library(UaoClient INTERFACE)
  add_library(UaoClient::UaoClientForOpcUaSca ALIAS UaoClientForOpcUaSca)

  ## Add -flto, if supported
  if(IPO_SUPPORTED)
    message(STATUS "Enabling IPO")
    # set_target_properties(UaoClientForOpcUaSca PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
  endif()

  find_package(Protobuf REQUIRED COMPONENTS libprotobuf)
  add_library(protobuf::libprotobuf UNKNOWN IMPORTED)

  ## Add -fPIC for inclusion in shared libs
  set_target_properties(UaoClientForOpcUaSca PROPERTIES POSITION_INDEPENDENT_CODE ON)

  target_sources(UaoClientForOpcUaSca PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/UaoClientForOpcUaSca/src/BitBangProtocol.pb.cc
  )
  target_include_directories(UaoClientForOpcUaSca BEFORE PUBLIC
    ${Protobuf_INCLUDE_DIRS}
  )
  target_include_directories(UaoClientForOpcUaSca BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/UaoClientForOpcUaSca/include>
  )
  target_link_directories(UaoClientForOpcUaSca BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/UaoClientForOpcUaSca>
  )

  target_link_libraries(UaoClientForOpcUaSca
    PUBLIC
      Open62541Compat::open62541-compat
      Open62541Compat::open62541
    PRIVATE
      ${Protobuf_LIBRARIES}
      Open62541Compat::LogIt
  )

  ## This *should* be done by MakeAvailable... no?
  install(TARGETS UaoClientForOpcUaSca
    EXPORT UaoClient
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

  install(EXPORT UaoClient
    FILE UaoClient.cmake
    NAMESPACE UaoClient::
    DESTINATION lib/cmake/UaoClientForOpcUaSca)

  set(uaoclientforopcuasca_SOURCE_DIR  ${uaoclientforopcuasca_SOURCE_DIR}  PARENT_SCOPE)
  set(uaoclientforopcuasca_BINARY_DIR  ${uaoclientforopcuasca_BINARY_DIR}  PARENT_SCOPE)
  set(uaoclientforopcuasca_INSTALL_DIR ${uaoclientforopcuasca_INSTALL_DIR} PARENT_SCOPE)
endmacro()
