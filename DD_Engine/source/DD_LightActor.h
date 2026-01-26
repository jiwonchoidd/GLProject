#pragma once
#include "DD_Actor.h"
#include "DD_LightComponent.h"
#include <memory>

class DD_LightActor : public DD_Actor
{
public:
    DD_LightActor();
    virtual ~DD_LightActor();

    virtual void Update(float deltaTime) override;

    // Light access (convenience methods)
    DD_LightComponent* GetLightComponent() const { return m_lightComponent.get(); }

    // Quick accessors through light component
    void SetLightType(LightType type) { m_lightComponent->SetLightType(type); }
    LightType GetLightType() const { return m_lightComponent->GetLightType(); }

    void SetColor(const Vec3& color) { m_lightComponent->SetColor(color); }
    Vec3 GetColor() const { return m_lightComponent->GetColor(); }

    void SetIntensity(float intensity) { m_lightComponent->SetIntensity(intensity); }
    float GetIntensity() const { return m_lightComponent->GetIntensity(); }

    void SetAmbient(float ambient) { m_lightComponent->SetAmbient(ambient); }
    float GetAmbient() const { return m_lightComponent->GetAmbient(); }

    // Shadow
    void SetCastShadow(bool cast) { m_lightComponent->SetCastShadow(cast); }
    bool GetCastShadow() const { return m_lightComponent->GetCastShadow(); }

    void SetShadowOrthoSize(float size) { m_lightComponent->SetShadowOrthoSize(size); }
    void SetShadowNearFar(float nearPlane, float farPlane) { m_lightComponent->SetShadowNearFar(nearPlane, farPlane); }
    void SetShadowBias(float bias) { m_lightComponent->SetShadowBias(bias); }
    float GetShadowBias() const { return m_lightComponent->GetShadowBias(); }

    // Direction (derived from rotation)
    Vec3 GetDirection() const { return m_lightComponent->GetDirection(); }
    void SetDirection(const Vec3& dir);  // Sets rotation from direction

    // Light matrices
    Matrix4 GetLightViewMatrix() const { return m_lightComponent->GetLightViewMatrix(); }
    Matrix4 GetLightProjectionMatrix() const { return m_lightComponent->GetLightProjectionMatrix(); }
    Matrix4 GetLightSpaceMatrix() const { return m_lightComponent->GetLightSpaceMatrix(); }

private:
    std::unique_ptr<DD_LightComponent> m_lightComponent;
};
