cmake_minimum_required(VERSION 2.8.4)

# subdir_depends(../to_pipeline_params_server)
set(LIBHELLO_SRC hello.c)

add_executable(main ${LIBHELLO_SRC})
# target_link_libraries(main paramserv)



