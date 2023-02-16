cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

find_package(Boost 1.78.0 REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# model library
file(GLOB_RECURSE MODEL_SRCS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/model/*.cpp)
add_library(game_model STATIC ${MODEL_SRCS})
target_include_directories(game_model PUBLIC ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(game_model PUBLIC Threads::Threads CONAN_PKG::boost)

# executable target
file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)
list(REMOVE_ITEM SRCS ${MODEL_SRCS})
add_executable(game_server ${SRCS})
target_include_directories(game_server PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(game_server PRIVATE game_model)

include(CTest)
include(${CONAN_BUILD_DIRS_CATCH2}/Catch.cmake)

# tests target
file(GLOB_RECURSE TEST_SRCS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/tests/*.cpp)
add_executable(game_server_tests ${TEST_SRCS})
target_link_libraries(game_server_tests PRIVATE CONAN_PKG::catch2 game_model)
catch_discover_tests(game_server_tests)
