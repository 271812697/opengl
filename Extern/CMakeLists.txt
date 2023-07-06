set(lib_folder "Extern")
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

if(NOT TARGET imgui)
    set(imgui_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/imgui)
    file(GLOB imgui_sources CONFIGURE_DEPENDS  "${imgui_SOURCE_DIR_}/*.cpp")
    file(GLOB imgui_impl CONFIGURE_DEPENDS  
    "${imgui_SOURCE_DIR_}/backends/imgui_impl_glfw.cpp" 
    "${imgui_SOURCE_DIR_}/backends/imgui_impl_glfw.h"
    "${imgui_SOURCE_DIR_}/IconsForkAwesome.h"
    "${imgui_SOURCE_DIR_}/backends/imgui_impl_opengl3.cpp" 
    "${imgui_SOURCE_DIR_}/backends/imgui_impl_opengl3.h"
    "${imgui_SOURCE_DIR_}/backends/imgui_impl_opengl3_loader.h")
    add_library(imgui STATIC ${imgui_sources} ${imgui_impl})
    target_link_libraries(imgui PUBLIC glfw)
    target_include_directories(imgui PUBLIC $<BUILD_INTERFACE:${imgui_SOURCE_DIR_}>)
    set_target_properties(imgui PROPERTIES FOLDER ${lib_folder}/imgui)
endif()
if(NOT TARGET glfw)
    option(GLFW_BUILD_EXAMPLES "" OFF)
    option(GLFW_BUILD_TESTS "" OFF)
    option(GLFW_BUILD_DOCS "" OFF)
    option(GLFW_INSTALL "" OFF)
    add_subdirectory(glfw)
    set_target_properties(glfw PROPERTIES FOLDER ${lib_folder}/glfw)
    set_target_properties(update_mappings PROPERTIES FOLDER ${lib_folder}/glfw)
endif()
if(NOT TARGET glad)
    set(glad_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/glad)
    file(GLOB_RECURSE glad_impl CONFIGURE_DEPENDS  
    "${glad_SOURCE_DIR_}/*.c")
    file(GLOB_RECURSE glad_head CONFIGURE_DEPENDS  
    "${glad_SOURCE_DIR_}/*.h")
    add_library(glad STATIC ${glad_impl} ${glad_head})
    target_include_directories(glad PUBLIC $<BUILD_INTERFACE:${glad_SOURCE_DIR_}>)
    set_target_properties(glad PROPERTIES FOLDER ${lib_folder}/glad)
endif()




if(NOT TARGET std_image)
    set(std_image_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/stb_Image)
    file(GLOB_RECURSE std_image_impl CONFIGURE_DEPENDS  
    "${std_image_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE std_image_head CONFIGURE_DEPENDS  
    "${std_image_SOURCE_DIR_}/*.h")
    source_group(TREE "${std_image_SOURCE_DIR_}" FILES ${std_image_head} ${std_image_impl})
    add_library(std_image STATIC ${std_image_head} ${std_image_impl})
    set_target_properties(std_image PROPERTIES LINKER_LANGUAGE CXX)
    target_include_directories(std_image PUBLIC $<BUILD_INTERFACE:${std_image_SOURCE_DIR_}/include>)
    set_target_properties(std_image PROPERTIES FOLDER ${lib_folder}/std_image)
endif()
if(NOT TARGET spdlog)
    option(SPDLOG_BUILD_EXAMPLE "" OFF)
    option(SPDLOG_INSTALL "" OFF)
    add_subdirectory(spdlog)
    set_target_properties(spdlog PROPERTIES FOLDER ${lib_folder}/spdlog)
endif()

if(NOT TARGET glm_static)
    add_subdirectory(glm)
    set_target_properties(glm_static PROPERTIES FOLDER ${lib_folder}/glm_static)
endif()

if(NOT TARGET assimp)
    set(ASSIMP_BUILD_ZLIB ON)
    set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
    set(ASSIMP_BUILD_TESTS OFF)
    set(ASSIMP_INSTALL OFF)
    set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
    add_subdirectory(assimp)
    set_target_properties(assimp PROPERTIES FOLDER ${lib_folder}/Assimp)
    set_target_properties(UpdateAssimpLibsDebugSymbolsAndDLLs PROPERTIES FOLDER ${lib_folder}/Assimp)
    set_target_properties(zlibstatic PROPERTIES FOLDER ${lib_folder}/Assimp)
    #set_target_properties(assimp_cmd PROPERTIES FOLDER ${lib_folder}/Assimp)

