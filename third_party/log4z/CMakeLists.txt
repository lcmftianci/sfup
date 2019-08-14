#cmake versions
cmake_minimum_required(VERSION 2.8)
 
PROJECT (log4z)
 
SET(CMAKE_BUILD_TYPE "Debug")
 
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS_DEBUG} -Wl,--rpath=./lib/ -m64 -Wall -o0 -g -ggdb -pthread -std=c++11 -fPIC")
 
#SET(SRC_LIST ${PROJECT_SOURCE_DIR}/src/*.cpp)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/ SRC_LIST) 

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
 
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../out)
 
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include/interface ${PROJECT_SOURCE_DIR}/include)
 
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib  /usr/local/lib)
 
add_definitions(-D_TTS_)
add_definitions(-g)
 
#ADD_SUBDIRECTORY(sfu_core)
 
#ADD_EXECUTABLE(sfu_media ${SRC_LIST})

ADD_LIBRARY(log4z SHARED ${SRC_LIST})

TARGET_LINK_LIBRARIES(log4z pthread)
