cmake_minimum_required(VERSION 3.15)
project(iohub)

set(CMAKE_CXX_STANDARD 11)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/out)

file(GLOB SRC_LIST ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_library(iohub SHARED ${SRC_LIST})
add_library(iohub_static STATIC ${SRC_LIST})

set_target_properties(iohub PROPERTIES OUTPUT_NAME "iohub")
set_target_properties(iohub_static PROPERTIES OUTPUT_NAME "iohub")
