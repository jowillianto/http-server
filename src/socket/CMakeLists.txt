cmake_minimum_required(VERSION 3.28)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)

file (
  GLOB socket_module_interface
  "${CMAKE_CURRENT_LIST_DIR}/*.ccm"
)
file (
  GLOB socket_module_impl
  "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)
add_library(server_socket)
target_sources(server_socket
  PUBLIC
    FILE_SET CXX_MODULES FILES
      ${socket_module_interface} 
  PUBLIC
    ${socket_module_impl}
)