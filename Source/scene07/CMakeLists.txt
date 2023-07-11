set(TARGET_NAME Scene07)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Create library ----

file(GLOB SCENE_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB SCENE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(${TARGET_NAME} ${SCENE_HEAD} ${SCENE_SRC})

# Link dependencies    
target_link_libraries(${TARGET_NAME} PUBLIC window)
target_link_libraries(${TARGET_NAME} PUBLIC opengl)
target_link_libraries(${TARGET_NAME} PUBLIC imgui)
target_link_libraries(${TARGET_NAME} PUBLIC glad)
target_link_libraries(${TARGET_NAME} PUBLIC ui)
target_link_libraries(${TARGET_NAME} PUBLIC tools)

set_property(TARGET ${TARGET_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")


set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "scene")










