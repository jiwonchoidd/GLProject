#pragma once
#include "DD_GLHelper.h"

enum class RenderTargetType
{
    Color,          // Color + Depth
    DepthOnly,      // Depth only (for shadow maps)
    ColorDepth      // Color + Depth texture
};

class DD_RenderTarget
{
public:
    DD_RenderTarget();
    ~DD_RenderTarget();

    bool Create(int width, int height, RenderTargetType type = RenderTargetType::Color);
    void Destroy();
    void Resize(int width, int height);

    void Bind();
    void Unbind();

    GLuint GetFramebuffer() const { return m_fbo; }
    GLuint GetColorTexture() const { return m_colorTexture; }
    GLuint GetDepthTexture() const { return m_depthTexture; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    bool CreateColorTarget();
    bool CreateDepthOnlyTarget();
    bool CreateColorDepthTarget();

private:
    GLuint m_fbo;
    GLuint m_colorTexture;
    GLuint m_depthTexture;
    GLuint m_depthRBO;       // Renderbuffer for depth when not sampling
    
    int m_width;
    int m_height;
    RenderTargetType m_type;
    bool m_initialized;
};
