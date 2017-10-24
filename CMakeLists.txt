cmake_minimum_required(VERSION 3.4)

# Standalone CMake file. If this is used in TDAQ release, need to be changed to TDAQ style

set(CMAKE_CXX_FLAGS " -std=c++14 -Wall -fPIC") 

find_package(LibXml2 REQUIRED)
find_package(Boost COMPONENTS system program_options REQUIRED )

message(STATUS "LibXML2 INCLUDE DIR IS: " ${LIBXML2_INCLUDE_DIR})

SET(LCG_TAG $ENV{LCGPLAT} )
MESSAGE(STATUS "LCG Software Tag: ${LCG_TAG}")

SET(FREEOPCUA_LIB_DIRS $ENV{FREEOPCUA_INSTALL_PATH}/${LCG_TAG}/usr/local/lib)
SET(FREEOPCUA_INCLUDE_DIRS $ENV{FREEOPCUA_INSTALL_PATH}/${LCG_TAG}/usr/local/include)

include_directories(${FREEOPCUA_INCLUDE_DIRS} ${LIBXML2_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${PROJECT_SOURCE_DIR}/NSWConfiguration)
link_directories(${FREEOPCUA_LIB_DIRS} ${Boost_LIBRARY_DIRS})

add_executable(simple_client app/simple_client.cpp)
target_include_directories(simple_client PUBLIC ${FREEOPCUA_INCLUDE_DIRS})

target_link_libraries(simple_client
    opcuaprotocol
    opcuacore
    opcuaclient
)

add_library(configreader src/ConfigReader.cpp)

target_link_libraries(configreader 
    ${Boost_SYSTEM_LIBRARY}
)

add_executable(read_config app/read_config.cpp)

target_link_libraries(read_config
    configreader
)
