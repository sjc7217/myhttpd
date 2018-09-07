cmake_minimum_required(VERSION 3.8)
project(myhttpd)

INCLUDE_DIRECTORIES( include )
#message(${SLIGHTTPD_BINARY_DIR})
add_compile_options(-std=c++11)

include_directories( /usr/local/include)
LINK_DIRECTORIES(/usr/local/lib)
LINK_LIBRARIES(dl uv http_parser)

#source directory
AUX_SOURCE_DIRECTORY(src DIR_SRCS)

add_executable(myhttpd ${DIR_SRCS} include/header.h)