#include "DD_World.h"
#include "DD_SimpleBox.h"
#include "DD_Camera.h"
#include <cstdio>

DD_World::DD_World()
    : m_camera(new DD_Camera())
{
}

DD_World::~DD_World()
{
    for (auto& box : m_boxes)
    {
        if (box)
        {
            delete box;
            box = nullptr;
        }
    }
    m_boxes.clear();

    delete m_camera;
}

void DD_World::Init(int width, int height)
{
    m_camera->UpdateProjection(width, height);

    DD_SimpleBox::CachePipline();

    int32_t maxBoxCount = 100;
    printf("Creating %d boxes...\n", maxBoxCount);

    for (int32_t i = 0; i < maxBoxCount; ++i)
    {
        DD_SimpleBox* box = new DD_SimpleBox();

        int gridSize = 5;
        float spacing = 5.0f;
        int x = i % gridSize;
        int y = (i / gridSize) % gridSize;
        int z = i / (gridSize * gridSize);

        Vec3 pos(
            (x - gridSize / 2) * spacing,
            (y - gridSize / 2) * spacing,
            (z - gridSize / 2) * spacing
        );

        box->Create(pos);
        m_boxes.push_back(box);
    }

    printf("Created %zu boxes\n", m_boxes.size());
}

void DD_World::Update(float deltaTime)
{
    for (const auto& box : m_boxes)
    {
        //box->AddPos(Vec3(0.f, 0.5f * deltaTime, 0.f));
        box->AddRot(Vec3(0.f, 0.3f * deltaTime, 0.f));
    }
}

void DD_World::Render()
{
    // Update global matrices
    extern Matrix4 g_View;
    extern Matrix4 g_Projection;
    g_View = m_camera->GetViewMatrix();
    g_Projection = m_camera->GetProjectionMatrix();

    for (const auto& box : m_boxes)
    {
        box->Render();
    }
}
