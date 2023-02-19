cmake_minimum_required(VERSION 3.11)

project(postgres CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(book_manager ${SRCS})
target_include_directories(book_manager PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(book_manager CONAN_PKG::libpqxx CONAN_PKG::boost)
