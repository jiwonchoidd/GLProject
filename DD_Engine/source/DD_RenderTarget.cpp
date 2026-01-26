#include "DD_RenderTarget.h"
#include <cstdio>

DD_RenderTarget::DD_RenderTarget()
    : m_fbo(0)
    , m_colorTexture(0)
    , m_depthTexture(0)
    , m_depthRBO(0)
    , m_width(0)
    , m_height(0)
    , m_type(RenderTargetType::Color)
    , m_initialized(false)
{
}

DD_RenderTarget::~DD_RenderTarget()
{
    Destroy();
}

bool DD_RenderTarget::Create(int width, int height, RenderTargetType type)
{
    if (m_initialized)
    {
        Destroy();
    }

    m_width = width;
    m_height = height;
    m_type = type;

    glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    bool success = false;
    switch (type)
    {
    case RenderTargetType::Color:
        success = CreateColorTarget();
        break;
    case RenderTargetType::DepthOnly:
        success = CreateDepthOnlyTarget();
        break;
    case RenderTargetType::ColorDepth:
        success = CreateColorDepthTarget();
        break;
    }

    if (!success)
    {
        Destroy();
        return false;
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("Framebuffer not complete: 0x%x\n", status);
        Destroy();
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    m_initialized = true;

    printf("RenderTarget created: %dx%d, type=%d\n", width, height, static_cast<int>(type));
    return true;
}

bool DD_RenderTarget::CreateColorTarget()
{
    // Color texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    // Depth renderbuffer (not sampled)
    glGenRenderbuffers(1, &m_depthRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, m_width, m_height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depthRBO);

    return true;
}

bool DD_RenderTarget::CreateDepthOnlyTarget()
{
    // Depth texture for shadow mapping
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    
#ifdef __EMSCRIPTEN__
    // WebGL2/GLES3 compatible
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0, 
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height, 0, 
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
#endif

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    
#ifndef __EMSCRIPTEN__
    // Border color for areas outside shadow map (lit)
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Hardware shadow comparison (PCF friendly)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
#endif

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

    // No color buffer for depth-only
#ifndef __EMSCRIPTEN__
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
#endif

    return true;
}

bool DD_RenderTarget::CreateColorDepthTarget()
{
    // Color texture
    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);

    // Depth texture (sampled)
    glGenTextures(1, &m_depthTexture);
    glBindTexture(GL_TEXTURE_2D, m_depthTexture);
    
#ifdef __EMSCRIPTEN__
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, m_width, m_height, 0,
                 GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, nullptr);
#else
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height, 0,
                 GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
#endif

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depthTexture, 0);

    return true;
}

void DD_RenderTarget::Destroy()
{
    if (m_colorTexture)
    {
        glDeleteTextures(1, &m_colorTexture);
        m_colorTexture = 0;
    }
    if (m_depthTexture)
    {
        glDeleteTextures(1, &m_depthTexture);
        m_depthTexture = 0;
    }
    if (m_depthRBO)
    {
        glDeleteRenderbuffers(1, &m_depthRBO);
        m_depthRBO = 0;
    }
    if (m_fbo)
    {
        glDeleteFramebuffers(1, &m_fbo);
        m_fbo = 0;
    }
    m_initialized = false;
}

void DD_RenderTarget::Resize(int width, int height)
{
    if (m_width == width && m_height == height)
        return;

    RenderTargetType type = m_type;
    Destroy();
    Create(width, height, type);
}

void DD_RenderTarget::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);
}

void DD_RenderTarget::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
