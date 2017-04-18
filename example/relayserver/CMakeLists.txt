cmake_minimum_required(VERSION 2.8)

set(project_name relayserver)
project(${project_name} CXX)

set(rootdir ${CMAKE_SOURCE_DIR}/example/relayserver)
set(anyserverdir ${CMAKE_SOURCE_DIR}/anyserver)
set(sources
	${rootdir}/main.cpp
)

include_directories(
	${anyserverdir}/include
)

add_definitions(-std=c++11 -fexceptions)
link_directories(
	${CMAKE_BINARY_DIR}
	${anyserverdir}/external/libwebsockets/lib/${CHIP_CORE}
    ${anyserverdir}/external/openssl/lib/${CHIP_CORE}
    ${anyserverdir}/external/zlib/lib/${CHIP_CORE}
)

add_executable(${project_name} ${sources})
target_link_libraries(${project_name} z ssl websockets anyserver)
