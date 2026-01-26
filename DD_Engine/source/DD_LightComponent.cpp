#include "DD_LightComponent.h"
#include "DD_Actor.h"
#include <glm/gtc/matrix_transform.hpp>

DD_LightComponent::DD_LightComponent()
    : m_lightType(LightType::Directional)
    , m_color(1.0f, 1.0f, 1.0f)
    , m_intensity(1.0f)
    , m_ambient(0.15f)
    , m_range(10.0f)
    , m_innerAngle(glm::radians(30.0f))
    , m_outerAngle(glm::radians(45.0f))
    , m_castShadow(true)
    , m_shadowOrthoSize(25.0f)
    , m_shadowNear(1.0f)
    , m_shadowFar(100.0f)
    , m_shadowBias(0.005f)
    , m_followCamera(true)
    , m_cameraPosition(0.0f)
    , m_lightView(1.0f)
    , m_lightProj(1.0f)
    , m_matricesDirty(true)
{
}

DD_LightComponent::~DD_LightComponent()
{
}

void DD_LightComponent::Update(float deltaTime)
{
    m_matricesDirty = true;
}

void DD_LightComponent::SetSpotAngle(float innerDegrees, float outerDegrees)
{
    m_innerAngle = glm::radians(innerDegrees);
    m_outerAngle = glm::radians(outerDegrees);
}

void DD_LightComponent::SetShadowNearFar(float nearPlane, float farPlane)
{
    m_shadowNear = nearPlane;
    m_shadowFar = farPlane;
    m_matricesDirty = true;
}

Vec3 DD_LightComponent::GetDirection() const
{
    if (!m_owner)
    {
        return Vec3(0.0f, -1.0f, 0.0f);
    }

    Quaternion q = m_owner->GetRotationQuat();
    Vec3 forward = q * Vec3(0.0f, 0.0f, -1.0f);
    return glm::normalize(forward);
}

Matrix4 DD_LightComponent::GetLightViewMatrix() const
{
    if (m_matricesDirty)
    {
        UpdateLightMatrices();
    }
    return m_lightView;
}

Matrix4 DD_LightComponent::GetLightProjectionMatrix() const
{
    if (m_matricesDirty)
    {
        UpdateLightMatrices();
    }
    return m_lightProj;
}

Matrix4 DD_LightComponent::GetLightSpaceMatrix() const
{
    return GetLightProjectionMatrix() * GetLightViewMatrix();
}

void DD_LightComponent::UpdateLightMatrices() const
{
    Vec3 direction = GetDirection();
    
    // Shadow center - follows camera if enabled
    Vec3 shadowCenter = m_followCamera ? m_cameraPosition : Vec3(0.0f);
    // Snap to texel grid to reduce shadow swimming
    float texelSize = (m_shadowOrthoSize * 2.0f) / 2048.0f;
    shadowCenter.x = floor(shadowCenter.x / texelSize) * texelSize;
    shadowCenter.z = floor(shadowCenter.z / texelSize) * texelSize;
    shadowCenter.y = 0.0f;  // Keep shadow at ground level
    
    // Light position: place light far away from shadow center
    float lightDistance = m_shadowFar * 0.5f;
    Vec3 lightPos = shadowCenter - direction * lightDistance;

    Vec3 up = Vec3(0.0f, 1.0f, 0.0f);
    if (glm::abs(glm::dot(direction, up)) > 0.99f)
    {
        up = Vec3(1.0f, 0.0f, 0.0f);
    }

    m_lightView = glm::lookAt(lightPos, shadowCenter, up);

    switch (m_lightType)
    {
    case LightType::Directional:
        {
            float halfSize = m_shadowOrthoSize;
            m_lightProj = glm::ortho(-halfSize, halfSize, -halfSize, halfSize, m_shadowNear, m_shadowFar);
        }
        break;

    case LightType::Point:
        m_lightProj = glm::perspective(glm::radians(90.0f), 1.0f, m_shadowNear, m_range);
        break;

    case LightType::Spot:
        m_lightProj = glm::perspective(m_outerAngle * 2.0f, 1.0f, m_shadowNear, m_range);
        break;
    }

    m_matricesDirty = false;
}
