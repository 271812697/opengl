cmake_minimum_required(VERSION 3.22)
project("DX11")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#设置文件夹分类
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#收集编译文件
file(GLOB SRC ${PROJECT_SOURCE_DIR}/SRC/*.cpp)
file(GLOB HEAD ${PROJECT_SOURCE_DIR}/SRC/*.h)
file(GLOB IMGUI ${PROJECT_SOURCE_DIR}/ImGui/*.*)
file(GLOB HLSL ${PROJECT_SOURCE_DIR}/HLSL/*.*)

#整理目录
source_group(imgui FILES ${IMGUI})
source_group(shader FILES ${HLSL})
add_executable(DirectX11 WIN32 ${SRC} ${HEAD} ${IMGUI} ${HLSL})

#设置工作目录
set_property(TARGET DirectX11 PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")


#设置使用unicode字符集
add_definitions(-DUNICODE -D_UNICODE -DUSE_IMGUI)
#设置包含目录
target_include_directories(DirectX11 PUBLIC ${PROJECT_SOURCE_DIR}/ImGui)
#链接输入
target_link_libraries(DirectX11
d3d11.lib
dxgi.lib
dxguid.lib
D3DCompiler.lib
d2d1.lib
dwrite.lib
winmm.lib
kernel32.lib
user32.lib
gdi32.lib
winspool.lib
shell32.lib
ole32.lib
oleaut32.lib
uuid.lib
comdlg32.lib
advapi32.lib
)