endif()

if(NOT TARGET opengl)   
    #设置使用unicode字符集
    add_definitions(-DUNICODE -D_UNICODE)
    set(opengl_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/Opengl)
    file(GLOB_RECURSE opengl_impl CONFIGURE_DEPENDS  
    "${opengl_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE opengl_head CONFIGURE_DEPENDS  
    "${opengl_SOURCE_DIR_}/*.h")
    source_group(TREE "${opengl_SOURCE_DIR_}" FILES ${opengl_impl} ${opengl_head})
    add_library(opengl STATIC ${opengl_impl} ${opengl_head})

    target_link_libraries(opengl PUBLIC glfw)
    target_link_libraries(opengl PUBLIC imgui)
    target_link_libraries(opengl PUBLIC glad)
    target_link_libraries(opengl PUBLIC spdlog)
    target_link_libraries(opengl PUBLIC std_image)
    target_link_libraries(opengl PUBLIC glm_static)
    target_link_libraries(opengl PUBLIC assimp)
    target_include_directories(opengl PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)
    set_target_properties(opengl PROPERTIES FOLDER ${lib_folder}/opengl)
endif()

if(NOT TARGET tools)   
    set(tools_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/tools)
    file(GLOB_RECURSE tools_impl CONFIGURE_DEPENDS  
    "${tools_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE tools_head CONFIGURE_DEPENDS  
    "${tools_SOURCE_DIR_}/*.h")
    source_group(TREE "${tools_SOURCE_DIR_}" FILES ${tools_impl} ${tools_head})
    add_library(tools STATIC ${tools_impl} ${tools_head})
    set_target_properties(tools PROPERTIES FOLDER ${lib_folder}/tools)
endif()

if(NOT TARGET math)
    set(math_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/Maths)
    file(GLOB_RECURSE math_impl CONFIGURE_DEPENDS  
    "${math_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE math_head CONFIGURE_DEPENDS  
    "${math_SOURCE_DIR_}/*.h")
    source_group(TREE "${math_SOURCE_DIR_}" FILES ${math_impl} ${math_head})
    add_library(math STATIC ${math_impl} ${math_head})
    target_include_directories(math PUBLIC $<BUILD_INTERFACE:${math_SOURCE_DIR_}/include>)
    set_target_properties(math PROPERTIES FOLDER ${lib_folder}/math)
endif()

if(NOT TARGET window)
    set(window_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/Window)
    file(GLOB_RECURSE window_impl CONFIGURE_DEPENDS  
    "${window_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE window_head CONFIGURE_DEPENDS  
    "${window_SOURCE_DIR_}/*.h")
    source_group(TREE "${window_SOURCE_DIR_}" FILES ${window_impl} ${window_head})
    add_library(window STATIC ${window_impl} ${window_head})
    target_link_libraries(window PUBLIC glfw)
    target_link_libraries(window PUBLIC tools)
    target_link_libraries(window PUBLIC std_image)
    set_target_properties(window PROPERTIES FOLDER ${lib_folder}/window)
endif()

if(NOT TARGET ui)
    set(ui_SOURCE_DIR_ ${CMAKE_CURRENT_SOURCE_DIR}/UI)
    file(GLOB_RECURSE ui_impl CONFIGURE_DEPENDS  
    "${ui_SOURCE_DIR_}/*.cpp")
    file(GLOB_RECURSE ui_head CONFIGURE_DEPENDS  
    "${ui_SOURCE_DIR_}/*.h")
    source_group(TREE "${ui_SOURCE_DIR_}" FILES ${ui_impl} ${ui_head})
    add_library(ui STATIC ${ui_impl} ${ui_head})
    target_link_libraries(ui PUBLIC imgui)
    target_link_libraries(ui PUBLIC glad)
    target_link_libraries(ui PUBLIC math)
    target_link_libraries(ui PUBLIC tools)
    target_include_directories(ui PUBLIC $<BUILD_INTERFACE:${ui_SOURCE_DIR_}/include>)
    set_target_properties(ui PROPERTIES FOLDER ${lib_folder}/ui)
endif()






