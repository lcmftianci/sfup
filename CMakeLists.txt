#cmake versions
cmake_minimum_required(VERSION 2.8)
 
PROJECT (sfup)
 
SET(CMAKE_BUILD_TYPE "Debug")
 
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS_DEBUG} -Wl,--rpath=./lib/ -m64 -Wall -o0 -g -ggdb -pthread -std=c++11 -fPIC")
 
#SET(SRC_LIST sfu_media/src/main.cpp)
aux_source_directory(${PROJECT_SOURCE_DIR}/sfu_media/src/ MEDIA_SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/sfu_signal_server/src/ SIGNAL_SRC_LIST)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
 
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out)
 
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/log4z/src/ ${PROJECT_SOURCE_DIR}/sfu_core/src/)
 
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib  /usr/local/lib)
 
add_definitions(-D_TTS_)
add_definitions(-g)
 
ADD_SUBDIRECTORY(sfu_core)
ADD_SUBDIRECTORY(third_party/log4z)
ADD_SUBDIRECTORY(third_party/websocketpp)
ADD_SUBDIRECTORY(third_party/jsoncpp)


#add_custom_target(TARGET ALL DEPENDS COPY_RES_MEDIA)
 
ADD_EXECUTABLE(sfu_media ${MEDIA_SRC_LIST})
ADD_EXECUTABLE(sfu_signal_server ${SIGNAL_SRC_LIST})

add_custom_command(TARGET  sfu_media
		POST_BUILD
		COMMAND cp ${PROJECT_SOURCE_DIR}/sfu_media/sfu_media.json ${PROJECT_SOURCE_DIR}/out/)

add_custom_command(TARGET  sfu_signal_server
		POST_BUILD
		COMMAND echo "copy config file"
		COMMAND cp ${PROJECT_SOURCE_DIR}/sfu_signal_server/sfu_signal.json ${PROJECT_SOURCE_DIR}/out/)
		
#TARGET_LINK_LIBRARIES(sfu_media sfu_core log4z)
TARGET_LINK_LIBRARIES(sfu_media sfu_core log4z)
TARGET_LINK_LIBRARIES(sfu_signal_server sfu_core log4z boost_date_time boost_system boost_filesystem boost_thread boost_random jsoncpp_lib)
