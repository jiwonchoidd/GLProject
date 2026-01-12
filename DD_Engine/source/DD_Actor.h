#pragma once
#include "DD_GLHelper.h"
#include "DD_Transform.h"
#include "DD_MeshComponent.h"
#include "DD_CollisionComponent.h"
#include <memory>

class DD_Actor
{
public:
    DD_Actor();
    ~DD_Actor();

    void SetMeshComponent(DD_MeshComponent* mesh) { meshComp = mesh; }
    DD_MeshComponent* GetMeshComponent() const { return meshComp; }

    void SetCollisionComponent(DD_CollisionComponent* coll) { collisionComp = coll; }
    DD_CollisionComponent* GetCollisionComponent() const { return collisionComp; }

    // Transform API - minimal: only setters/getters
    void SetPosition(const Vec3& pos) { m_transform.position = pos; }
    Vec3 GetPosition() const { return m_transform.position; }

    void SetRotationQuat(const Quaternion& q) { SetQuat(m_transform, q); }
    Quaternion GetRotationQuat() const { return GetQuat(m_transform); }

    void SetRotationEuler(const Vec3& eulerRad) { m_transform.rotation = eulerRad; }
    Vec3 GetRotationEuler() const { return m_transform.rotation; }

    void SetScale(const Vec3& s) { m_transform.scale = s; }
    Vec3 GetScale() const { return m_transform.scale; }

    // simple position/rotation helpers
    void AddPosition(const Vec3& delta) { Translate(m_transform, delta); }
    void AddRotationEuler(const Vec3& eulerDelta) { Rotate(m_transform, eulerDelta); }
    void AddRotationAxis(float angleRadians, const Vec3& axis) { RotateAxis(m_transform, angleRadians, axis); }

    // non-inline heavier functions implemented in cpp
    Matrix4 GetModelMatrix() const;
    void Render(const Matrix4& view, const Matrix4& projection);

public:
    // components are non-owning pointers; owner expected elsewhere (World or Resource manager)
    DD_MeshComponent* meshComp = nullptr;
    DD_CollisionComponent* collisionComp = nullptr;

private:
    Transform m_transform;
};
