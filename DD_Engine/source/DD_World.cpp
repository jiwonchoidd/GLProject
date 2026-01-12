#include "DD_World.h"
#include "DD_Core.h"
#include "DD_Camera.h"
#include "DD_SimpleBox.h"
#include "DD_Actor.h"
#include "DD_CollisionComponent.h"
#include "DD_MeshComponent.h"
#include "DD_CollisionUtils.h"
#include "DD_DebugDraw.h"
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>

DD_World::DD_World()
    : m_camera(std::make_unique<DD_Camera>())
    , m_sharedMesh(nullptr)
    , m_simTime(0.0f)
{
}

DD_World::~DD_World()
{
}

void DD_World::Init(int width, int height)
{
    m_camera->UpdateProjection(width, height);

    DD_SimpleBox::CachePipline();
    auto box = std::make_unique<DD_SimpleBox>();
    box->CreateMesh();
    m_sharedMesh = std::move(box);

    m_meshComponents.emplace_back(std::make_unique<DD_MeshComponent>());
    m_collisionComponents.push_back(std::make_unique<DD_CollisionComponent>());

    int32_t maxActors = 10;
    printf("Creating %d actors...\n", maxActors);

    m_movers.resize(maxActors);

    for (int32_t i = 0; i < maxActors; ++i)
    {
        auto actor = std::make_unique<DD_Actor>();

        DD_MeshComponent* meshComp = m_meshComponents.back().get();
        meshComp->SetMesh(m_sharedMesh.get());

        actor->SetMeshComponent(meshComp);

        int gridSize = 5;
        float spacing = 3.0f;
        int x = i % gridSize;
        int y = (i / gridSize) % gridSize;
        int z = i / (gridSize * gridSize);

        Vec3 pos(
            (x - gridSize / 2) * spacing,
            (y - gridSize / 2) * spacing,
            (z - gridSize / 2) * spacing
        );

        // initialize actor transform via encapsulated APIs
        actor->SetPosition(pos);
        actor->SetRotationEuler(Vec3(0.0f));
        actor->SetScale(Vec3(1.0f));

        DD_CollisionComponent* col = m_collisionComponents.back().get();
        col->m_type = CollisionShapeType::AABB;
        col->m_aabbHalfExtents = Vec3(1.0f);
        actor->SetCollisionComponent(col);

        // initialize mover for some actors
        if (i % 2 == 0)
        {
            auto& mv = m_movers[i];
            mv.enabled = true;
            mv.basePosition = pos;
            mv.amplitude = 1.5f;
            mv.frequency = 1.0f + i * 0.2f;
            mv.phase = 0.0f;
            mv.axis = Vec3(1.0f, 0.0f, 0.0f);
        }
        else
        {
            auto& mv = m_movers[i];
            mv.enabled = false;
            mv.basePosition = Vec3(0.0f);
            mv.amplitude = 0.0f;
            mv.frequency = 1.0f;
            mv.phase = 0.0f;
            mv.axis = Vec3(1.0f, 0.0f, 0.0f);
        }

        m_actors.push_back(std::move(actor));
    }

    printf("Created %zu actors\n", m_actors.size());
}

void DD_World::Update(float deltaTime)
{
    m_simTime += deltaTime;

    // update movers using cosine-based motion
    for (size_t i = 0; i < m_actors.size() && i < m_movers.size(); ++i)
    {
        auto& mv = m_movers[i];
        if (!mv.enabled) continue;
        float v = cosf(m_simTime * mv.frequency + mv.phase);
        Vec3 offset = mv.axis * (mv.amplitude * v);
        m_actors[i]->SetPosition(mv.basePosition + offset);
    }

    PostTick(deltaTime);
}

void DD_World::PostTick(float deltaTime)
{
    // update camera view once per frame
    m_camera->UpdateView();

    ProcessCollisions(deltaTime);
}

void DD_World::Render()
{
    const Matrix4 view = m_camera->GetViewMatrix();
    const Matrix4 proj = m_camera->GetProjectionMatrix();

    for (auto& actorPtr : m_actors)
    {
        actorPtr->Render(view, proj);
    }

    // Draw debug AABBs
    if (DebugDraw::Enabled)
    {
        for (auto& actorPtr : m_actors)
        {
            DD_CollisionComponent* col = actorPtr->GetCollisionComponent();
            if (col && col->m_type == CollisionShapeType::AABB)
            {
                AABB aabb{actorPtr->GetPosition(), col->m_aabbHalfExtents};
                DebugDraw::DrawAABBWire(aabb, view, proj, Vec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
        }
    }
}

void DD_World::ProcessCollisions(float deltaTime)
{
    size_t n = m_actors.size();
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = i + 1; j < n; ++j)
        {
            DD_CollisionComponent* a = m_actors[i]->GetCollisionComponent();
            DD_CollisionComponent* b = m_actors[j]->GetCollisionComponent();
            if (!a || !b) continue;

            if (a->m_type == CollisionShapeType::AABB && b->m_type == CollisionShapeType::AABB)
            {
                AABB A{m_actors[i]->GetPosition(), a->m_aabbHalfExtents};
                AABB B{m_actors[j]->GetPosition(), b->m_aabbHalfExtents};

                Vec3 mtv;
                if (CollisionUtils::TestAABBvsAABB(A, B, mtv))
                {
                    float ma = a->m_mass;
                    float mb = b->m_mass;

                    float total = ma + mb;
                    if (total < DD_SMALL_NUMBER)
                    {
                        continue;
                    }
                    Vec3 moveA = -mtv * (mb / total);
                    Vec3 moveB = mtv * (ma / total);

                    m_actors[i]->AddPosition(moveA);
                    m_actors[j]->AddPosition(moveB);
                }
            }
        }
    }
}

void DD_World::SetDebugDraw(bool enabled)
{
    DebugDraw::Enabled = enabled;
}
