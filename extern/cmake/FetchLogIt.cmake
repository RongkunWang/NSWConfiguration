option(BUILD_LOGIT "Build LogIt from source" OFF)
set(LOGIT_VERSION v0.1.3 CACHE STRING "Select version of LogIt to build")

set(LOGIT_DIR ${CMAKE_CURRENT_BINARY_DIR}/LogIt)
include(FetchContent)

function(fetch_LogIt)
  if(NOT NSWCONFIG_FETCH_METHOD STREQUAL "github")
    message(STATUS "  Fetching LogIt from CERN GitLab.")
  else()
    message(STATUS "  Fetching LogIt from github.")
  endif()

  message(STATUS "  *NOTE* fetching version [${LOGIT_VERSION}]")

  FetchContent_Declare(
    LogIt
    GIT_REPOSITORY ${GIT_URL_ENDPOINT}/quasar-team/LogIt.git
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

  add_compile_options(-Wno-error -Wno-pedantic -Wno-cast-qual -w)
  add_compile_options($<$<COMPILE_LANGUAGE:C>:-Wno-discarded-qualifiers>)

  if(NOT NSW_EXTERN_WARN)
    add_compile_options(-w)
  endif()

  # FetchContent_MakeAvailable(LogIt)
  ## Done to disable the default header installation location
  ## otherwise, use the above FetchContent_MakeAvailable
  FetchContent_GetProperties(LogIt)
  if(NOT logit_POPULATED)
    FetchContent_Populate(LogIt)
    add_subdirectory(${logit_SOURCE_DIR} ${logit_BINARY_DIR} EXCLUDE_FROM_ALL)
  endif()

  # add_library(LogIt INTERFACE)
  # add_library(LogIt::LogIt ALIAS LogIt)

  ## Add -flto, if supported
  if(IPO_SUPPORTED)
    # message(STATUS "  Enabling IPO for LogIt")
    # set_target_properties(LogIt PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON)
  endif()

  ## Add -fPIC for inclusion in shared libs
  set_target_properties(LogIt
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

  target_include_directories(LogIt SYSTEM BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/LogIt/include>
    $<INSTALL_INTERFACE:extern/include>
  )

  target_link_directories(LogIt BEFORE INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/LogIt>
    $<INSTALL_INTERFACE:extern/lib>
  )

  install(TARGETS LogIt
    DESTINATION extern/lib
    )

  install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/LogIt/include/
    DESTINATION extern/include/LogIt
    FILES_MATCHING PATTERN "*.h*"
    )

  install(TARGETS LogIt
    EXPORT LogIt
    ARCHIVE DESTINATION extern/lib
    LIBRARY DESTINATION extern/lib
    RUNTIME DESTINATION extern/bin
    INCLUDES DESTINATION extern/include/LogIt
    PUBLIC_HEADER DESTINATION extern/include/LogIt
    PRIVATE_HEADER DESTINATION extern/include/LogIt
  )

  install(EXPORT LogIt
    FILE LogIt.cmake
    NAMESPACE LogIt::
    DESTINATION extern/lib/cmake/LogIt
  )

  set(logit_SOURCE_DIR  ${logit_SOURCE_DIR}  PARENT_SCOPE)
  set(logit_BINARY_DIR  ${logit_BINARY_DIR}  PARENT_SCOPE)
  set(logit_INSTALL_DIR ${logit_INSTALL_DIR} PARENT_SCOPE)
endmacro()
