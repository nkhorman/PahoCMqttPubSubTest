cmake_minimum_required(VERSION 3.16)

project(test LANGUAGES CXX)

include(ExternalProject)
include(FetchContent)

# Enable C++11 support
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_MQTT_W_SSL ON)

# message(STATUS "Build target ${BUILD_TARGET}")
if("${BUILD_TARGET}" STREQUAL "NEST")
	set(BUILD_MQTT_W_LIBRESSL ON)
else()
	set(BUILD_MQTT_W_LIBRESSL OFF)
endif()

if(BUILD_MQTT_W_LIBRESSL)
	add_subdirectory("vendor/libressl")
endif()
add_subdirectory("vendor/mqtt")
add_subdirectory("src")
