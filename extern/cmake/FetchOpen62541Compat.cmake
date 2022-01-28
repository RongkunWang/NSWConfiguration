option(BUILD_OPEN62541_COMPAT "Build open62541-compat from source" ON)
set(OPEN62541_COMPAT_VERSION v1.4.1 CACHE STRING "Select version of open62541-compat to build")

set(OPEN62541_COMPAT_DIR ${CMAKE_CURRENT_BINARY_DIR}/open62541-compat)
include(FetchContent)

function(fetch_open62541_compat)
  if(NOT NSWCONFIG_FETCH_METHOD STREQUAL "github")
    message(STATUS "  Fetching open62541-compat from CERN GitLab.")
    set(updateCommand UPDATE_COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/patch-open62541-compat.sh")
  else()
    message(STATUS "  Fetching open62541-compat from github.")
    unset(updateCommand)
  endif()

  message(STATUS "  *NOTE* fetching version [${OPEN62541_COMPAT_VERSION}]")

  FetchContent_Declare(
    open62541-compat
    GIT_REPOSITORY ${GIT_URL_ENDPOINT}/quasar-team/open62541-compat.git
    GIT_TAG        ${OPEN62541_COMPAT_VERSION}
    GIT_SHALLOW    "1"
    SOURCE_DIR     ${OPEN62541_COMPAT_DIR}
    BINARY_DIR     ${OPEN62541_COMPAT_DIR}
    ${updateCommand}
  )
endfunction()

macro(build_open62541_compat)
  set(OPEN62541-COMPAT_BUILD_CONFIG_FILE boost_lcg.cmake CACHE STRING "")
  set(CMAKE_INSTALL_PREFIX  ${CMAKE_PROJECT_BINARY_DIR}/install/open62541-compat CACHE STRING "")
  option(STANDALONE_BUILD CACHE ON)
  option(STANDALONE_BUILD_SHARED CACHE OFF)
  option(SKIP_TESTS CACHE ON)
  option(PULL_OPEN62541 CACHE OFF)

  if(LOGIT_BUILD_OPTION STREQUAL "LOGIT_AS_EXT_STATIC")
    if(logit_BINARY_DIR)
      set(LOGIT_INCLUDE_DIR ${logit_BINARY_DIR}/include CACHE INTERNAL "")
      set(LOGIT_EXT_LIB_DIR ${logit_BINARY_DIR}/lib CACHE INTERNAL "")
    endif()
  endif()

  ## Because tdaq_cmake turns this on globally, resulting in the above cache variable
  ## being ignored after a reconfiguration
  if(NOT STANDALONE_BUILD_SHARED)
    set(BUILD_SHARED_LIBS OFF)
  endif()

  ### compile_commands.json DB
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

  fetch_open62541_compat()

  add_compile_options(-Wno-error -Wno-pedantic -Wno-cast-qual)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-discarded-qualifiers>)

  # FetchContent_MakeAvailable(open62541-compat)
  ## Done to disable the default header installation location
  ## otherwise, use the above FetchContent_MakeAvailable
  FetchContent_GetProperties(open62541-compat)
  if(NOT uaoclientforopcuasca_POPULATED)
    FetchContent_Populate(open62541-compat)
    add_subdirectory(${open62541-compat_SOURCE_DIR} ${open62541-compat_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()

  add_library(Open62541Compat INTERFACE)
  add_library(Open62541Compat::open62541-compat ALIAS open62541-compat)

  if(NOT LOGIT_BUILD_OPTION STREQUAL "LOGIT_AS_EXT_STATIC")
    add_library(Open62541Compat::LogIt ALIAS LogIt)
  endif()

  ## Add -flto, if supported
  if(IPO_SUPPORTED)
    # message(STATUS "  Enabling IPO for open62541-compat")
    # set_target_properties(LogIt open62541-compat PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
  endif()

  ## Add -fPIC for shared lib inclusion
  set_target_properties(LogIt open62541-compat
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

  set_target_properties(open62541-compat PROPERTIES
    PUBLIC_HEADER "${OPEN62541_COMPAT_DIR}/extern/open62541/include/open62541.h")

  target_include_directories(open62541-compat SYSTEM BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/open62541-compat/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/open62541-compat/extern/open62541/include>
    $<INSTALL_INTERFACE:extern/include>
  )

  target_link_directories(open62541-compat BEFORE
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/open62541-compat>
        $<INSTALL_INTERFACE:extern/lib>
  )

  install(TARGETS open62541-compat
    DESTINATION extern/lib
  )

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/open62541-compat/include/
    DESTINATION extern/include/Open62541Compat
    FILES_MATCHING PATTERN "*.h*"
  )

  install(TARGETS open62541-compat
    EXPORT Open62541Compat
    ARCHIVE DESTINATION extern/lib
    LIBRARY DESTINATION extern/lib
    RUNTIME DESTINATION extern/bin
    INCLUDES DESTINATION extern/include/Open62541Compat
    PUBLIC_HEADER DESTINATION extern/include/Open62541Compat
    PRIVATE_HEADER DESTINATION extern/include/Open62541Compat
  )

  if(NOT LOGIT_BUILD_OPTION STREQUAL "LOGIT_AS_EXT_STATIC")
    target_include_directories(LogIt SYSTEM BEFORE INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/open62541-compat/LogIt/include>
      $<INSTALL_INTERFACE:extern/include>
    )

    target_link_directories(LogIt BEFORE INTERFACE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/open62541-compat/LogIt>
      $<INSTALL_INTERFACE:extern/lib>
    )

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/open62541-compat/LogIt/include/
      DESTINATION extern/include/LogIt
      FILES_MATCHING PATTERN "*.h*"
    )

    install(TARGETS LogIt
      EXPORT Open62541Compat
      ARCHIVE DESTINATION extern/lib
      LIBRARY DESTINATION extern/lib
      RUNTIME DESTINATION extern/bin
      INCLUDES DESTINATION extern/include/LogIt
      PUBLIC_HEADER DESTINATION extern/include/LogIt
      PRIVATE_HEADER DESTINATION extern/include/LogIt
    )
  endif()

  install(EXPORT Open62541Compat
    FILE Open62541Compat-extern.cmake
    NAMESPACE Open62541Compat::
    DESTINATION extern/lib/cmake/Open62541Compat
  )

  set(open62541-compat_SOURCE_DIR  ${open62541-compat_SOURCE_DIR}  PARENT_SCOPE)
  set(open62541-compat_BINARY_DIR  ${open62541-compat_BINARY_DIR}  PARENT_SCOPE)
  set(open62541-compat_INSTALL_DIR ${open62541-compat_INSTALL_DIR} PARENT_SCOPE)
endmacro()
