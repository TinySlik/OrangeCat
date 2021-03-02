# Copyright 2021 Tiny Oh, Ltd. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.0)

project(OrangeCat VERSION 0.0.1 LANGUAGES C CXX)

include(cmake/Common.cmake)

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX d)
endif()

LIST(APPEND CMAKE_MODULE_PATH cmake)

# options

include(cmake/Option.cmake)

# flags

if(OS_WIN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
else()
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
endif()

include(cmake/DetectCXX11.cmake)

# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=native")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")

string(STRIP "${CMAKE_C_FLAGS}" CMAKE_C_FLAGS)
string(STRIP "${CMAKE_CXX_FLAGS}" CMAKE_CXX_FLAGS)
message(STATUS "C_FLAGS: ${CMAKE_C_FLAGS}")
message(STATUS "CXX_FLAGS: ${CMAKE_CXX_FLAGS}")

# packages

find_package(Threads QUIET)

macro(target_link_threads NAME)
  if(THREADS_HAVE_PTHREAD_ARG)
    target_compile_options(PUBLIC ${NAME} "-pthread")
  endif()
  if(CMAKE_THREAD_LIBS_INIT)
    target_link_libraries(${NAME} PUBLIC "${CMAKE_THREAD_LIBS_INIT}")
  endif()
endmacro()

include(CMakePackageConfigHelpers)

# config

set(ORANGECAT ${PROJECT_NAME})

set(ORANGECAT_SPACE "orangecat")
message(STATUS "Namespace: ${ORANGECAT_SPACE}")

if(MSVC)
  string(REPLACE "/" "\\\\" ORANGECAT_SDK_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
  string(REPLACE "/" "\\\\" ORANGECAT_SDK_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}")
else()
  file(TO_NATIVE_PATH "${CMAKE_CURRENT_SOURCE_DIR}" ORANGECAT_SDK_ROOT_DIR)
  file(TO_NATIVE_PATH "${CMAKE_INSTALL_PREFIX}" ORANGECAT_SDK_INSTALL_DIR)
endif()

# configure_file(
#   include/mynteye/mynteye.h.in
#   include/mynteye/mynteye.h @ONLY
# )

# configure_file(
#   cmake/templates/pkginfo.sh.in
#   ${CMAKE_CURRENT_SOURCE_DIR}/pkginfo.sh @ONLY
# )
# if(OS_WIN)
#   configure_file(
#     scripts/win/nsis/winpack.nsi.in
#     ${CMAKE_CURRENT_SOURCE_DIR}/winpack.nsi @ONLY
#   )
# endif()

# rpath

# set(CMAKE_MACOSX_RPATH 1)
# set(MYNTEYE_CMAKE_RPATH "")
# if(WITH_OPENCV)
#   list(APPEND MYNTEYE_CMAKE_RPATH ${OpenCV_LIB_PATH})
# endif()
# if(MYNTEYE_CMAKE_RPATH)
#   message(STATUS "RPATH: ${MYNTEYE_CMAKE_RPATH}")
#   set(CMAKE_INSTALL_RPATH "${MYNTEYE_CMAKE_RPATH}")
# endif()

# targets

add_definitions(-DORANGECAT_EXPORTS)

set(OUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_output")
set_outdir(
  "${OUT_DIR}/lib"
  "${OUT_DIR}/lib"
  "${OUT_DIR}/bin"
)

# set_target_properties(${TO_PIPELINE} PROPERTIES
#   VERSION ${PROJECT_VERSION}
#   SOVERSION ${PROJECT_VERSION_MAJOR}
# )

# install

set(ORANGECAT_CMAKE_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/include/${TO_PIPELINE}")
set(ORANGECAT_CMAKE_BINDIR "${CMAKE_INSTALL_PREFIX}/bin")
set(ORANGECAT_CMAKE_LIBDIR "${CMAKE_INSTALL_PREFIX}/lib")
set(ORANGECAT_CMAKE_INSTALLDIR "${ORANGECAT_CMAKE_LIBDIR}/cmake/${TO_PIPELINE}")

# install(TARGETS ${TO_PIPELINE}
#   EXPORT ${TO_PIPELINE}-targets
#   RUNTIME DESTINATION ${ORANGECAT_CMAKE_BINDIR}
#   LIBRARY DESTINATION ${ORANGECAT_CMAKE_LIBDIR}
#   ARCHIVE DESTINATION ${ORANGECAT_CMAKE_LIBDIR}
# )

# configure_package_config_file(mynteye-config.cmake.in
#   ${CMAKE_CURRENT_BINARY_DIR}/mynteye-config.cmake
#   INSTALL_DESTINATION ${ORANGECAT_CMAKE_INSTALLDIR}
#   NO_CHECK_REQUIRED_COMPONENTS_MACRO
# )

# write_basic_package_version_file(mynteye-config-version.cmake
#   VERSION ${PROJECT_VERSION} COMPATIBILITY SameMajorVersion
# )

# install(FILES
#   ${CMAKE_CURRENT_BINARY_DIR}/mynteye-config.cmake
#   ${CMAKE_CURRENT_BINARY_DIR}/mynteye-config-version.cmake
#   DESTINATION ${MYNTEYE_CMAKE_INSTALLDIR}
# )

# install(EXPORT ${TO_PIPELINE}-targets
#   DESTINATION ${ORANGECAT_CMAKE_INSTALLDIR}
# )

add_subdirectory(src)
set(RES_DIR_OUTPUT ${OUT_DIR}/res)
set(RES_DIR ${PROJECT_SOURCE_DIR}/res)
add_custom_command(OUTPUT  ${RES_DIR_OUTPUT}
  COMMAND echo "Generating res dir"
  COMMAND ${CMAKE_COMMAND} -E copy_directory ${RES_DIR}  ${RES_DIR_OUTPUT}
  COMMENT  "res file copy"
 )
add_custom_target(res ALL DEPENDS ${RES_DIR_OUTPUT})


add_custom_command(TARGET res
  POST_BUILD 
  COMMAND echo "executing a res command"
 )

## build.info

# set default int values for yaml file (build.info)
set_default_value(CMAKE_CXX_COMPILER_VERSION_MAJOR 0)
set_default_value(CMAKE_CXX_COMPILER_VERSION_MINOR 0)
set_default_value(CMAKE_CXX_COMPILER_VERSION_PATCH 0)
set_default_value(CMAKE_CXX_COMPILER_VERSION_TWEAK 0)
set_default_value(${PROJECT_NAME}_VERSION_MAJOR 0)
set_default_value(${PROJECT_NAME}_VERSION_MINOR 0)
set_default_value(${PROJECT_NAME}_VERSION_PATCH 0)
set_default_value(${PROJECT_NAME}_VERSION_TWEAK 0)

# configure_file(
#   cmake/templates/build.info.in
#   build.info @ONLY
# )

# install(FILES
#   ${CMAKE_CURRENT_BINARY_DIR}/build.info
#   DESTINATION ${CMAKE_INSTALL_PREFIX}/share/mynteye
# )
