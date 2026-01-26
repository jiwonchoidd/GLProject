cmake_minimum_required(VERSION 3.10)
project(DD_Engine)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Engine source files
set(ENGINE_SOURCES
    source/DD_Application.cpp
    source/DD_Camera.cpp
    source/DD_Core.cpp
    source/DD_GLDevice.cpp
    source/DD_SimpleBox.cpp
    source/DD_World.cpp
    source/DD_Mesh.cpp
    source/DD_MeshComponent.cpp
    source/DD_Actor.cpp
    source/DD_CameraController.cpp
    source/DD_CollisionUtils.cpp
    source/DD_DebugDraw.cpp
    source/DD_RenderTarget.cpp
    source/DD_ShadowRenderer.cpp
    source/DD_SceneRenderer.cpp
    source/DD_Component.cpp
    source/DD_LightComponent.cpp
    source/DD_LightActor.cpp
    source/DD_Material.cpp
    source/DD_Texture.cpp
    source/DD_GBuffer.cpp
    source/DD_DeferredRenderer.cpp
)

set(ENGINE_HEADERS
    source/DD_Application.h
    source/DD_Camera.h
    source/DD_Core.h
    source/DD_GLDevice.h
    source/DD_SimpleBox.h
    source/DD_World.h
    source/DD_GLHelper.h
    source/framework.h
    source/DD_Mesh.h
    source/DD_MeshComponent.h
    source/DD_CollisionComponent.h
    source/DD_DebugDraw.h
    source/DD_RenderTarget.h
    source/DD_ShadowRenderer.h
    source/DD_SceneRenderer.h
    source/DD_Component.h
    source/DD_LightComponent.h
    source/DD_LightActor.h
    source/DD_Material.h
    source/DD_Texture.h
    source/DD_GBuffer.h
    source/DD_DeferredRenderer.h
    source/stb_image.h
)

# Platform-specific settings    
if(EMSCRIPTEN)
    # Emscripten (Web) build - Static library
    add_library(DD_Engine STATIC ${ENGINE_SOURCES} ${ENGINE_HEADERS})
    
    target_compile_definitions(DD_Engine PUBLIC __EMSCRIPTEN__)
    
    # Include GLM header-only library
    target_include_directories(DD_Engine PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${CMAKE_CURRENT_SOURCE_DIR}/glm
    )
    
else()
    # Desktop build - Shared library
    find_package(OpenGL REQUIRED)
    
    # GLFW and GLEW from NuGet packages
    set(GLFW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/packages/glfw.3.4.0/build/native/include")
    set(GLEW_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/packages/glew-2.2.0.2.2.0.1/build/native/include")
    set(GLM_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/packages/glm.1.0.3/build/native/include")
    
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(GLFW_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/packages/glfw.3.4.0/build/native/lib/x64/v143/glfw3.lib")
        set(GLEW_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/packages/glew-2.2.0.2.2.0.1/build/native/lib/x64/v143/glew32.lib")
    else()
        set(GLFW_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/packages/glfw.3.4.0/build/native/lib/Win32/v143/glfw3.lib")
        set(GLEW_LIBRARY "${CMAKE_CURRENT_SOURCE_DIR}/packages/glew-2.2.0.2.2.0.1/build/native/lib/Win32/v143/glew32.lib")
    endif()
    
    add_library(DD_Engine SHARED ${ENGINE_SOURCES} ${ENGINE_HEADERS})
    
    target_include_directories(DD_Engine PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/source
        ${GLFW_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
        ${OPENGL_INCLUDE_DIR}
    )
    
    target_link_libraries(DD_Engine PUBLIC
        ${GLFW_LIBRARY}
        ${GLEW_LIBRARY}
        ${OPENGL_gl_LIBRARY}
    )
    
    target_compile_definitions(DD_Engine PRIVATE DD_ENGINE_EXPORTS)
endif()

# Add sample subdirectory
add_subdirectory(../DD_Sample DD_Sample)
