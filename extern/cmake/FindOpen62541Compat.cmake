# - Try to find Open62541Compat libraries
#
# Once done this will define the following variables:
#  Open62541Compat_FOUND - system has Open62541Compat software stack available
#  Open62541Compat_LIB_DIR - the Open62541Compat lib directory
#  Open62541Compat_INCLUDE_DIR - the Open62541Compat include directory
#  Open62541Compat_LIBRARY - the Open62541Compat library
#  open62541_INCLUDE_DIR - the open62541 include directory
#  open62541_LIBRARY - the open62541 library
#  LogIt_INCLUDE_DIR - the LogIt include directory, if present
#  LogIt_LIBRARY - the LogIt library, if present

#  Additionally, the following targets will be available
#   -  Open62541Compat::open62541-compat
#   -  Open62541Compat::open62541
#   -  Open62541Compat::LogIt
#   -  open62541-compat
#   -  open62541
#   -  LogIt
#
# To override the default search paths, point Open62541Compat_DIR to the location of your
# open62541-compat installation
#
#  -DOpen62541Compat_DIR=/path/to/open62541-compat/installed
#

## Maybe want to be able to select Open62541Compat version and compiler profile here?
set(Open62541Compat_SEARCH_PATHS "")

message(VERBOSE "Looking for Open62541Compat dependencies in " ${Open62541Compat_DIR} ${Open62541Compat_SEARCH_PATHS})

find_path(Open62541Compat_LIB_DIR
  NAMES
    open62541-compat
    libopen62541-compat.so
    libopen62541-compat.a
  HINTS ${Open62541Compat_DIR}
  PATHS ${Open62541Compat_SEARCH_PATHS}
  PATH_SUFFIXES lib
 )

find_path(Open62541Compat_INCLUDE_DIR
  NAMES open62541_compat_common.h
  HINTS ${Open62541Compat_DIR}
  PATHS ${Open62541Compat_SEARCH_PATHS}
  PATH_SUFFIXES
    include
    include/Open62541Compat
 )

find_library(Open62541Compat_LIBRARY
  NAMES
    open62541-compat
    libopen62541-compat.so
    libopen62541-compat.a
  HINTS ${Open62541Compat_DIR}
  PATHS ${Open62541Compat_SEARCH_PATHS}
  PATH_SUFFIXES lib
  )

find_path(open62541_INCLUDE_DIR
  NAMES open62541.h
  HINTS ${Open62541Compat_DIR}
  PATHS ${Open62541Compat_SEARCH_PATHS}
  PATH_SUFFIXES include
 )

find_path(open62541_LIBRARY
  NAMES
    open62541
    libopen62541.so
    libopen62541.a
  HINTS ${Open62541Compat_DIR}
  PATHS ${Open62541Compat_SEARCH_PATHS}
  PATH_SUFFIXES lib
 )

find_path(LogIt_INCLUDE_DIR
  NAMES LogIt.h
  HINTS ${Open62541Compat_DIR}
  PATHS ${Open62541Compat_SEARCH_PATHS}
  PATH_SUFFIXES
    include
    include/LogIt
 )

find_path(LogIt_LIBRARY
  NAMES
    LogIt
    libLogIt.so
    libLogIt.a
  HINTS ${Open62541Compat_DIR}
  PATHS ${Open62541Compat_SEARCH_PATHS}
  PATH_SUFFIXES lib
 )

if(Open62541Compat_LIBRARY)
  ### Somehow get the version?
  set(Open62541Compat_VERSION_MAJOR 1)
  set(Open62541Compat_VERSION_MINOR 3)
  set(Open62541Compat_VERSION_PATCH 9)
  set(Open62541Compat_VERSION "${Open62541Compat_VERSION_MAJOR}.${Open62541Compat_VERSION_MINOR}.${Open62541Compat_VERSION_PATCH}")
  set(Open62541Compat_FIND_VERSION_COMPLETE ${Open62541Compat_VERSION})
  #   list(GET VERSION_LIST 1 Open62541Compat_VERSION_MAJOR)
  #   list(GET VERSION_LIST 2 Open62541Compat_VERSION_MINOR)
  #   list(GET VERSION_LIST 3 Open62541Compat_VERSION_PATCH)

  #   string(REGEX REPLACE "^[0]?([0-9]+)[^0-9]*$" "\\1" Open62541Compat_VERSION_PATCH ${Open62541Compat_VERSION_PATCH})
  #   string(REGEX REPLACE "^[0]?([0-9]+)[^0-9]*$" "\\1" Open62541Compat_VERSION_MINOR ${Open62541Compat_VERSION_MINOR})
  #   string(REGEX REPLACE "^[0]?([0-9]+)[^0-9]*$" "\\1" Open62541Compat_VERSION_MAJOR ${Open62541Compat_VERSION_MAJOR})

  #   set(Open62541Compat_VERSION "${Open62541Compat_VERSION_MAJOR}.${Open62541Compat_VERSION_MINOR}.${Open62541Compat_VERSION_PATCH}")
  #   set(Open62541Compat_FIND_VERSION_COMPLETE ${Open62541Compat_VERSION})

  #   ## This may be too strict, but for now, if the version is not an
  #   ## exact match to that specified, it is marked as incompatible
  #   if(Open62541Compat_FIND_VERSION)
  #     if(${Open62541Compat_VERSION} VERSION_EQUAL ${Open62541Compat_FIND_VERSION})
  #       set(Open62541Compat_VERSION_EXACT TRUE)
  #       set(Open62541Compat_VERSION_UNSUITABLE FALSE)
  #       set(Open62541Compat_VERSION_COMPATIBLE TRUE)
  #     else()
  #       set(Open62541Compat_VERSION_EXACT FALSE)
  #       set(Open62541Compat_VERSION_UNSUITABLE TRUE)
  #       set(Open62541Compat_VERSION_COMPATIBLE FALSE)
  #     endif()
  #   endif()
