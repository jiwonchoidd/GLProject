#pragma once
#include "DD_GLHelper.h"
#include "DD_Transform.h"
#include <memory>
#include <vector>
#include <string>

class DD_Component;
class DD_MeshComponent;
class DD_CollisionComponent;

class DD_Actor
{
public:
    DD_Actor();
    virtual ~DD_Actor();

    // Component system
    void AddComponent(DD_Component* component);
    void RemoveComponent(DD_Component* component);
    
    template<typename T>
    T* GetComponent() const
    {
        for (auto* comp : m_components)
        {
            T* result = dynamic_cast<T*>(comp);
            if (result) return result;
        }
        return nullptr;
    }

    const std::vector<DD_Component*>& GetComponents() const { return m_components; }

    // Legacy component accessors (for backward compatibility)
    void SetMeshComponent(DD_MeshComponent* mesh);
    DD_MeshComponent* GetMeshComponent() const { return meshComp; }

    void SetCollisionComponent(DD_CollisionComponent* coll);
    DD_CollisionComponent* GetCollisionComponent() const { return collisionComp; }

    // Transform API
    void SetPosition(const Vec3& pos) { m_transform.position = pos; OnTransformChanged(); }
    Vec3 GetPosition() const { return m_transform.position; }

    void SetRotationQuat(const Quaternion& q) { SetQuat(m_transform, q); OnTransformChanged(); }
    Quaternion GetRotationQuat() const { return GetQuat(m_transform); }

    void SetRotationEuler(const Vec3& eulerRad) { m_transform.rotation = eulerRad; OnTransformChanged(); }
    Vec3 GetRotationEuler() const { return m_transform.rotation; }

    void SetScale(const Vec3& s) { m_transform.scale = s; OnTransformChanged(); }
    Vec3 GetScale() const { return m_transform.scale; }

    // Position/rotation helpers
    void AddPosition(const Vec3& delta) { Translate(m_transform, delta); OnTransformChanged(); }
    void AddRotationEuler(const Vec3& eulerDelta) { Rotate(m_transform, eulerDelta); OnTransformChanged(); }
    void AddRotationAxis(float angleRadians, const Vec3& axis) { RotateAxis(m_transform, angleRadians, axis); OnTransformChanged(); }

    // Transform access
    const Transform& GetTransform() const { return m_transform; }
    Transform& GetTransform() { return m_transform; }

    // Virtual methods for derived actors
    virtual void Update(float deltaTime);
    virtual void Render(const Matrix4& view, const Matrix4& projection);
    
    Matrix4 GetModelMatrix() const;

    // Actor lifecycle
    void SetActive(bool active) { m_active = active; }
    bool IsActive() const { return m_active; }

    void SetName(const std::string& name) { m_name = name; }
    const std::string& GetName() const { return m_name; }

protected:
    virtual void OnTransformChanged() {}

protected:
    // Legacy component pointers (for backward compatibility)
    DD_MeshComponent* meshComp = nullptr;
    DD_CollisionComponent* collisionComp = nullptr;

    // Component list (non-owning)
    std::vector<DD_Component*> m_components;

private:
    Transform m_transform;
    bool m_active;
    std::string m_name;
};
