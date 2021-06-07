# - Try to find UaoClient libraries
#
# Once done this will define
#  UaoClient_FOUND - UaoClientForOpcUaSca software stack available
#  UaoClient_INCLUDE_DIR - the UaoClientForOpcUaSca include directory
#  UaoClient_LIB_DIR - the UaoClientForOpcUaSca lib directory
#  UaoClientForOpcUaSca_LIBRARY - the UaoClientForOpcUaSca library
#
#  Additionally, the following targets will be available
#  UaoClient::UaoClientForOpcUaSca
#  UaoClientForOpcUaSca::UaoClientForOpcUaSca
#  UaoClientForOpcUaSca
#
# To override the default search paths, point UaoClient_DIR to the location of your
# UaoClient installation
#
#  -DUaoClient_DIR=/path/to/UaoClient/installed
#


set(UaoClient_SEARCH_PATHS "")

message(VERBOSE "Looking for UaoClientForOpcUaSca dependencies in " ${UaoClient_DIR} ${UaoClient_SEARCH_PATHS})

find_path(UaoClient_LIB_DIR
  NAMES
    UaoClientForOpcUaSca
    libUaoClientForOpcUaSca.so
    libUaoClientForOpcUaSca.a
  HINTS ${UaoClient_DIR}
  PATHS ${UaoClient_SEARCH_PATHS}
  PATH_SUFFIXES lib
 )

find_path(UaoClient_INCLUDE_DIR
  NAMES ClientSessionFactory.h
  HINTS ${UaoClient_DIR}
  PATHS ${UaoClient_SEARCH_PATHS}
  PATH_SUFFIXES
    include
    include/UaoClient
 )


find_library(UaoClientForOpcUaSca_LIBRARY
  NAMES
    UaoClientForOpcUaSca
    libUaoClientForOpcUaSca.so
    libUaoClientForOpcUaSca.a
  HINTS ${UaoClient_DIR}
  PATHS ${UaoClient_SEARCH_PATHS}
  PATH_SUFFIXES lib
  )

### Somehow get the version
  ### This is hardcoded to the version we pull from GitLab...
if(UaoClientForOpcUaSca_LIBRARY)
  set(UaoClient_VERSION_MAJOR 1)
  set(UaoClient_VERSION_MINOR 5)
  set(UaoClient_VERSION_PATCH 0)
  set(UaoClient_VERSION "${UaoClient_VERSION_MAJOR}.${UaoClient_VERSION_MINOR}.${UaoClient_VERSION_PATCH}")
  set(UaoClient_FIND_VERSION_COMPLETE ${UaoClient_VERSION})
  #   list(GET VERSION_LIST 1 UaoClient_VERSION_MAJOR)
  #   list(GET VERSION_LIST 2 UaoClient_VERSION_MINOR)
  #   list(GET VERSION_LIST 3 UaoClient_VERSION_PATCH)

  #   string(REGEX REPLACE "^[0]?([0-9]+)[^0-9]*$" "\\1" UaoClient_VERSION_PATCH ${UaoClient_VERSION_PATCH})
  #   string(REGEX REPLACE "^[0]?([0-9]+)[^0-9]*$" "\\1" UaoClient_VERSION_MINOR ${UaoClient_VERSION_MINOR})
  #   string(REGEX REPLACE "^[0]?([0-9]+)[^0-9]*$" "\\1" UaoClient_VERSION_MAJOR ${UaoClient_VERSION_MAJOR})

  #   set(UaoClient_VERSION "${UaoClient_VERSION_MAJOR}.${UaoClient_VERSION_MINOR}.${UaoClient_VERSION_PATCH}")
  #   set(UaoClient_FIND_VERSION_COMPLETE ${UaoClient_VERSION})

  #   ## This may be too strict, but for now, if the version is not an
  #   ## exact match to that specified, it is marked as incompatible
  #   if(UaoClient_FIND_VERSION)
  #     if(${UaoClient_VERSION} VERSION_EQUAL ${UaoClient_FIND_VERSION})
  #       set(UaoClient_VERSION_EXACT TRUE)
  #       set(UaoClient_VERSION_UNSUITABLE FALSE)
  #       set(UaoClient_VERSION_COMPATIBLE TRUE)
  #     else()
  #       set(UaoClient_VERSION_EXACT FALSE)
  #       set(UaoClient_VERSION_UNSUITABLE TRUE)
  #       set(UaoClient_VERSION_COMPATIBLE FALSE)
  #     endif()
  #   endif()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(UaoClient
  FOUND_VAR UaoClient_FOUND
  REQUIRED_VARS
    UaoClient_INCLUDE_DIR
    UaoClientForOpcUaSca_LIBRARY
    UaoClient_LIB_DIR
  VERSION_VAR UaoClient_VERSION
)

if(UaoClient_FOUND)
  if(NOT UaoClient_FIND_QUIETLY)
    message(STATUS "Found UaoClient version: " ${UaoClient_VERSION})
  endif()

  if(UaoClientForOpcUaSca_LIBRARY AND NOT TARGET UaoClient::UaoClientForOpcUaSca)
    find_package(Protobuf REQUIRED COMPONENTS libprotobuf)

    add_library(UaoClient::UaoClientForOpcUaSca INTERFACE IMPORTED GLOBAL)
    set_target_properties(UaoClient::UaoClientForOpcUaSca
      PROPERTIES
        IMPORTED_LIBNAME
          UaoClientForOpcUaSca
        INTERFACE_LINK_DIRECTORIES
          "${UaoClient_LIB_DIR}"
        INTERFACE_INCLUDE_DIRECTORIES
          "${UaoClient_INCLUDE_DIR};${Protobuf_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES
          "Open62541Compat::open62541-compat;${Protobuf_LIBRARIES}"
    )
  endif()

  ## Alias targets
  if(NOT TARGET UaoClientForOpcUaSca::UaoClientForOpcUaSca)
    add_library(UaoClientForOpcUaSca::UaoClientForOpcUaSca ALIAS UaoClient::UaoClientForOpcUaSca)
  endif()

  if(NOT TARGET UaoClientForOpcUaSca)
    add_library(UaoClientForOpcUaSca ALIAS UaoClient::UaoClientForOpcUaSca)
  endif()
endif()

mark_as_advanced(UaoClient_LIB_DIR UaoClient_INCLUDE_DIR UaoClient_LIBRARIES)

unset(UaoClient_SEARCH_PATHS)
