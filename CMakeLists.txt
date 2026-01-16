cmake_minimum_required(VERSION 3.15)
project(PhoenixCorridorEcoHUDServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ECOHUD_BUILD_DEBUG_LOG "Enable extra debug logging" ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

add_executable(ecohud_server
    src/main.cpp
)

target_compile_features(ecohud_server PUBLIC cxx_std_17)

if(ECOHUD_BUILD_DEBUG_LOG)
    target_compile_definitions(ecohud_server PRIVATE ECOHUD_DEBUG_LOG=1)
endif()
