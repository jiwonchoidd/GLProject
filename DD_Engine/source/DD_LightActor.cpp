#include "DD_LightActor.h"
#include <glm/gtc/matrix_transform.hpp>

DD_LightActor::DD_LightActor()
    : m_lightComponent(std::make_unique<DD_LightComponent>())
{
    SetName("LightActor");
    AddComponent(m_lightComponent.get());
}

DD_LightActor::~DD_LightActor()
{
    RemoveComponent(m_lightComponent.get());
}

void DD_LightActor::Update(float deltaTime)
{
    DD_Actor::Update(deltaTime);
}

void DD_LightActor::SetDirection(const Vec3& dir)
{
    Vec3 normalizedDir = glm::normalize(dir);
    
    Vec3 defaultForward(0.0f, 0.0f, -1.0f);
    
    float dot = glm::dot(defaultForward, normalizedDir);
    
    if (dot > 0.9999f)
    {
        SetRotationQuat(Quaternion(1.0f, 0.0f, 0.0f, 0.0f));
    }
    else if (dot < -0.9999f)
    {
        SetRotationQuat(glm::angleAxis(glm::pi<float>(), Vec3(0.0f, 1.0f, 0.0f)));
    }
    else
    {
        Vec3 axis = glm::cross(defaultForward, normalizedDir);
        float angle = glm::acos(dot);
        SetRotationQuat(glm::angleAxis(angle, glm::normalize(axis)));
    }
}
