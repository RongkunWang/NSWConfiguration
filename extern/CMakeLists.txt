cmake_minimum_required(VERSION 3.14.0)

project(NSWConfiguration_extern)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
option(NSWCONFIG_FIND_EXTERNAL CACHE OFF)

if(NSWCONFIG_FIND_EXTERNAL)
  find_package(Open62541Compat REQUIRED)
  find_package(UaoClient REQUIRED)
endif()

if(NSW_STANDALONE_CI_BUILD)
  message(STATUS " Executing a standalone CI build, fetching NSW dependencies")
  option(NSW_CI_BUILD_USE_EXTERNAL_ICHANDLER CACHE ON)
  if(NOT NSW_DEP_FETCH_METHOD)
    set(NSW_DEP_FETCH_METHOD "ci" CACHE INTERNAL "")
  else()
    set(NSW_DEP_FETCH_METHOD "${NSW_DEP_FETCH_METHOD}" CACHE INTERNAL "")
  endif()
  set(NSWCONFIG_FETCH_METHOD "${NSW_DEP_FETCH_METHOD}" CACHE INTERNAL "")
endif()

if(NOT GIT_URL_ENDPOINT)
  set(NSWCONFIG_FETCH_METHOD "ssh" CACHE STRING
    "Method to use to clone external dependencies, choose from: [ssh], krb, https, github")

  message(STATUS "  NSWCONFIG_FETCH_METHOD ${NSWCONFIG_FETCH_METHOD}")

  if(NSWCONFIG_FETCH_METHOD STREQUAL "ssh")
    message(STATUS "  Explicitly told to use SSH endpoint for FetchContent")
    set(GIT_URL_ENDPOINT "ssh://git@gitlab.cern.ch:7999" CACHE INTERNAL "")
  elseif(NSWCONFIG_FETCH_METHOD STREQUAL "https")
    message(STATUS "  Explicitly told to use HTTPS endpoint for FetchContent")
    set(GIT_URL_ENDPOINT "https://gitlab.cern.ch" CACHE INTERNAL "")
  elseif(NSWCONFIG_FETCH_METHOD STREQUAL "ci")
    message(STATUS "  Explicitly told to use HTTPS endpoint with token for FetchContent")
    set(GIT_URL_ENDPOINT "https://gitlab-ci-token:$ENV{CI_JOB_TOKEN}@gitlab.cern.ch" CACHE INTERNAL "")
  elseif(NSWCONFIG_FETCH_METHOD STREQUAL "github")
    message(STATUS "  Using github endpoint for FetchContent")
    set(GIT_URL_ENDPOINT "https://github.com" CACHE INTERNAL "")
  else()
    message(STATUS "  Using KRB endpoint for FetchContent (requested ${NSWCONFIG_FETCH_METHOD})")
    set(GIT_URL_ENDPOINT "https://:@gitlab.cern.ch:8443" CACHE INTERNAL "")
  endif()
endif()

message(STATUS "  NSWConfiguration FetchContent GIT_URL_ENDPOINT ${GIT_URL_ENDPOINT}")

option(NSW_EXTERN_WARN "Enable compiler warnings for extern" OFF)
if(NOT NSW_EXTERN_WARN)
  ## Disable compiler warnings in all external components
  add_compile_options(-w)
endif()

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
  message(STATUS "Unable to find UaoClientForOpcUaSca, will build locally")
  if(GIT_URL_ENDPOINT STREQUAL "https://github.com")
    set(GIT_URL_ENDPOINT "https://:@gitlab.cern.ch:8443" CACHE INTERNAL "")
  endif()
  include(FetchUaoClientForOpcUaSca)

  if(BUILD_UAOCLIENTFOROPCUASCA)
    message(STATUS "  Building UAOCLIENTFOROPCUASCA_VERSION [${UAOCLIENTFOROPCUASCA_VERSION}]")
    build_UaoClientForOpcUaSca()
  endif()
endif()

set(NSWCONFIG_FMT_VERSION 10.1.1 CACHE INTERNAL "Select the fmt version")
option(NSWCONFIG_USE_EXTERNAL_FMT "Use an external fmt library" OFF)
if(NOT TARGET fmt::fmt-header-only)
    if(NSWCONFIG_USE_EXTERNAL_FMT)
        message(STATUS "Fetching fmt from github, using version ${NSWCONFIG_FMT_VERSION}")

        ## Fetch fmt from github
        include(FetchContent)

        FetchContent_Declare(fmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG ${NSWCONFIG_FMT_VERSION}
        )
    endif()

    option(FMT_INSTALL "Enable install for fmt project" ON)
    option(FMT_DOC "" OFF)
    option(FMT_TEST "" OFF)
    option(FMT_OS "" OFF)
    option(FMT_OS "" ON)
    option(FMT_SYSTEM_HEADERS "" ON)

    if(NSWCONFIG_USE_EXTERNAL_FMT)
        FetchContent_GetProperties(fmt)
        if(NOT fmt_POPULATED)
            FetchContent_Populate(fmt)
            add_subdirectory(${fmt_SOURCE_DIR} ${fmt_BINARY_DIR} EXCLUDE_FROM_ALL)
        endif()
    else()
        ## Use bundled version
        message(STATUS "Using extern/fmt bundled with NSWConfiguration")
        add_subdirectory(fmt EXCLUDE_FROM_ALL)
    endif()
else()
    message(STATUS "Target fmt is already present, reusing")
endif()
