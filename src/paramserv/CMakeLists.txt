cmake_minimum_required (VERSION 3.1.0)

project(paramserv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

include_directories(../log)
include_directories(./udp-discovery-cpp)

add_definitions(-D_WIN_PARAM_SERV_EXPORTING)

add_library(${PROJECT_NAME}  SHARED
                        udp-discovery-cpp/udp_discovery_ip_port.hpp
                        udp-discovery-cpp/udp_discovery_peer.hpp
                        udp-discovery-cpp/udp_discovery_protocol.hpp
                        udp-discovery-cpp/udp_discovery_ip_port.cpp
                        udp-discovery-cpp/udp_discovery_peer.cpp
                        udp-discovery-cpp/udp_discovery_protocol.cpp
                        mongoose.h
                        mongoose.c
                        parameterserver.h
                        parameterserver.cpp
                        urldecode.h
                        urldecode.cpp
                        simplethread.h
                        configuru.hpp
                        windllsupport.h
                        )
