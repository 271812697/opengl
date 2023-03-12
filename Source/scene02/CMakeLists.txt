set(TARGET_NAME Scene02)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Create library ----

file(GLOB SCENE_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB SCENE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(${TARGET_NAME} ${SCENE_HEAD} ${SCENE_SRC})
# Link dependencies    
target_link_libraries(${TARGET_NAME} PUBLIC Window)

set_property(TARGET ${TARGET_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")


set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "scene")










