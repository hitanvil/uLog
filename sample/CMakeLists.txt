cmake_minimum_required(VERSION 3.10)
project(sampleApplication CXX)

# Compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror=format -O3")

set(CMAKE_BUILD_TYPE Debug)
#set(CMAKE_BUILD_TYPE Release)

# Include uLog headers
include_directories(../src)

# Source files
set(CLIENT_SOURCES ulog_client.cpp ../src/uLog.cpp)

set(SERVER_SOURCES ulog_server.cpp ../src/uLog.cpp)


# Add executable
add_executable(ulog_client ${CLIENT_SOURCES})

add_executable(ulog_server ${SERVER_SOURCES})


