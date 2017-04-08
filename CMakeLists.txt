cmake_minimum_required(VERSION 2.8.12)


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/src")
message(${CMAKE_MODULE_PATH})

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-std=gnu++0x)
endif()

find_package(ZLIB REQUIRED)
find_package(OpenCV REQUIRED)
#find_package(Boost COMPONENTS thread REQUIRED)
#find_package(Boost COMPONENTS filesystem REQUIRED)
#find_package(Boost COMPONENTS system REQUIRED)
#find_package(Boost COMPONENTS date_time REQUIRED)

#find_package(RealSense REQUIRED)
#message(STATUS "librealsense found: ${REALSENSE_FOUND}")
find_package(LibRealSense REQUIRED)
message(STATUS "librealsense found: ${LIBREALSENSE_FOUND}")
#list(APPEND INTERNAL_INC  ${LIBREALSENSE_INCLUDE_DIRS} )
#list(APPEND LINK_LIBS ${LIBREALSENSE_LIBRARIES} )


add_definitions(-DWITH_REALSENSE)

find_package(PkgConfig)
pkg_check_modules(libusb-1.0 REQUIRED libusb-1.0)



IF(UNIX)
    set(CMAKE_CXX_FLAGS "-O3 -msse2 -msse3")
ENDIF (UNIX)

find_package(Threads REQUIRED)


include_directories(./src)

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIR})
INCLUDE_DIRECTORIES(${ZLIB_INCLUDE_DIR})
include_directories(${LIBREALSENSE_INCLUDE_DIRS})

#set(Boost_USE_STATIC_LIBS OFF)
#set(Boost_USE_MULTITHREADED ON)
#set(Boost_USE_STATIC_RUNTIME OFF)
#set(BOOST_ALL_DYN_LINK ON)   # force dynamic linking for all libraries


add_executable(DataClinet 
               src/tcp_logger_main.cpp
               src/tcp_logger.cpp
               src/RealSenseInterface.cpp
  )

target_link_libraries(DataClinet
                      ${ZLIB_LIBRARY}
                      #${Boost_SYSTEM_LIBRARIES}
#                      ${Boost_THREAD_LIBRARIES}
#                      ${Boost_FILESYSTEM_LIBRARIES}
                      #${OPENNI_LIBRARY}
                      ${LIBREALSENSE_LIBRARY}
                      ${OpenCV_LIBS} 
                      ${CMAKE_THREAD_LIBS_INIT}
                      ${libusb-1.0_LIBRARIES})

