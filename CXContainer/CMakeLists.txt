#CMakeLists.txt
cmake_minimum_required (VERSION 2.6)

#project name
PROJECT(CXContainer)

INCLUDE_DIRECTORIES(
../CXCommon/include
../CXLock/include
 ./include
)

aux_source_directory(./src DIR_SRCS)
file (GLOB HEADERS "./include/*.h")
SET(DIR_SRCS ${DIR_SRCS} ${HEADERS})
source_group("Header Files" FILES ${HEADERS})


IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    #SET(option WIN32)
    SET(CMAKE_BUILD_TYPE "Debug")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    SET(CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
    LINK_DIRECTORIES(../../lib/debug)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
    SET( CMAKE_CXX_FLAGS "-std=c++11 -O0")
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")

ADD_LIBRARY(CXContainer SHARED ${DIR_SRCS})
#添加静态库
ADD_LIBRARY(CXContainer_static STATIC ${DIR_SRCS})

SET(LIBRARY_OUTPUT_PATH ../../lib)

#SET(EXECUTABLE_OUTPUT_PATH ../../bin)
#add_executable(CXContainer ${DIR_SRCS} main.cpp )

IF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    TARGET_LINK_LIBRARIES(CXContainer CXCommon_static.lib)
	TARGET_LINK_LIBRARIES(CXContainer CXLock_static.lib)
	
	TARGET_LINK_LIBRARIES(CXContainer_static CXCommon_static.lib)
    TARGET_LINK_LIBRARIES(CXContainer_static CXLock_static.lib)
ELSE (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    LINK_DIRECTORIES(../../lib)
    TARGET_LINK_LIBRARIES(CXContainer        libCXCommon.so)
	TARGET_LINK_LIBRARIES(CXContainer        libCXLock.so)
    TARGET_LINK_LIBRARIES(CXContainer_static        libCXCommon_static.a)
	TARGET_LINK_LIBRARIES(CXContainer_static        libCXLock_static.a)
ENDIF (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")



#生成动态库的版本号
#SET_TARGET_PROPERTIES(CXContainer PROPERTIES VERSION 1.0 SOVERSION 1)
#SET_TARGET_PROPERTIES(CXContainer_static PROPERTIES VERSION 1.0 SOVERSION 1)


