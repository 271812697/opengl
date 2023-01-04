cmake_minimum_required (VERSION 2.4)
set(CMAKE_CXX_STANDARD 20)
#设置使用unicode字符集
add_definitions(-DUNICODE -D_UNICODE)
file(GLOB TEST_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
add_executable(Test ${TEST_SRC})
target_link_libraries(Test Engine)