cmake_minimum_required(VERSION 2.8)

project(ddui C CXX)

list(APPEND ddui_SOURCES
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/gl3w/src/gl3w.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/nanovg/src/nanovg.c 
)

add_subdirectory(src)
add_subdirectory(lib/glfw)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/nanovg/src
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/include
  ${CMAKE_CURRENT_SOURCE_DIR}/lib/gl3w/include
)

add_library(ddui ${ddui_SOURCES})
