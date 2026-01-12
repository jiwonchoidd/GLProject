#pragma once
#include "DD_CollisionComponent.h"

class CollisionUtils
{
public:
    // Test AABB vs AABB. Returns true if colliding and sets outMTV to the minimum translation vector to separate B from A.
    static bool TestAABBvsAABB(const AABB& A, const AABB& B, Vec3& outMTV);
};
