#pragma once
#include "DD_GLHelper.h"
#include "DD_RenderTarget.h"
#include <memory>

class DD_Actor;
class DD_LightComponent;

class DD_ShadowRenderer
{
public:
    DD_ShadowRenderer();
    ~DD_ShadowRenderer();

    bool Initialize(int shadowMapSize = 2048);
    void Shutdown();

    void BeginShadowPass(const DD_LightComponent& light);
    void RenderActor(DD_Actor* actor);
    void EndShadowPass();

    GLuint GetShadowMap() const;
    Matrix4 GetLightSpaceMatrix() const { return m_lightSpaceMatrix; }
    int GetShadowMapSize() const { return m_shadowMapSize; }

    static bool CacheShaders();
    static void ClearShaders();

private:
    std::unique_ptr<DD_RenderTarget> m_shadowTarget;
    Matrix4 m_lightSpaceMatrix;
    int m_shadowMapSize;
    bool m_initialized;

    // Static shader resources
    static GLuint s_depthProgram;
    static GLint s_depthModelLoc;
    static GLint s_depthLightSpaceLoc;
    static bool s_shadersReady;
};
