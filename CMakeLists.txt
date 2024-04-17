cmake_minimum_required(VERSION 3.28)
project(http_socket)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)
# add_compile_options("-Werror")

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src)

add_executable(http_main EXCLUDE_FROM_ALL ${CMAKE_CURRENT_LIST_DIR}/main.cpp)
target_link_libraries(http_main 
  server_server
)
target_compile_options(http_main PRIVATE "-Ofast")