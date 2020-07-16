project(UAO_SCA_DEMO CXX)
cmake_minimum_required(VERSION 2.8)

add_definitions( -std=c++0x )

include_directories(
	build/open62541-compat/include/
	build/open62541-compat/build/open62541
	build/open62541-compat/build/LogIt/include
	../include/
	)

file(GLOB SOURCES ../src/*.cpp)

message( ${PROJECT_BINARY_DIR}/open62541-compat/build/libopen62541-compat.a)

add_library( this OBJECT ${SOURCES})

add_executable(
	demo
	demo.cpp
	$<TARGET_OBJECTS:this>
	)

add_executable(
        demo_adc_samples
        demo_adc_samples.cpp
        $<TARGET_OBJECTS:this>
        )
        
add_executable(
        demo_send_bitfile
        demo_send_bitfile.cpp
        $<TARGET_OBJECTS:this>
        )

set(COMMON_LIBS
	${PROJECT_BINARY_DIR}/open62541-compat/build/libopen62541-compat.a
        ${PROJECT_BINARY_DIR}/open62541-compat/build/open62541/libopen62541.a
        -lboost_program_options-mt
	)

target_link_libraries(
	demo
	${COMMON_LIBS}
	)

target_link_libraries(
        demo_adc_samples
	${COMMON_LIBS}
        )

target_link_libraries(
        demo_send_bitfile
        ${COMMON_LIBS}
        )