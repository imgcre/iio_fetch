cmake_minimum_required(VERSION 3.12)
project(iio_fetch)

set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -pthread)

include_directories(.)
add_executable(iio_fetch main.cpp dev_context.cxx dev_channel.cxx safe_queue.cxx)

target_link_libraries(iio_fetch /usr/lib/libiio.so.0)