endif()

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Open62541Compat
  FOUND_VAR Open62541Compat_FOUND
  REQUIRED_VARS
    Open62541Compat_LIB_DIR
    Open62541Compat_INCLUDE_DIR
    Open62541Compat_LIBRARY
    open62541_INCLUDE_DIR
    open62541_LIBRARY
  VERSION_VAR Open62541Compat_VERSION
)

if(Open62541Compat_FOUND)
  if(NOT Open62541Compat_FIND_QUIETLY)
    # message(STATUS "Open62541Compat version: " ${Open62541Compat_VERSION})
  endif()

  if(open62541_LIBRARY AND NOT TARGET Open62541Compat::open62541)
      add_library(Open62541Compat::open62541 INTERFACE IMPORTED GLOBAL)
      set_target_properties(Open62541Compat::open62541
          PROPERTIES
          IMPORTED_LIBNAME
            open62541
          INTERFACE_LINK_DIRECTORIES
            "${Open62541Compat_LIB_DIR}"
          INTERFACE_INCLUDE_DIRECTORIES
            "${open62541_INCLUDE_DIR}"
	  INTERFACE_LINK_LIBRARIES ""
      )
    endif()

  if(Open62541Compat_LIBRARY AND NOT TARGET Open62541Compat::open62541-compat)
      add_library(Open62541Compat::open62541-compat INTERFACE IMPORTED GLOBAL)
      set_target_properties(Open62541Compat::open62541-compat
          PROPERTIES
          IMPORTED_LIBNAME
            open62541-compat
          INTERFACE_LINK_DIRECTORIES
            "${Open62541Compat_LIB_DIR}"
          INTERFACE_INCLUDE_DIRECTORIES
            "${Open62541Compat_INCLUDE_DIR}"
	  INTERFACE_LINK_LIBRARIES ""
      )
    endif()

  if(LogIt_LIBRARY AND NOT TARGET Open62541Compat::LogIt)
      add_library(Open62541Compat::LogIt INTERFACE IMPORTED GLOBAL)
      set_target_properties(Open62541Compat::LogIt
          PROPERTIES
          IMPORTED_LIBNAME
            LogIt
          INTERFACE_LINK_DIRECTORIES
            "${Open62541Compat_LIB_DIR}"
          INTERFACE_INCLUDE_DIRECTORIES
            "${LogIt_INCLUDE_DIR};${Open62541Compat_INCLUDE_DIR}"
	  INTERFACE_LINK_LIBRARIES ""
          )
  endif()

  ## Alias targets
  if(NOT TARGET open62541)
      add_library(open62541 ALIAS Open62541Compat::open62541)
  endif()

  if(NOT TARGET open62541-compat)
      add_library(open62541-compat ALIAS Open62541Compat::open62541-compat)
  endif()

  if(NOT TARGET Open62541Compat::LogIt)
      add_library(Open62541Compat::LogIt ALIAS Open62541Compat::open62541-compat)
      if(NOT TARGET LogIt)
          add_library(LogIt ALIAS Open62541Compat::open62541-compat)
      endif()
  else()
      if(NOT TARGET LogIt)
          add_library(LogIt ALIAS Open62541Compat::LogIt)
      endif()
  endif()
endif()

mark_as_advanced(Open62541Compat_LIB_DIR Open62541Compat_INCLUDE_DIR open62541_INCLUDE_DIR LogIt_INCLUDE_DIR Open62541Compat_LIBRARIES)

unset(Open62541Compat_SEARCH_PATHS)
