#in case UaoClientForOpcUaSca is used as a quasar module ... (e.g. for a peripheral server or so
#then we provide this CMakeLists.txt which defined OBJECT library

file(GLOB SOURCES src/*.cpp )
add_library(UaoClientForOpcUaSca OBJECT ${SOURCES} )
