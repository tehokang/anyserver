cmake_minimum_required(VERSION 2.8)

set(project_name example)
project(${project_name} CXX)

set(rootdir ${CMAKE_SOURCE_DIR}/example)
set(anyserverdir ${CMAKE_SOURCE_DIR}/anyserver)
set(SOURCES
	${rootdir}/main.cpp
)

include_directories(
	${anyserverdir}/include
	${anyserverdir}/external/json
	${anyserverdir}/external/libmicrohttpd/include
	${anyserverdir}/external/libwebsockets/include
)

add_definitions(-std=c++11 -fexceptions)
link_directories(
	${CMAKE_BINARY_DIR}
	${anyserverdir}/external/libmicrohttpd/lib/${CHIP_CORE}
	${anyserverdir}/external/libwebsockets/lib/${CHIP_CORE}
)

add_executable(${project_name} ${SOURCES})
target_link_libraries(${project_name} websockets microhttpd anyserver)
