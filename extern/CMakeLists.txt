cmake_minimum_required(VERSION 3.14.0)

project(NSWConfiguration_extern)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
option(NSWCONFIG_FIND_EXTERNAL CACHE OFF)

if(NSWCONFIG_FIND_EXTERNAL)
  find_package(Open62541Compat REQUIRED)
  find_package(UaoClient REQUIRED)
endif()

set(NSWCONFIG_FETCH_METHOD "github" CACHE STRING
  "Method to use to clone external dependencies, choose from: ssh, [krb], https, github")

message(VERBOSE "  NSWCONFIG_FETCH_METHOD ${NSWCONFIG_FETCH_METHOD}")

if(NSWCONFIG_FETCH_METHOD STREQUAL "ssh")
  message(VERBOSE "  Explicitly told to use SSH endpoint for FetchContent")
  set(GIT_URL_ENDPOINT "ssh://git@gitlab.cern.ch:7999" CACHE INTERNAL "")
elseif(NSWCONFIG_FETCH_METHOD STREQUAL "https")
  message(VERBOSE "  Explicitly told to use HTTPS endpoint for FetchContent")
  set(GIT_URL_ENDPOINT "https://gitlab.cern.ch" CACHE INTERNAL "")
elseif(NSWCONFIG_FETCH_METHOD STREQUAL "github")
  message(VERBOSE "  Using github endpoint for FetchContent")
  set(GIT_URL_ENDPOINT "https://github.com" CACHE INTERNAL "")
else()
  message(VERBOSE "  Using KRB endpoint for FetchContent")
  set(GIT_URL_ENDPOINT "https://:@gitlab.cern.ch:8443" CACHE INTERNAL "")
endif()

message(STATUS "  FetchContent GIT_URL_ENDPOINT ${GIT_URL_ENDPOINT}")

if(NOT Open62541Compat_FOUND)
  message(STATUS "Unable to find Open62541Compat, will build locally")
  include(FetchLogIt)
  if(BUILD_LOGIT)
    message(STATUS "  Building LOGIT_VERSION [${LOGIT_VERSION}]")
    build_LogIt()
  endif()

  include(FetchOpen62541Compat)
  if(BUILD_OPEN62541_COMPAT)
    message(STATUS "  Building OPEN62541_COMPAT_VERSION [${OPEN62541_COMPAT_VERSION}]")
    build_open62541_compat()
  endif()
endif()

if(NOT UaoClient_FOUND)
  message(STATUS "Unable to find or UaoClientForOpcUaSca, will build locally")
  if(GIT_URL_ENDPOINT STREQUAL "https://github.com")
    set(GIT_URL_ENDPOINT "https://:@gitlab.cern.ch:8443" CACHE INTERNAL "")
  endif()
  include(FetchUaoClientForOpcUaSca)

  if(BUILD_UAOCLIENTFOROPCUASCA)
    message(STATUS "  Building UAOCLIENTFOROPCUASCA_VERSION [${UAOCLIENTFOROPCUASCA_VERSION}]")
    build_UaoClientForOpcUaSca()
  endif()
endif()
