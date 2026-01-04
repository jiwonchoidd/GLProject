cmake_minimum_required(VERSION 3.10)
project(DD_Engine)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
set(ENGINE_SOURCES
    ../DD_Engine/DD_Core.cpp
    ../DD_Engine/DD_WinApplication.cpp
    ../DD_Engine/DD_GLDevice.cpp
    ../DD_Engine/DD_SimpleBox.cpp
)

set(ENGINE_HEADERS
    ../DD_Engine/DD_Core.h
    ../DD_Engine/DD_WinApplication.h
    ../DD_Engine/DD_GLDevice.h
    ../DD_Engine/DD_SimpleBox.h
    ../DD_Engine/DD_GLHelper.h
    ../DD_Engine/framework.h
)

set(SAMPLE_SOURCES
    DD_Sample.cpp
)

# Platform-specific settings
if(EMSCRIPTEN)
    # Emscripten (Web) build
    add_executable(DD_Sample ${ENGINE_SOURCES} ${ENGINE_HEADERS} ${SAMPLE_SOURCES})
    
    set_target_properties(DD_Sample PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "-s USE_GLFW=3 -s USE_WEBGL2=1 -s FULL_ES3=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 --shell-file ${CMAKE_SOURCE_DIR}/shell.html"
    )
    
    target_compile_definitions(DD_Sample PRIVATE __EMSCRIPTEN__)
    
else()
    # Desktop build
    find_package(OpenGL REQUIRED)
    
    # GLFW and GLEW from NuGet packages
    set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/packages/glfw.3.4.0/build/native/include")
    set(GLEW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/packages/glew-2.2.0.2.2.0.1/build/native/include")
    set(GLM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/packages/glm.1.0.3/build/native/include")
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(GLFW_LIBRARY "${CMAKE_SOURCE_DIR}/packages/glfw.3.4.0/build/native/lib/x64/v143/glfw3.lib")
        set(GLEW_LIBRARY "${CMAKE_SOURCE_DIR}/packages/glew-2.2.0.2.2.0.1/build/native/lib/x64/v143/glew32.lib")
    else()
        set(GLFW_LIBRARY "${CMAKE_SOURCE_DIR}/packages/glfw.3.4.0/build/native/lib/Win32/v143/glfw3.lib")
        set(GLEW_LIBRARY "${CMAKE_SOURCE_DIR}/packages/glew-2.2.0.2.2.0.1/build/native/lib/Win32/v143/glew32.lib")
    endif()
    
    add_executable(DD_Sample WIN32 ${ENGINE_SOURCES} ${ENGINE_HEADERS} ${SAMPLE_SOURCES})
    
    target_include_directories(DD_Sample PRIVATE 
        ${GLFW_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
    )
    
    target_link_libraries(DD_Sample 
        ${GLFW_LIBRARY}
        ${GLEW_LIBRARY}
        ${OPENGL_gl_LIBRARY}
    )
    
    target_compile_definitions(DD_Sample PRIVATE DD_ENGINE_EXPORTS)
endif()

# Include directories
target_include_directories(DD_Sample PRIVATE ../DD_Engine)
