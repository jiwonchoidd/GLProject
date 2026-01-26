#pragma once
#include "DD_GLHelper.h"

class DD_Actor;
class DD_Material;
class DD_GBuffer;
class DD_LightComponent;

// Maximum lights for deferred rendering
constexpr int MAX_POINT_LIGHTS = 32;
constexpr int MAX_SPOT_LIGHTS = 8;

struct PointLightData
{
    Vec3 position;
    float radius;
    Vec3 color;
    float intensity;
};

struct SpotLightData
{
    Vec3 position;
    float innerAngle;
    Vec3 direction;
    float outerAngle;
    Vec3 color;
    float intensity;
    float range;
    float padding[3];
};

class DD_DeferredRenderer
{
public:
    DD_DeferredRenderer();
    ~DD_DeferredRenderer();

    bool Initialize(int width, int height);
    void Shutdown();
    void Resize(int width, int height);

    // Geometry Pass - renders to G-Buffer
    void BeginGeometryPass(const Matrix4& view, const Matrix4& projection);
    void RenderActor(DD_Actor* actor);
    void EndGeometryPass();

    // Lighting Pass - reads G-Buffer, outputs final image
    void BeginLightingPass(const Vec3& cameraPos);
    void SetDirectionalLight(const DD_LightComponent* light, GLuint shadowMap, const Matrix4& lightSpaceMatrix);
    void AddPointLight(const Vec3& position, const Vec3& color, float intensity, float radius);
    void EndLightingPass();

    void ClearPointLights() { m_pointLightCount = 0; }

    DD_GBuffer* GetGBuffer() { return m_gBuffer.get(); }

    static bool CacheShaders();
    static void ClearShaders();

private:
    void RenderFullscreenQuad();

private:
    std::unique_ptr<DD_GBuffer> m_gBuffer;
    Matrix4 m_view;
    Matrix4 m_projection;

    // Point lights
    PointLightData m_pointLights[MAX_POINT_LIGHTS];
    int m_pointLightCount;

    // Fullscreen quad VAO
    GLuint m_quadVAO;
    GLuint m_quadVBO;

    // Geometry pass shader
    static GLuint s_geometryProgram;
    static GLint s_geoModelLoc;
    static GLint s_geoViewLoc;
    static GLint s_geoProjLoc;
    static GLint s_geoAlbedoLoc;
    static GLint s_geoMetallicLoc;
    static GLint s_geoRoughnessLoc;
    static GLint s_geoAOLoc;
    static GLint s_geoHasAlbedoTexLoc;
    static GLint s_geoAlbedoTexLoc;

    // Lighting pass shader
    static GLuint s_lightingProgram;
    static GLint s_litPositionTexLoc;
    static GLint s_litNormalTexLoc;
    static GLint s_litAlbedoTexLoc;
    static GLint s_litMaterialTexLoc;
    static GLint s_litShadowMapLoc;
    static GLint s_litCameraPosLoc;
    static GLint s_litLightSpaceLoc;
    
    // Directional light uniforms
    static GLint s_litDirLightDirLoc;
    static GLint s_litDirLightColorLoc;
    static GLint s_litDirLightAmbientLoc;
    static GLint s_litShadowBiasLoc;
    
    // Point light uniforms
    static GLint s_litPointLightCountLoc;
    static GLint s_litPointLightsLoc;

    static bool s_shadersReady;
};
