cmake_minimum_required(VERSION 3.10)

project(gear_display_facade LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gear_display_facade
    main.cpp
    src/modules/gear_display_facade.cc)

target_include_directories(gear_display_facade PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/modules
    ${CMAKE_CURRENT_SOURCE_DIR}/src/signals
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framework
)