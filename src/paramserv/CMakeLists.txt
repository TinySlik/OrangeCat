cmake_minimum_required (VERSION 3.1.0)

project(paramserv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

include_directories(../log)

add_definitions(-D_WIN_PARAM_SERV_EXPORTING)

add_library(${PROJECT_NAME}  SHARED
                        mongoose.h
                        mongoose.c
                        parameterserver.h
                        parameterserver.cpp
                        urldecode.h
                        urldecode.cpp
                        configuru.hpp
                        windllsupport.h
                        )

target_link_libraries(${PROJECT_NAME})
