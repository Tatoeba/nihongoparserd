cmake_minimum_required(VERSION 2.6)

set (PROJECT_NAME nihongoparserd)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11" CACHE STRING "" FORCE)

project(${PROJECT_NAME})

file(
    GLOB
    source_files
    src/*.cpp
)

add_executable(
    ${PROJECT_NAME}
    ${source_files}
)

target_link_libraries(
    ${PROJECT_NAME}
    stdc++
    expat
    event
    mecab
)
