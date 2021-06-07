cmake_minimum_required(VERSION 3.14.0)

project(NSWConfiguration_extern)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

## Want to only do this if requested?
## Otherwise, dependencies are found after the first run
find_package(Open62541Compat)
find_package(UaoClient)

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
  include(FetchUaoClientForOpcUaSca)

  if(BUILD_UAOCLIENTFOROPCUASCA)
    message(STATUS "  Building UAOCLIENTFOROPCUASCA_VERSION [${UAOCLIENTFOROPCUASCA_VERSION}]")
    build_UaoClientForOpcUaSca()
  endif()
endif()
