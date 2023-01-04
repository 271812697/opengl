cmake_minimum_required (VERSION 2.4)
project("opengl")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#设置文件夹分类
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#收集编译的文件
file(GLOB SHADER *.glsl)
file(GLOB ASSET_SRC ${PROJECT_SOURCE_DIR}/asset/*.*)
file(GLOB COMPONENT_SRC ${PROJECT_SOURCE_DIR}/component/*.*)
file(GLOB CORE_SRC ${PROJECT_SOURCE_DIR}/core/*.*)
file(GLOB EXAMPLE_SRC ${PROJECT_SOURCE_DIR}/example/*.*)
file(GLOB SCENE_SRC ${PROJECT_SOURCE_DIR}/scene/*.*)
file(GLOB UTIL_SRC ${PROJECT_SOURCE_DIR}/util/*.*)
file(GLOB src_files  ${PROJECT_SOURCE_DIR}/*.h ${PROJECT_SOURCE_DIR}/*.cpp ${PROJECT_SOURCE_DIR}/*.c)
#整理目录
source_group(asset FILES ${ASSET_SRC})
source_group(component FILES ${COMPONENT_SRC})
source_group(core FILES ${CORE_SRC})
source_group(util FILES ${UTIL_SRC})
source_group(example FILES ${EXAMPLE_SRC})
source_group(scene FILES ${SCENE_SRC})
source_group(src FILES ${src_files})
source_group(shader FILES ${SHADER})
add_executable(Opengl ${ASSET_SRC} ${COMPONENT_SRC} ${CORE_SRC} ${EXAMPLE_SRC} ${SCENE_SRC} ${UTIL_SRC} ${src_files} ${SHADER})
target_link_libraries(Opengl assimp)
target_link_libraries(Opengl Engine)
add_subdirectory("Engine")
#
# Assimp
#
add_subdirectory("assimp")
set(ASSIMP_BUILD_ZLIB ON)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
#分类
set_target_properties(assimp PROPERTIES FOLDER "Assimp")
set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER "Assimp")
set_target_properties(zlibstatic PROPERTIES FOLDER "Assimp")
set_target_properties(assimp_cmd PROPERTIES FOLDER "Assimp")
#设置工作目录
set_property(TARGET Opengl PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
include_directories(
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/includes
)
target_link_directories(Opengl PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/lib
)
#设置使用unicode字符集
add_definitions(-DUNICODE -D_UNICODE)
#链接输入
target_link_libraries(Opengl
opengl32.lib
gdi32.lib
winmm.lib
user32.lib
gdiplus.lib
glfw3.lib
ImGui.lib
ImGuizmo.lib
)
#拷贝文件到工作目录
#foreach(list ${SHADER})
#file(COPY ${list} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#endforeach(list ${SHADER})
#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assimp-vc142-mtd.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
#file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

