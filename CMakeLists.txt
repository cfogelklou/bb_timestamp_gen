cmake_minimum_required(VERSION 3.5)
project(bb_timestamp_gen)

set(CMAKE_CXX_STANDARD 11)

if (WIN32)
elseif (APPLE)
    set(CMAKE_BUILD_TYPE Debug)
elseif (UNIX)
    set(CMAKE_BUILD_TYPE Debug)
endif()

file(GLOB LOCAL_SRC
    ${CMAKE_CURRENT_SOURCE_DIR}/*.c
    ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h
    ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
)

set(SOURCE_FILES
    # SOP src
    ${LOCAL_SRC}
    ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt

)

list(REMOVE_DUPLICATES SOURCE_FILES)

include_directories(
    ./
    ../
)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

