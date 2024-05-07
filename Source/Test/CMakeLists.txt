set(TARGET_NAME Test)
set(CMAKE_CXX_STANDARD "/std:c++latest")
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PUBLIC_HDR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/filament/include)
set(GENERATION_ROOT ${CMAKE_CURRENT_BINARY_DIR})
add_definitions(-DFILAMENT_SUPPORTS_OPENGL)
add_definitions(-DFILAMENT_DRIVER_SUPPORTS_VULKAN)

# ---- Create library ----

file(GLOB_RECURSE Test_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE Test_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${Test_SRC} ${Test_HEAD} )
add_executable(${TARGET_NAME} ${Test_HEAD} ${Test_SRC} )
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filament)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filament/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filament/backend/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filament/filabridge/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filament/backend/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filament/filamat/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filament/filaflat/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filament/smol-v)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filament/bluegl/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filament/bluevk/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/filament/ibl/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../glslang)
target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${PUBLIC_HDR_DIR}>)


# specify where the public headers of this library are
#target_include_directories(${TARGET} ${PUBLIC_HDR_DIR})

set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Test")

# Link dependencies    
target_link_libraries(${TARGET_NAME} PUBLIC window)
target_link_libraries(${TARGET_NAME} PUBLIC opengl)
target_link_libraries(${TARGET_NAME} PUBLIC imgui)
target_link_libraries(${TARGET_NAME} PUBLIC glad)
target_link_libraries(${TARGET_NAME} PUBLIC ui)
target_link_libraries(${TARGET_NAME} PUBLIC tools)
target_link_libraries(${TARGET_NAME} PUBLIC glslang)
target_link_libraries(${TARGET_NAME} PUBLIC SPIRV)
target_link_libraries(${TARGET_NAME} PUBLIC SPIRV-Tools)
target_link_libraries(${TARGET_NAME} PUBLIC SPIRV-Tools-opt)
target_link_libraries(${TARGET_NAME} PUBLIC spirv-cross-core)
target_link_libraries(${TARGET_NAME} PUBLIC spirv-cross-glsl)
target_link_libraries(${TARGET_NAME} PUBLIC spirv-cross-msl)
target_link_libraries(${TARGET_NAME} PUBLIC opengl32 gdi32 Shlwapi)














