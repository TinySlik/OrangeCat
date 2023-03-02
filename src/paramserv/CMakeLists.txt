cmake_minimum_required (VERSION 3.1.0)

project(paramserv)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

add_subdirectory(base64)

include_directories(../log)

add_definitions(-D_WIN_PARAM_SERV_EXPORTING)

add_library(${PROJECT_NAME}  SHARED
                        mongoose.h
                        mongoose.c
                        parameterserver.h
                        parameterserver.cpp
                        urldecode.h
                        urldecode.cpp
                        simplethread.h
                        configuru.hpp
                        windllsupport.h
                            # library files
                        # base64/lib/lib.c
                        # base64/lib/codec_choose.c
                        # base64/include/libbase64.h
                        
                        # base64/lib/tables/tables.c
                        # # Add generated headers explicitly to target, to insert them in the dependency tree
                        # base64/lib/tables/table_dec_32bit.h
                        # base64/lib/tables/table_enc_12bit.h

                        # # codec implementations
                        # base64/lib/arch/generic/codec.c

                        # base64/lib/arch/ssse3/codec.c
                        # base64/lib/arch/sse41/codec.c
                        # base64/lib/arch/sse42/codec.c
                        # base64/lib/arch/avx/codec.c
                        # base64/lib/arch/avx2/codec.c
                        # base64/lib/arch/avx512/codec.c
    
                        # base64/lib/arch/neon64/codec.c
                        # base64/lib/arch/neon32/codec.c
                        )

target_link_libraries(${PROJECT_NAME} base64)
