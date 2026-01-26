#include "DD_GBuffer.h"
#include <cstdio>

DD_GBuffer::DD_GBuffer()
    : m_fbo(0)
    , m_positionTex(0)
    , m_normalTex(0)
    , m_albedoTex(0)
    , m_materialTex(0)
    , m_depthTex(0)
    , m_width(0)
    , m_height(0)
    , m_initialized(false)
{
}

DD_GBuffer::~DD_GBuffer()
{
    Shutdown();
}

bool DD_GBuffer::Initialize(int width, int height)
{
    m_width = width;
    m_height = height;

    if (!CreateBuffers())
    {
        printf("Failed to create G-Buffer\n");
        return false;
    }

    m_initialized = true;
    printf("G-Buffer initialized: %dx%d\n", width, height);
    return true;
}

void DD_GBuffer::Shutdown()
{
    DestroyBuffers();
    m_initialized = false;
}

void DD_GBuffer::Resize(int width, int height)
{
    if (width == m_width && height == m_height) return;

    m_width = width;
    m_height = height;

    DestroyBuffers();
    CreateBuffers();
}

bool DD_GBuffer::CreateBuffers()
{
    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

#ifdef __EMSCRIPTEN__
    // WebGL2: RGBA16F needs EXT_color_buffer_float for render targets
    // Use RGBA32F which is more widely supported, or fall back to RGBA8
    GLenum floatFormat = GL_RGBA16F;
    GLenum floatType = GL_HALF_FLOAT;
#else
    GLenum floatFormat = GL_RGBA16F;
    GLenum floatType = GL_FLOAT;
#endif

    // Position texture
    glGenTextures(1, &m_positionTex);
    glBindTexture(GL_TEXTURE_2D, m_positionTex);
    glTexImage2D(GL_TEXTURE_2D, 0, floatFormat, m_width, m_height, 0, GL_RGBA, floatType, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_positionTex, 0);

    // Normal texture
    glGenTextures(1, &m_normalTex);
    glBindTexture(GL_TEXTURE_2D, m_normalTex);
    glTexImage2D(GL_TEXTURE_2D, 0, floatFormat, m_width, m_height, 0, GL_RGBA, floatType, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_normalTex, 0);

    // Albedo + AO texture (RGBA8)
    glGenTextures(1, &m_albedoTex);
    glBindTexture(GL_TEXTURE_2D, m_albedoTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, m_albedoTex, 0);

    // Material texture (RGBA8)
    glGenTextures(1, &m_materialTex);
    glBindTexture(GL_TEXTURE_2D, m_materialTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, m_materialTex, 0);

    // Depth texture
    glGenTextures(1, &m_depthTex);
    glBindTexture(GL_TEXTURE_2D, m_depthTex);
#ifdef __EMSCRIPTEN__
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
#endif
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTex, 0);

    // Set draw buffers
    GLenum attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, attachments);

    // Check completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("G-Buffer framebuffer incomplete: 0x%x\n", status);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    printf("G-Buffer created successfully\n");
    return true;
}

void DD_GBuffer::DestroyBuffers()
{
    if (m_positionTex) { glDeleteTextures(1, &m_positionTex); m_positionTex = 0; }
    if (m_normalTex) { glDeleteTextures(1, &m_normalTex); m_normalTex = 0; }
    if (m_albedoTex) { glDeleteTextures(1, &m_albedoTex); m_albedoTex = 0; }
    if (m_materialTex) { glDeleteTextures(1, &m_materialTex); m_materialTex = 0; }
    if (m_depthTex) { glDeleteTextures(1, &m_depthTex); m_depthTex = 0; }
    if (m_fbo) { glDeleteFramebuffers(1, &m_fbo); m_fbo = 0; }
}

void DD_GBuffer::BindForGeometryPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void DD_GBuffer::UnbindGeometryPass()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DD_GBuffer::BindTexturesForLightingPass()
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_positionTex);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m_normalTex);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m_albedoTex);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m_materialTex);
}

void DD_GBuffer::UnbindTextures()
{
    for (int i = 0; i < 4; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}
