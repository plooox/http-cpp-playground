cmake_minimum_required(VERSION 3.26)
project(libevent_test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

message(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/libs/libevent/include)
link_directories(${CMAKE_SOURCE_DIR}/libs/libevent/lib)

# Simple Timer
add_executable(SimpleTimer SimpleTimer.cpp)
target_link_libraries(SimpleTimer event)