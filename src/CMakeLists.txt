cmake_minimum_required(VERSION 3.28)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)

file (
  GLOB server_module_interface
  "${CMAKE_CURRENT_LIST_DIR}/*.ccm"
)
file (
  GLOB server_module_impl
  "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)
add_library(server_server)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/multithreading)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/socket)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/http)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/timer)
target_sources(server_server
  PUBLIC
    FILE_SET CXX_MODULES FILES
      ${server_module_interface} 
  PUBLIC
    ${server_module_impl}
)
target_link_libraries(server_server 
PUBLIC 
  server_multithreading server_socket http_http server_timer
)