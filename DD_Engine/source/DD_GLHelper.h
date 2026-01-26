#pragma once

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#ifdef _WIN32
#include <GL/glew.h>
#endif
#endif

#include <GLFW/glfw3.h>

// Enable experimental extensions used by GLM (gtx)
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/component_wise.hpp>

#include <vector>
#include <memory>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Matrix4 = glm::mat4;
using Color = glm::vec4;
using Quaternion = glm::quat;

//==============================================================================
// Platform-specific GL compatibility
//==============================================================================

// GL_CLAMP_TO_BORDER is not available in WebGL/GLES
#ifdef __EMSCRIPTEN__
    #ifndef GL_CLAMP_TO_BORDER
        #define GL_CLAMP_TO_BORDER GL_CLAMP_TO_EDGE
    #endif
    #ifndef GL_TEXTURE_BORDER_COLOR
        #define GL_TEXTURE_BORDER_COLOR 0x1004
    #endif
#endif

// Depth texture formats
#ifdef __EMSCRIPTEN__
    #define DD_DEPTH_COMPONENT GL_DEPTH_COMPONENT24
    #define DD_DEPTH_INTERNAL_FORMAT GL_DEPTH_COMPONENT24
#else
    #define DD_DEPTH_COMPONENT GL_DEPTH_COMPONENT
    #define DD_DEPTH_INTERNAL_FORMAT GL_DEPTH_COMPONENT24
#endif

// Shadow sampler - WebGL2 supports sampler2DShadow but needs setup
#ifdef __EMSCRIPTEN__
    #define DD_SHADOW_COMPARE_SUPPORTED 1
#else
    #define DD_SHADOW_COMPARE_SUPPORTED 1
#endif

//==============================================================================
// GL Helper utilities
//==============================================================================

namespace GLHelper
{
    // Configure texture for shadow mapping (platform-safe)
    inline void ConfigureShadowTexture()
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        // Use CLAMP_TO_EDGE for WebGL compatibility
        // On Desktop, CLAMP_TO_BORDER with white border is ideal but CLAMP_TO_EDGE works
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
#ifndef __EMSCRIPTEN__
        // Desktop-only: comparison mode for sampler2DShadow
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
#endif
    }

    // Configure regular depth texture (no comparison)
    inline void ConfigureDepthTexture()
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }

    // Check if extension is available
    inline bool HasExtension(const char* name)
    {
#ifdef __EMSCRIPTEN__
        // WebGL extension check
        return false; // Simplified - can be expanded
#else
        return glewIsSupported(name) == GL_TRUE;
#endif
    }
}
