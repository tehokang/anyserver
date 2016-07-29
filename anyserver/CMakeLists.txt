cmake_minimum_required(VERSION 2.8)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(project_name anyserver)
project(${project_name} CXX)

if ("${CMAKE_BUILD_TYPE}" MATCHES "release")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2")
else ("${CMAKE_BUILD_TYPE}" MATCHES "debug")
	set(CMAKE_CXX_FLAGS_DEBUG "-DCONFIG_DEBUG -O -g")
endif ("${CMAKE_BUILD_TYPE}" MATCHES "release")

set(rootdir ${CMAKE_SOURCE_DIR}/anyserver)
set(SOURCES 
	${rootdir}/src/anylogger.cpp
	${rootdir}/src/anyserver.cpp
	${rootdir}/src/anyserver_configuration.cpp
	${rootdir}/src/anyserver_controller.cpp
	${rootdir}/src/anyserver_factory.cpp
	${rootdir}/src/http_tcp_server.cpp
	${rootdir}/src/inet_domainsocket_tcp_server.cpp
	${rootdir}/src/inet_domainsocket_udp_server.cpp
	${rootdir}/src/session.cpp
	${rootdir}/src/session_communicator.cpp
	${rootdir}/src/session_processor.cpp
	${rootdir}/src/unix_domainsocket_tcp_server.cpp
	${rootdir}/src/unix_domainsocket_udp_server.cpp
	${rootdir}/src/websocket_tcp_server.cpp
	${rootdir}/src/posix_signal_interceptor.cpp
	${rootdir}/external/json/jsoncpp.cpp
)

include_directories(
	${rootdir}/src
	${rootdir}/include
	${rootdir}/port/include
	${rootdir}/external/json
	${rootdir}/external/libmicrohttpd/include
	${rootdir}/external/libwebsockets/include
)

add_definitions(-std=c++11 -fexceptions)

link_directories(
	${rootdir}/external/libmicrohttpd/lib/${CHIP_CORE}
	${rootdir}/external/libwebsockets/lib/${CHIP_CORE}
)

add_library(${project_name} SHARED ${SOURCES})

# Version configuration
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 1)

set_target_properties(
	${project_name} 
	PROPERTIES 
	VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" 
	SOVERSION "${VERSION_MAJOR}"
)

target_link_libraries(${project_name} websockets microhttpd pthread)


