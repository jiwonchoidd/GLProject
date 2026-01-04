cmake_minimum_required(VERSION 3.10)
project(DD_Sample)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sample source files
set(SAMPLE_SOURCES
    DD_Sample.cpp
)

# Platform-specific settings
if(EMSCRIPTEN)
    # Emscripten (Web) build
    add_executable(DD_Sample ${SAMPLE_SOURCES})
    
    set_target_properties(DD_Sample PROPERTIES
        SUFFIX ".html"
        LINK_FLAGS "-s USE_GLFW=3 -s USE_WEBGL2=1 -s FULL_ES3=1 -s WASM=1 -s ALLOW_MEMORY_GROWTH=1 --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
    )
    
    target_link_libraries(DD_Sample DD_Engine)
    
else()
    # Desktop build
    add_executable(DD_Sample WIN32 ${SAMPLE_SOURCES})
    
    target_link_libraries(DD_Sample DD_Engine)
endif()

target_include_directories(DD_Sample PRIVATE ../DD_Engine)
