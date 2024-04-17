cmake_minimum_required(VERSION 3.28)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)

find_package(CURL REQUIRED)

file (
  GLOB http_module_interface
  "${CMAKE_CURRENT_LIST_DIR}/*.ccm"
)
file (
  GLOB http_module_impl
  "${CMAKE_CURRENT_LIST_DIR}/*.cpp"
)
add_library(http_http)
target_sources(http_http
  PUBLIC
    FILE_SET CXX_MODULES FILES
      ${http_module_interface} 
  PUBLIC
    ${http_module_impl}
)
target_link_libraries(http_http PUBLIC CURL::libcurl)