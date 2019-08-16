#cmake versions
cmake_minimum_required(VERSION 2.8)
 
PROJECT (sfup)

option(BUILD_SIGNAL_SERVER "Build jsoncpp_lib as a shared library." OFF)
 
SET(CMAKE_BUILD_TYPE "Debug")
 
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS_DEBUG} -Wl,--rpath=./lib/ -m64 -Wall -o0 -g -ggdb -pthread -std=c++11 -fPIC")
 
#SET(SRC_LIST sfu_media/src/main.cpp)
aux_source_directory(${PROJECT_SOURCE_DIR}/sfu_media/src/ MEDIA_SRC_LIST)
aux_source_directory(${PROJECT_SOURCE_DIR}/sfu_signal_server/src/ SIGNAL_SRC_LIST)

INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})
 
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/out)
 
INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/third_party/log4z/src/ ${PROJECT_SOURCE_DIR}/sfu_core/src/ ${PROJECT_SOURCE_DIR}/third_party/websocketpp/)
 
LINK_DIRECTORIES(${PROJECT_SOURCE_DIR}/lib  /usr/local/lib)
 
add_definitions(-D_TTS_)
add_definitions(-g)
 
ADD_SUBDIRECTORY(sfu_core)
ADD_SUBDIRECTORY(third_party/log4z)
ADD_SUBDIRECTORY(third_party/websocketpp)
ADD_SUBDIRECTORY(third_party/jsoncpp)


#add_custom_target(TARGET ALL DEPENDS COPY_RES_MEDIA)
 
ADD_EXECUTABLE(sfu_media ${MEDIA_SRC_LIST})

if(BUILD_SIGNAL_SERVER)
	ADD_EXECUTABLE(sfu_signal_server ${SIGNAL_SRC_LIST})
else()
	message(WARNING "you select do not build signal server!")
endif()

add_custom_command(TARGET  sfu_media
		POST_BUILD
		COMMAND cp ${PROJECT_SOURCE_DIR}/sfu_media/sfu_media.json ${PROJECT_SOURCE_DIR}/out/)

#execute_process(COMMAND cd ${PROJECT_SOURCE_DIR}/third_party/boost_1_68_0/
#		COMMAND sh bootstrap.sh --prefix=${PROJECT_SOURCE_DIR}/out/boost/
#		)

#add_custom_command(TARGET  sfu_signal_server
#		PRE_BUILD
#		COMMAND cd ${PROJECT_SOURCE_DIR}/third_party/boost_1_68_0/
#		COMMAND sh bootstrap.sh --prefix=${PROJECT_SOURCE_DIR}/out/boost/
#		COMMAND ./b2 install --with-system --with-date_time --with-random --with-regex link=static runtime-link=shared threading=multi debug
#		COMMAND cd -)

#add_custom_command(TARGET  sfu_signal_server
#		PRE_BUILD
#		COMMAND sh ${PROJECT_SOURCE_DIR}/third_party/boost_1_68_0/bootstrap.sh --prefix=${PROJECT_SOURCE_DIR}/out/boost/
#		COMMAND mv ${PROJECT_BINARY_DIR}/b2 ${PROJECT_SOURCE_DIR}/third_party/boost_1_68_0/
#		COMMAND mv ${PROJECT_BINARY_DIR}/bjam ${PROJECT_SOURCE_DIR}/third_party/boost_1_68_0/
#		COMMAND mv ${PROJECT_BINARY_DIR}/project-config.jam ${PROJECT_SOURCE_DIR}/third_party/boost_1_68_0/
#		COMMAND echo "`${PROJECT_SOURCE_DIR}/third_party/boost_1_68_0/b2 install --with-system --with-date_time --with-random --with-regex link=static runtime-link=shared threading=multi debug`")

if(BUILD_SIGNAL_SERVER)
add_custom_command(TARGET  sfu_signal_server
		POST_BUILD
		COMMAND echo "copy config file"
		COMMAND cp ${PROJECT_SOURCE_DIR}/sfu_signal_server/sfu_signal.json ${PROJECT_SOURCE_DIR}/out/)
endif()
		
#TARGET_LINK_LIBRARIES(sfu_media sfu_core log4z)
TARGET_LINK_LIBRARIES(sfu_media sfu_core log4z boost_system ssl crypto)

if(BUILD_SIGNAL_SERVER)
TARGET_LINK_LIBRARIES(sfu_signal_server sfu_core log4z boost_date_time boost_system boost_filesystem boost_thread boost_random boost_wserialization ssl crypto jsoncpp_lib)
endif()
