#pragma once
#include "DD_GLHelper.h"
#include "DD_CollisionComponent.h"

namespace DebugDraw
{
    void Init();
    void Shutdown();

    // Draw an AABB as wireframe in world space. view and projection are passed in.
    void DrawAABBWire(const AABB& aabb, const Matrix4& view, const Matrix4& proj, const Vec4& color = Vec4(1.0f, 0.0f, 0.0f, 1.0f));

    extern bool Enabled;
}
