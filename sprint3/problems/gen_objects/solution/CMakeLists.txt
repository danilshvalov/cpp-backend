cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Boost 1.78.0 REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE TEST_SRCS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/*.cpp)

add_executable(game_server ${SRCS})
target_include_directories(game_server PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(game_server PRIVATE Threads::Threads CONAN_PKG::boost)

add_executable(tests ${TEST_SRCS} ${SRCS})
target_include_directories(tests PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(tests PRIVATE Threads::Threads CONAN_PKG::boost CONAN_PKG::catch2)
