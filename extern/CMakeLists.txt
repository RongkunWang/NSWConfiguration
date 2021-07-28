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

set(NSWCONFIG_FMT_VERSION 8.0.1 CACHE INTERNAL "Select the fmt version")
option(NSWCONFIG_USE_EXTERNAL_FMT "Use an external fmt library" OFF)
if(NSWCONFIG_USE_EXTERNAL_FMT)
    message(STATUS "Fetching fmt from github, using version ${NSWCONFIG_FMT_VERSION}")

    ## Fetch fmt from github
    include(FetchContent)

    FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG ${NSWCONFIG_FMT_VERSION}
    )
    option(FMT_INSTALL "Enable install for fmt project" ON)
    option(FMT_DOC "" OFF)
    option(FMT_TEST "" OFF)
    option(FMT_OS "" OFF)

    FetchContent_GetProperties(fmt)
    if(NOT fmt_POPULATED)
        FetchContent_Populate(fmt)
        add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR} EXCLUDE_FROM_ALL)
    endif()
else()
    ## Use bundled version
    message(STATUS "Using extern/fmt bundled with NSWConfiguration")
    option(FMT_INSTALL "Enable install for fmt project" ON)
    option(FMT_DOC "" OFF)
    option(FMT_TEST "" OFF)
    option(FMT_OS "" OFF)
    add_subdirectory(fmt EXCLUDE_FROM_ALL)
endif()
