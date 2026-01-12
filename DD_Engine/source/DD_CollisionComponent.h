#pragma once
#include "DD_GLHelper.h"

enum class CollisionShapeType
{
    None = 0,
    AABB,
    OBB,
    Sphere
};

struct AABB
{
    Vec3 center;
    Vec3 halfExtents;
};

struct OBB
{
    Vec3 center;
    Vec3 halfExtents;
    glm::quat orientation;
};

struct Sphere
{
    Vec3 center;
    float radius;
};

class DD_CollisionComponent
{
public:
    DD_CollisionComponent() : m_type(CollisionShapeType::None), m_mass(1.0f), m_aabbHalfExtents(1.0f) {}

    CollisionShapeType m_type;
    float m_mass;
    Vec3 m_aabbHalfExtents;
};
