cmake_minimum_required(VERSION 2.6)

project(ChessV3)

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
   message(FATAL_ERROR "ERROR: Building in source, please build out of it")
endif()

set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_CXX_FLAGS_DEBUG "-Wall -march=native -std=c++0x  -g -O0 -Wshadow  -lpthread -pedantic -Winvalid-pch -include precomp.h")

set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake;${CMAKE_MODULE_PATH}")	

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/arch)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
   

set(Boost_ADDITIONAL_VERSIONS "1.42" "1.42.0" "1.41.0" "1.41" "1.40.0" "1.40" "1.39.0" "1.39" "1.38.0" "1.38" "1.37.0" "1.37" )
set(BOOST_COMPONENTS system thread date_time)


find_package(Boost COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

find_package(SFML REQUIRED)
include_directories(${SFML_INCLUDE_DIR})

find_package(GTK2 REQUIRED gtk gtkmm) 
include_directories(${GTK2_INCLUDE_DIRS} /usr/include/freetype2)


include_directories(shared)
include_directories(${CMAKE_SOURCE_DIR})

include(PCH_GCC4_v3)
ADD_PCH_RULE(precomp.h HEADER)

add_executable(Precomping ${HEADER} test.cpp)
#target_link_libraries(Precomping ${GTK2_LIBRARIES} ${Boost_LIBRARIES})
#target_link_libraries(Precomping ${Boost_LIBRARIES})

add_subdirectory(client) 
add_subdirectory(server) 
add_subdirectory(shared) 
