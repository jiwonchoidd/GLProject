#include "DD_CollisionUtils.h"
#include <cmath>

bool CollisionUtils::TestAABBvsAABB(const AABB& A, const AABB& B, Vec3& outMTV)
{
    Vec3 delta = B.center - A.center;
    Vec3 overlap = Vec3(
        (A.halfExtents.x + B.halfExtents.x) - fabs(delta.x),
        (A.halfExtents.y + B.halfExtents.y) - fabs(delta.y),
        (A.halfExtents.z + B.halfExtents.z) - fabs(delta.z)
    );

    if (overlap.x > 0 && overlap.y > 0 && overlap.z > 0)
    {
        if (overlap.x < overlap.y && overlap.x < overlap.z)
        {
            outMTV = Vec3((delta.x < 0) ? -overlap.x : overlap.x, 0, 0);
        }
        else if (overlap.y < overlap.z)
        {
            outMTV = Vec3(0, (delta.y < 0) ? -overlap.y : overlap.y, 0);
        }
        else
        {
            outMTV = Vec3(0, 0, (delta.z < 0) ? -overlap.z : overlap.z);
        }
        return true;
    }

    return false;
}
