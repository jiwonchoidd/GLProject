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
    # Emscripten (Web) build - Executable
    add_executable(DD_Sample ${SAMPLE_SOURCES})
    
    target_compile_definitions(DD_Sample PUBLIC __EMSCRIPTEN__)
    
    # Link to DD_Engine
    target_link_libraries(DD_Sample PRIVATE DD_Engine)

    # Ensure GLFW port is enabled for Emscripten so glfw* symbols are available
    # Use linker flags since CMake version may not support target_link_options
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3")
    
    # Set output to HTML
    set_target_properties(DD_Sample PROPERTIES SUFFIX ".html")
    
else()
    # Desktop build - Executable
    add_executable(DD_Sample ${SAMPLE_SOURCES})
    
    # Include DD_Engine headers and dependencies
    target_include_directories(DD_Sample PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}/../DD_Engine
        ${CMAKE_CURRENT_SOURCE_DIR}/../DD_Engine/packages/glfw.3.4.0/build/native/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../DD_Engine/packages/glew-2.2.0.2.2.0.1/build/native/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../DD_Engine/packages/glm.1.0.3/build/native/include
    )
    
    # Link to DD_Engine
    target_link_libraries(DD_Sample PRIVATE DD_Engine)
    
    # Copy DLL to output directory
    add_custom_command(TARGET DD_Sample POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        $<TARGET_FILE:DD_Engine>
        $<TARGET_FILE_DIR:DD_Sample>
    )
endif()
