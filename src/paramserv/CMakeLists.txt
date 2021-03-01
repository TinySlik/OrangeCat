cmake_minimum_required (VERSION 3.1.0)

project(paramserv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories(../log)

add_library(paramserv  SHARED
                         mongoose.c 
                         parameterserver.cpp
                         urldecode.cpp
                         )
