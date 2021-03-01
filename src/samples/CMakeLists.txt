cmake_minimum_required(VERSION 2.8.4)

project(samples)

subdir_depends()
set(LIBHELLO_SRC hello.c)

add_executable(main ${LIBHELLO_SRC})
target_link_libraries(main paramserv)



