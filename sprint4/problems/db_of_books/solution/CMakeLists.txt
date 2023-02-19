cmake_minimum_required(VERSION 3.11)

project(postgres CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo_multi.cmake)
conan_basic_setup(TARGETS)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

file(GLOB_RECURSE SRCS CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(db_of_books ${SRCS})
target_include_directories(db_of_books PRIVATE ${CMAKE_SOURCE_DIR}/src)
target_link_libraries(db_of_books CONAN_PKG::libpqxx CONAN_PKG::boost)
