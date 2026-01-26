#pragma once

#include "DD_GLHelper.h"

class DD_Light;
class DD_LightActor;
class DD_ShadowRenderer;
class DD_SceneRenderer;
class DD_DeferredRenderer;
class DD_Material;
class DD_Texture;

class DD_World
{
public:
    DD_World();
    ~DD_World();

    void Init(int width, int height);
    void Update(float deltaTime);
    void PostTick(float deltaTime);
    void Render();
    void Resize(int width, int height);

    void SetDebugDraw(bool enabled);
    void SetDeferredRendering(bool enabled) { m_useDeferredRendering = enabled; }
    bool IsDeferredRendering() const { return m_useDeferredRendering; }

    // Actor management
    void AddActor(class DD_Actor* actor);
    void RemoveActor(class DD_Actor* actor);
    const std::vector<std::unique_ptr<class DD_Actor>>& GetActors() const { return m_actors; }

    // Light management
    DD_LightActor* CreateDirectionalLight();
    DD_LightActor* CreatePointLight(const Vec3& position, const Vec3& color, float intensity, float radius);
    DD_LightActor* GetMainLight() const { return m_mainLight; }
    const std::vector<DD_LightActor*>& GetLights() const { return m_lights; }

    class DD_Camera* GetCamera() const { return m_camera.get(); }

    // Shadow settings
    void SetShadowEnabled(bool enabled) { m_shadowEnabled = enabled; }
    bool IsShadowEnabled() const { return m_shadowEnabled; }

private:
    // Component storage owned by the world
    std::vector<std::unique_ptr<class DD_MeshComponent>> m_meshComponents;
    std::vector<std::unique_ptr<class DD_CollisionComponent>> m_collisionComponents;
    std::vector<DD_Material*> m_materials;
    std::vector<DD_Texture*> m_textures;

    std::vector<std::unique_ptr<class DD_Actor>> m_actors;
    std::vector<DD_LightActor*> m_lights;  // Non-owning pointers to lights in m_actors
    DD_LightActor* m_mainLight;            // Primary light for shadows

    std::unique_ptr<class DD_Camera> m_camera;
    std::unique_ptr<class DD_Mesh> m_sharedMesh;

    // Rendering system
    std::unique_ptr<DD_ShadowRenderer> m_shadowRenderer;
    std::unique_ptr<DD_SceneRenderer> m_sceneRenderer;
    std::unique_ptr<DD_DeferredRenderer> m_deferredRenderer;
    
    bool m_shadowEnabled;
    bool m_useDeferredRendering;
    int m_viewportWidth;
    int m_viewportHeight;

    float m_simTime = 0.0f;

    // simple per-actor mover for cosine-based motion used in simulation
    struct Mover
    {
        bool enabled = false;
        Vec3 basePosition;
        float amplitude = 0.0f;
        float frequency = 1.0f;
        float phase = 0.0f;
        Vec3 axis;
    };
    std::vector<Mover> m_movers;

    // collision helpers
    void ProcessCollisions(float deltaTime);

    // Rendering passes
    void RenderShadowPass();
    void RenderScenePass();
    void RenderDeferred();
    void RenderLegacy();  // Fallback without shadows
};

