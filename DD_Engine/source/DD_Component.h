#pragma once
#include "DD_GLHelper.h"

class DD_Actor;

enum class ComponentType
{
    None,
    Mesh,
    Collision,
    Light,
    Camera
};

class DD_Component
{
public:
    DD_Component();
    virtual ~DD_Component();

    virtual ComponentType GetType() const { return ComponentType::None; }
    virtual void OnAttach(DD_Actor* owner) { m_owner = owner; }
    virtual void OnDetach() { m_owner = nullptr; }
    virtual void Update(float deltaTime) {}

    DD_Actor* GetOwner() const { return m_owner; }
    void SetEnabled(bool enabled) { m_enabled = enabled; }
    bool IsEnabled() const { return m_enabled; }

protected:
    DD_Actor* m_owner;
    bool m_enabled;
};
