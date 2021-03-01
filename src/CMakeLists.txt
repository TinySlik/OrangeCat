cmake_minimum_required(VERSION 3.2)
project(orangecat_src)

# add_subdirectory(samples)
add_subdirectory(paramserv)
set(SOURCE_FILES ${PROJECT_SOURCE_DIR}/samples/paramserv.cpp)

include_directories(log)
include_directories(paramserv)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} paramserv)
install(TARGETS ${PROJECT_NAME} DESTINATION ${ORANGECAT_CMAKE_INSTALLDIR})