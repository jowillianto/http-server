cmake_minimum_required(VERSION 3.28)
project(moderna_http)

set (CMAKE_CXX_STANDARD 23)
set (CMAKE_CXX_STANDARD_REQUIRED true)
set (CMAKE_CXX_SCAN_FOR_MODULES true)

if (NOT TARGET ${PROJECT_NAME})
  find_package(CURL REQUIRED)
  file (
    GLOB ${PROJECT_NAME}_src
    "${CMAKE_CURRENT_LIST_DIR}/src/*/*.ccm"
  )
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/variant_error)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/io io)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/libs/reflect-cpp)
  add_library (${PROJECT_NAME})
  target_sources(${PROJECT_NAME}
    PUBLIC
      FILE_SET CXX_MODULES FILES
        ${${PROJECT_NAME}_src}
  )
  target_include_directories(moderna_http PRIVATE ${CURL_INCLUDE_DIRS})
  target_link_libraries(moderna_http 
    PUBLIC 
      ${CURL_LIBRARIES} 
      moderna_variant_error
      moderna_io
      moderna_thread_plus
      reflectcpp
  )
endif()

if (EXISTS "${CMAKE_CURRENT_LIST_DIR}/../test-lib/CMakeLists.txt" AND PROJECT_IS_TOP_LEVEL)
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../test-lib test-lib)
  include (CTest)
  moderna_add_test(http_tests 
    ${CMAKE_CURRENT_LIST_DIR}/tests/http_tests.cpp
    LIBRARIES ${PROJECT_NAME} moderna_test_lib
    SANITIZERS all
  )
elseif (PROJECT_IS_TOP_LEVEL)
  message ( "Clone Moderna Library to Test")
endif()