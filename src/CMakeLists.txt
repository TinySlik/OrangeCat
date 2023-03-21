cmake_minimum_required(VERSION 3.2)
project(orangecat_src)

# add_subdirectory(samples)
add_subdirectory(paramserv)
set(SOURCE_FILES ${PROJECT_SOURCE_DIR}/samples/paramserv.cpp)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -O3")

include_directories(log)
include_directories(paramserv)
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-expansion-to-defined -Wno-deprecated-declarations" )
add_executable(paramserv_example ${SOURCE_FILES})
target_link_libraries(paramserv_example paramserv)
install(TARGETS paramserv_example DESTINATION ${ORANGECAT_CMAKE_INSTALLDIR})