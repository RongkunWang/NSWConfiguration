option(BUILD_OPEN62541_COMPAT "Build open62541-compat from source" ON)
set(OPEN62541_COMPAT_VERSION v1.1.3-rc0 CACHE STRING "Select version of open62541-compat to build")

set(OPEN62541_COMPAT_DIR ${CMAKE_CURRENT_BINARY_DIR}/open62541-compat)
include(FetchContent)

function(fetch_open62541_compat)
  message(STATUS "fetching open62541-compat from github. *NOTE* fetching version [${OPEN62541_COMPAT_VERSION}]")
  FetchContent_Declare(
    open62541-compat
    GIT_REPOSITORY https://github.com/quasar-team/open62541-compat.git
    GIT_TAG        ${OPEN62541_COMPAT_VERSION}
    GIT_SHALLOW    "1"
    SOURCE_DIR     ${OPEN62541_COMPAT_DIR}
    BINARY_DIR     ${OPEN62541_COMPAT_DIR}
    )

endfunction()

macro(build_open62541_compat)
  set(OPEN62541-COMPAT_BUILD_CONFIG_FILE boost_lcg.cmake CACHE STRING "")
  set(CMAKE_INSTALL_PREFIX  ${CMAKE_PROJECT_BINARY_DIR}/install/open62541-compat CACHE STRING "")
  option(STANDALONE_BUILD CACHE ON)
  option(STANDALONE_BUILD_SHARED CACHE OFF)
  option(SKIP_TESTS CACHE ON)

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

  FetchContent_MakeAvailable(open62541-compat)

  add_library(Open62541Compat INTERFACE)
  add_library(Open62541Compat::open62541-compat ALIAS open62541-compat)
  add_library(Open62541Compat::open62541 ALIAS open62541)
  add_library(Open62541Compat::LogIt ALIAS LogIt)

  ## Add -flto, if supported
  if(IPO_SUPPORTED)
    message(STATUS "Enabling IPO for open62541-compat")
    # set_target_properties(open62541 LogIt open62541-compat PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
  endif()

  ## Add -fPIC for shared lib inclusion
  set_target_properties(open62541 LogIt open62541-compat PROPERTIES POSITION_INDEPENDENT_CODE ON)

  target_include_directories(open62541-compat BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/open62541-compat/include>
    )
  target_include_directories(open62541  BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/open62541-compat/open62541/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/open62541-compat/open62541>
    )
  target_include_directories(LogIt BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/open62541-compat/LogIt/include>
    )

  target_link_directories(open62541-compat BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/open62541-compat>
    )
  ## Should only be needed for a shared object, but CMake is putting -lopen62541 even when the library is static...
  target_link_directories(open62541 BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/open62541-compat/open62541>
    )

  ## This *should* be done by MakeAvailable... no?
  install(TARGETS open62541-compat
    EXPORT Open62541Compat
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    )

  install(TARGETS LogIt
    EXPORT Open62541Compat
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
    )

  install(EXPORT Open62541Compat
    FILE Open62541Compat.cmake
    NAMESPACE Open62541Compat::
    DESTINATION lib/cmake/Open62541Compat)

  set(open62541-compat_SOURCE_DIR  ${open62541-compat_SOURCE_DIR}  PARENT_SCOPE)
  set(open62541-compat_BINARY_DIR  ${open62541-compat_BINARY_DIR}  PARENT_SCOPE)
  set(open62541-compat_INSTALL_DIR ${open62541-compat_INSTALL_DIR} PARENT_SCOPE)
endmacro()
