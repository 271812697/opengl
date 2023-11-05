set(TARGET_NAME PathTrace)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Create library ----

file(GLOB_RECURSE Path_HEAD ${CMAKE_CURRENT_SOURCE_DIR}/*.h ${CMAKE_CURRENT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE Path_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${Path_SRC} ${Path_HEAD})
add_executable(${TARGET_NAME} ${Path_HEAD} ${Path_SRC})


#set_property(TARGET ${TARGET_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")

set(OIDN_LIBRARIES "OpenImageDenoise")
target_link_directories(${TARGET_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}/oidn/lib
)


set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "PathTrace")

# Link dependencies    
target_link_libraries(${TARGET_NAME} PUBLIC ${OIDN_LIBRARIES})
target_link_libraries(${TARGET_NAME} PUBLIC window)
target_link_libraries(${TARGET_NAME} PUBLIC opengl)
target_link_libraries(${TARGET_NAME} PUBLIC tinygltf)
target_link_libraries(${TARGET_NAME} PUBLIC imgui)
target_link_libraries(${TARGET_NAME} PUBLIC glad)
target_link_libraries(${TARGET_NAME} PUBLIC ui)
target_link_libraries(${TARGET_NAME} PUBLIC tools)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/oidn/lib/OpenImageDenoise.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/oidn/lib/tbb.dll DESTINATION ${CMAKE_CURRENT_BINARY_DIR})











