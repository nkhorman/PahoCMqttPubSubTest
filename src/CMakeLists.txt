cmake_minimum_required(VERSION 3.10)

project(pubsub LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(pubsub main.cpp MqttSubPub.cpp)

set(MQTT_I ${CMAKE_BINARY_DIR}/mqtt/src/mqtt-build/include)
set(MQTT_L ${CMAKE_BINARY_DIR}/mqtt/src/mqtt-build/lib)

if(BUILD_MQTT_W_SSL)
	set(MQTT_LIB libpaho-mqtt3cs.a)
	if(BUILD_MQTT_W_LIBRESSL)
		set(LIBRESSL_STATIC libssl.a libtls.a libcrypto.a )
		set(LIBRESSL_DIR ${CMAKE_BINARY_DIR}/libressl/src/libressl-build)
		set(LIBRESSL_INCLUDE ${LIBRESSL_DIR}/include)
		set(LIBRESSL_LIB ${LIBRESSL_DIR}/lib)
		list(APPEND MQTT_LIB ${LIBRESSL_STATIC})
	else()
		list(APPEND MQTT_LIB ssl crypto)
	endif()
else()
	set(MQTT_LIB libpaho-mqtt3c.a)
endif()

target_include_directories(pubsub PRIVATE ${MQTT_I} ${LIBRESSL_INCLUDE})
target_link_directories(pubsub PRIVATE ${MQTT_L} ${LIBRESSL_LIB})

target_link_libraries(pubsub PRIVATE ${MQTT_LIB} pthread)
target_compile_definitions(pubsub PRIVATE MQTT_STAGE)
