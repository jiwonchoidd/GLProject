#pragma once
#include "DD_GLHelper.h"

// G-Buffer for Deferred Rendering
// Layout:
//   RT0: Position (RGB) + Depth (A)       - RGBA16F
//   RT1: Normal (RGB) + unused (A)        - RGBA16F  
//   RT2: Albedo (RGB) + AO (A)            - RGBA8
//   RT3: Metallic (R) + Roughness (G) + EmissiveFlag (B) + unused (A) - RGBA8

class DD_GBuffer
{
public:
    DD_GBuffer();
    ~DD_GBuffer();

    bool Initialize(int width, int height);
    void Shutdown();
    void Resize(int width, int height);

    void BindForGeometryPass();
    void UnbindGeometryPass();

    void BindTexturesForLightingPass();
    void UnbindTextures();

    GLuint GetPositionTexture() const { return m_positionTex; }
    GLuint GetNormalTexture() const { return m_normalTex; }
    GLuint GetAlbedoTexture() const { return m_albedoTex; }
    GLuint GetMaterialTexture() const { return m_materialTex; }
    GLuint GetDepthTexture() const { return m_depthTex; }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }

private:
    bool CreateBuffers();
    void DestroyBuffers();

private:
    GLuint m_fbo;
    GLuint m_positionTex;   // RT0: World Position + Depth
    GLuint m_normalTex;     // RT1: World Normal
    GLuint m_albedoTex;     // RT2: Albedo + AO
    GLuint m_materialTex;   // RT3: Metallic + Roughness
    GLuint m_depthTex;      // Depth buffer

    int m_width;
    int m_height;
    bool m_initialized;
};
