set(TARGET_NAME Editor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(LibFolder ${PROJECT_SOURCE_DIR}/Extern)

# ---- Add source files ----
file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cpp")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})
# ---- Create library ----
add_executable(${TARGET_NAME} ${HEADER_FILES} ${SOURCE_FILES})
set_property(TARGET ${TARGET_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}")
target_include_directories(${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Editor")


# Link dependencies    
target_link_libraries(${TARGET_NAME} PUBLIC opengl32.lib)
target_link_libraries(${TARGET_NAME} PUBLIC core)
target_link_libraries(${TARGET_NAME} PUBLIC opengl)

file(GLOB IRRKDLL ${LibFolder}/irrklang/bin/*.dll)
foreach(list ${IRRKDLL})
file(COPY ${list} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/Debug)
endforeach(list ${IRRKDLL})







