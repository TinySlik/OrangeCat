cmake_minimum_required (VERSION 3.1.0)

project(paramserv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -O3")

include_directories(../log)

# find_package(OpenSSL)

add_definitions(-D_WIN_PARAM_SERV_EXPORTING)

# add_definitions(-DMG_ENABLE_SSL)

add_library(${PROJECT_NAME}  SHARED
                        mongoose.h
                        mongoose.c
                        parameterserver.h
                        parameterserver.cpp
                        urldecode.h
                        urldecode.cpp
                        configuru.hpp
                        windllsupport.h
                        lodepng.cpp
                        )
target_link_libraries(${PROJECT_NAME})
# target_link_libraries(${PROJECT_NAME} ssl crypto)
