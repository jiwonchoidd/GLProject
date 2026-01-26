#include "DD_ShadowRenderer.h"
#include "DD_LightComponent.h"
#include "DD_Actor.h"
#include "DD_MeshComponent.h"
#include "DD_Mesh.h"
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>

GLuint DD_ShadowRenderer::s_depthProgram = 0;
GLint DD_ShadowRenderer::s_depthModelLoc = -1;
GLint DD_ShadowRenderer::s_depthLightSpaceLoc = -1;
bool DD_ShadowRenderer::s_shadersReady = false;

bool DD_ShadowRenderer::CacheShaders()
{
    if (s_shadersReady) return true;

#ifdef __EMSCRIPTEN__
    // WebGL2 / GLSL ES 3.00
    const char* vertexShaderSource =
        "#version 300 es\n"
        "precision highp float;\n"
        "layout(location = 0) in vec3 aPos;\n"
        "uniform mat4 uModel;\n"
        "uniform mat4 uLightSpace;\n"
        "void main() {\n"
        "    gl_Position = uLightSpace * uModel * vec4(aPos, 1.0);\n"
        "}\n";

    const char* fragmentShaderSource =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 fragColor;\n"
        "void main() {\n"
        "    fragColor = vec4(1.0);\n"
        "}\n";
#else
    // Desktop OpenGL 3.3 Core
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        
        uniform mat4 uModel;
        uniform mat4 uLightSpace;
        
        void main()
        {
            gl_Position = uLightSpace * uModel * vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        
        void main()
        {
            // Depth is written automatically
        }
    )";
#endif

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        printf("Shadow vertex shader compilation failed:\n%s\n", infoLog);
        return false;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        printf("Shadow fragment shader compilation failed:\n%s\n", infoLog);
        glDeleteShader(vertexShader);
        return false;
    }

    s_depthProgram = glCreateProgram();
    glAttachShader(s_depthProgram, vertexShader);
    glAttachShader(s_depthProgram, fragmentShader);

#ifdef __EMSCRIPTEN__
    glBindAttribLocation(s_depthProgram, 0, "aPos");
#endif

    glLinkProgram(s_depthProgram);

    glGetProgramiv(s_depthProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(s_depthProgram, 512, nullptr, infoLog);
        printf("Shadow program linking failed:\n%s\n", infoLog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    s_depthModelLoc = glGetUniformLocation(s_depthProgram, "uModel");
    s_depthLightSpaceLoc = glGetUniformLocation(s_depthProgram, "uLightSpace");

    printf("Shadow shader program created (model=%d, lightSpace=%d)\n", 
           s_depthModelLoc, s_depthLightSpaceLoc);

    s_shadersReady = true;
    return true;
}

void DD_ShadowRenderer::ClearShaders()
{
    if (s_depthProgram)
    {
        glDeleteProgram(s_depthProgram);
        s_depthProgram = 0;
    }
    s_shadersReady = false;
}

DD_ShadowRenderer::DD_ShadowRenderer()
    : m_shadowTarget(nullptr)
    , m_lightSpaceMatrix(1.0f)
    , m_shadowMapSize(2048)
    , m_initialized(false)
{
}

DD_ShadowRenderer::~DD_ShadowRenderer()
{
    Shutdown();
}

bool DD_ShadowRenderer::Initialize(int shadowMapSize)
{
    if (!CacheShaders())
    {
        return false;
    }

    m_shadowMapSize = shadowMapSize;
    m_shadowTarget = std::make_unique<DD_RenderTarget>();
    
    if (!m_shadowTarget->Create(shadowMapSize, shadowMapSize, RenderTargetType::DepthOnly))
    {
        printf("Failed to create shadow map render target\n");
        return false;
    }

    m_initialized = true;
    printf("ShadowRenderer initialized with %dx%d shadow map\n", shadowMapSize, shadowMapSize);
    return true;
}

void DD_ShadowRenderer::Shutdown()
{
    m_shadowTarget.reset();
    m_initialized = false;
}

void DD_ShadowRenderer::BeginShadowPass(const DD_LightComponent& light)
{
    if (!m_initialized) return;

    m_lightSpaceMatrix = light.GetLightSpaceMatrix();

    m_shadowTarget->Bind();
    glClear(GL_DEPTH_BUFFER_BIT);

    // Use depth shader
    glUseProgram(s_depthProgram);
    glUniformMatrix4fv(s_depthLightSpaceLoc, 1, GL_FALSE, glm::value_ptr(m_lightSpaceMatrix));

    // Cull front faces to reduce shadow acne
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    // Polygon offset for additional bias
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(2.0f, 4.0f);
}

void DD_ShadowRenderer::RenderActor(DD_Actor* actor)
{
    if (!actor) return;

    DD_MeshComponent* meshComp = actor->GetMeshComponent();
    if (!meshComp) return;

    DD_Mesh* mesh = meshComp->GetMesh();
    if (!mesh) return;

    Matrix4 model = actor->GetModelMatrix();
    glUniformMatrix4fv(s_depthModelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Bind mesh buffers and draw
#ifndef __EMSCRIPTEN__
    GLuint vao = mesh->GetVAO();
    if (vao) glBindVertexArray(vao);
#else
    // Manual attribute binding for WebGL
    glBindBuffer(GL_ARRAY_BUFFER, mesh->GetVertexBuffer());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIndexBuffer());
    glEnableVertexAttribArray(0);
    // SimpleVertex has position at offset 0, size 3 floats, stride = sizeof(SimpleVertex) = sizeof(vec3) + sizeof(vec2)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(glm::vec2), (void*)0);
#endif

    glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_SHORT, 0);

#ifndef __EMSCRIPTEN__
    if (vao) glBindVertexArray(0);
#else
    glDisableVertexAttribArray(0);
#endif
}

void DD_ShadowRenderer::EndShadowPass()
{
    if (!m_initialized) return;

    glDisable(GL_POLYGON_OFFSET_FILL);
    
    // Restore normal culling
    glCullFace(GL_BACK);

    m_shadowTarget->Unbind();
}

GLuint DD_ShadowRenderer::GetShadowMap() const
{
    if (m_shadowTarget)
    {
        return m_shadowTarget->GetDepthTexture();
    }
    return 0;
}
