tdaq_package() #This will make a package called NSWConfiguration

#set(CMAKE_CXX_FLAGS " -std=c++14 -Wall -fPIC") 

find_package(LibXml2 REQUIRED)
#find_package(Boost COMPONENTS system program_options REQUIRED )

# freeopcua used for OPC UA Client
SET(LCG_TAG $ENV{LCGPLAT} )
SET(FREEOPCUA_LIB_DIRS $ENV{FREEOPCUA_INSTALL_PATH}/${LCG_TAG}/usr/local/lib)
SET(FREEOPCUA_INCLUDE_DIRS $ENV{FREEOPCUA_INSTALL_PATH}/${LCG_TAG}/usr/local/include)

MESSAGE(STATUS "LCG Software Tag: ${LCG_TAG}")
#message(STATUS "LibXML2 include dir: " ${LIBXML2_INCLUDE_DIR})
message(STATUS "FREEOPCUA include dir: " ${FREEOPCUA_INCLUDE_DIRS})

include_directories(${FREEOPCUA_INCLUDE_DIRS})# ${LIBXML2_INCLUDE_DIR})# ${Boost_INCLUDE_DIRS})
link_directories(${FREEOPCUA_LIB_DIRS})# ${Boost_LIBRARY_DIRS})

tdaq_add_executable(simple_client app/simple_client.cpp
  INCLUDE_DIRECTORIES ${FREEOPCUA_INCLUDE_DIRS}
  LINK_LIBRARIES opcuaprotocol opcuacore opcuaclient Boost::boost LibXml2
)

tdaq_add_library(configreader src/ConfigReader.cpp src/ConfigReaderApi.cpp
  LINK_LIBRARIES Boost::boost
)

tdaq_add_executable(read_config app/read_config.cpp
  LINK_LIBRARIES configreader
)
