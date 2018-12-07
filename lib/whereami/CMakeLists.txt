cmake_minimum_required(VERSION 2.8)
project(whereami C)
add_library(whereami
    ${CMAKE_CURRENT_SOURCE_DIR}/whereami.h
    ${CMAKE_CURRENT_SOURCE_DIR}/whereami.c
)
target_include_directories(whereami PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
