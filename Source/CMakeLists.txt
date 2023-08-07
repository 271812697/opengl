set(TARGET_NAME Scene)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Create library ----
add_library(${TARGET_NAME} STATIC GameApp.cpp GameApp.h GameTimer.cpp GameTimer.h openglApp.cpp openglApp.h all.h)



set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "window")


# Link dependencies    
target_link_libraries(${TARGET_NAME} PUBLIC glfw)
target_link_libraries(${TARGET_NAME} PUBLIC imgui)
target_link_libraries(${TARGET_NAME} PUBLIC opengl32.lib)
target_link_libraries(${TARGET_NAME} PUBLIC opengl)



add_subdirectory(scene01)
add_subdirectory(scene02)
add_subdirectory(scene03)
add_subdirectory(scene04)
add_subdirectory(scene05)
add_subdirectory(scene06)
add_subdirectory(scene07)
add_subdirectory(scene08)
add_subdirectory(PathTrace)
add_subdirectory(Editor)



