#pragma once
#include "DD_GLHelper.h"

class DD_LightComponent;
class DD_Actor;
class DD_Material;

class DD_SceneRenderer
{
public:
    DD_SceneRenderer();
    ~DD_SceneRenderer();

    static bool CacheShaders();
    static void ClearShaders();

    void BeginScenePass(const Matrix4& view, const Matrix4& projection,
                        const DD_LightComponent& light, GLuint shadowMap,
                        const Matrix4& lightSpaceMatrix,
                        const Vec3& cameraPos);
    void RenderActor(DD_Actor* actor);
    void RenderActorWithMaterial(DD_Actor* actor, DD_Material* material);
    void EndScenePass();

private:
    Matrix4 m_view;
    Matrix4 m_projection;
    Matrix4 m_lightSpaceMatrix;
    Vec3 m_cameraPos;

    // Static shader resources
    static GLuint s_sceneProgram;
    static GLint s_modelLoc;
    static GLint s_viewLoc;
    static GLint s_projLoc;
    static GLint s_lightSpaceLoc;
    static GLint s_shadowMapLoc;
    static GLint s_lightDirLoc;
    static GLint s_lightColorLoc;
    static GLint s_ambientLoc;
    static GLint s_shadowBiasLoc;
    static GLint s_cameraPosLoc;
    
    // Material uniforms
    static GLint s_albedoLoc;
    static GLint s_metallicLoc;
    static GLint s_roughnessLoc;
    static GLint s_aoLoc;
    static GLint s_hasAlbedoTexLoc;
    static GLint s_albedoTexLoc;
    
    static bool s_shadersReady;
};
