#pragma once

#include "DD_GLHelper.h"

class DD_World
{
public:
    DD_World();
    ~DD_World();

    void Init(int width, int height);
    void Update(float deltaTime);
    void PostTick(float deltaTime);
    void Render();

    void SetDebugDraw(bool enabled);

    class DD_Camera* GetCamera() const { return m_camera.get(); }

private:
    // component storage owned by the world
    std::vector<std::unique_ptr<class DD_MeshComponent>> m_meshComponents;
    std::vector<std::unique_ptr<class DD_CollisionComponent>> m_collisionComponents;

    std::vector<std::unique_ptr<class DD_Actor>> m_actors;
    std::unique_ptr<class DD_Camera> m_camera;
    std::unique_ptr<class DD_Mesh> m_sharedMesh;

    float m_simTime = 0.0f;

    // simple per-actor mover for cosine-based motion used in simulation
    struct Mover
    {
        bool enabled;
        Vec3 basePosition;
        float amplitude;
        float frequency;
        float phase;
        Vec3 axis;
    };
    std::vector<Mover> m_movers;

    // collision helpers
    void ProcessCollisions(float deltaTime);
};

