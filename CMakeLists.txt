cmake_minimum_required(VERSION 3.19 FATAL_ERROR)
project(Opengl VERSION 0.1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS OFF)

set(THIRD_PARTY_DIR ${PROJECT_SOURCE_DIR}/Extern)

add_subdirectory(DX11)
add_subdirectory(Source)
add_subdirectory(Extern)
add_subdirectory(PolygonDemo)
