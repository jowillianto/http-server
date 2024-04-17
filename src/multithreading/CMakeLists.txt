cmake_minimum_required(VERSION 3.28)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)

file (
  GLOB multithreading_module_interface
  "${CMAKE_CURRENT_LIST_DIR}/*.ccm"
)
file (
  GLOB multithreading_module_impl
  "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)
add_library(server_multithreading)
target_sources(server_multithreading
  PUBLIC
    FILE_SET CXX_MODULES FILES
      ${multithreading_module_interface} 
  PUBLIC
    ${multithreading_module_impl}
)