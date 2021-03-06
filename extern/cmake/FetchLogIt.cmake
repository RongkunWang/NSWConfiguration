option(BUILD_LOGIT "Build LogIt from source" OFF)
set(LOGIT_VERSION OPCUA-1710-install-target-for-LogIt CACHE STRING "Select version of LogIt to build")

set(LOGIT_DIR ${CMAKE_CURRENT_BINARY_DIR}/LogIt)
include(FetchContent)

function(fetch_LogIt)
  message(STATUS "Fetching LogIt from github. *NOTE* fetching version [${LOGIT_VERSION}]")
  FetchContent_Declare(
    LogIt
    GIT_REPOSITORY https://github.com/quasar-team/LogIt.git
    GIT_TAG        ${LOGIT_VERSION}
    GIT_SHALLOW    "1"
    SOURCE_DIR     ${LOGIT_DIR}
    BINARY_DIR     ${LOGIT_DIR}
    )
endfunction()

macro(build_LogIt)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_PROJECT_BINARY_DIR}/install/LogIt CACHE STRING "")
  option(LOGIT_BUILD_STATIC_LIB CACHE ON)
  option(LOGIT_BUILD_STAND_ALONE CACHE ON)

  ### compile_commands.json DB
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE INTERNAL "")

  fetch_LogIt()

  FetchContent_MakeAvailable(LogIt)

  add_library(LogIt INTERFACE)
  add_library(LogIt::LogIt ALIAS LogIt)

  ## Add -flto, if supported
  if(IPO_SUPPORTED)
    message(STATUS "Enabling IPO for LogIt")
    # set_target_properties(LogIt PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
  endif()

  ## Add -fPIC for inclusion in shared libs
  set_target_properties(LogIt
    PROPERTIES
      POSITION_INDEPENDENT_CODE ON
      C_CLANG_TIDY ""
      CXX_CLANG_TIDY ""
      C_CPPCHECK ""
      CXX_CPPCHECK ""
      C_INCLUDE_WHAT_YOU_USE ""
      CXX_INCLUDE_WHAT_YOU_USE ""
    )

  target_include_directories(LogIt SYSTEM BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/LogIt/include>
    )
  target_link_directories(LogIt BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/LogIt>
    )

  ## this *should* be done by MakeAvailable...
  install(TARGETS LogIt
    EXPORT LogIt
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include)

  install(EXPORT LogIt
    FILE LogIt.cmake
    NAMESPACE LogIt::
    DESTINATION lib/cmake/LogIt)

  ## These *should* be visible outside but aren't set...
  set(logit_SOURCE_DIR  ${logit_SOURCE_DIR}  PARENT_SCOPE)
  set(logit_BINARY_DIR  ${logit_BINARY_DIR}  PARENT_SCOPE)
  set(logit_INSTALL_DIR ${logit_INSTALL_DIR} PARENT_SCOPE)
endmacro()
