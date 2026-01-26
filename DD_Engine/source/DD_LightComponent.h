#pragma once
#include "DD_Component.h"

enum class LightType
{
    Directional,
    Point,
    Spot
};

class DD_LightComponent : public DD_Component
{
public:
    DD_LightComponent();
    virtual ~DD_LightComponent();

    virtual ComponentType GetType() const override { return ComponentType::Light; }
    virtual void Update(float deltaTime) override;

    // Light type
    void SetLightType(LightType type) { m_lightType = type; m_matricesDirty = true; }
    LightType GetLightType() const { return m_lightType; }

    // Light properties
    void SetColor(const Vec3& color) { m_color = color; }
    Vec3 GetColor() const { return m_color; }

    void SetIntensity(float intensity) { m_intensity = intensity; }
    float GetIntensity() const { return m_intensity; }

    void SetAmbient(float ambient) { m_ambient = ambient; }
    float GetAmbient() const { return m_ambient; }

    // Point/Spot light
    void SetRange(float range) { m_range = range; }
    float GetRange() const { return m_range; }

    // Spot light
    void SetSpotAngle(float innerDegrees, float outerDegrees);
    float GetInnerAngle() const { return m_innerAngle; }
    float GetOuterAngle() const { return m_outerAngle; }

    // Shadow mapping
    void SetCastShadow(bool cast) { m_castShadow = cast; }
    bool GetCastShadow() const { return m_castShadow; }

    void SetShadowOrthoSize(float size) { m_shadowOrthoSize = size; m_matricesDirty = true; }
    float GetShadowOrthoSize() const { return m_shadowOrthoSize; }

    void SetShadowNearFar(float nearPlane, float farPlane);
    float GetShadowNear() const { return m_shadowNear; }
    float GetShadowFar() const { return m_shadowFar; }

    void SetShadowBias(float bias) { m_shadowBias = bias; }
    float GetShadowBias() const { return m_shadowBias; }

    // Camera follow for directional light shadows
    void SetFollowCamera(bool follow) { m_followCamera = follow; }
    bool GetFollowCamera() const { return m_followCamera; }
    void SetCameraPosition(const Vec3& camPos) { m_cameraPosition = camPos; m_matricesDirty = true; }

    // Light space matrix (uses owner's transform)
    Matrix4 GetLightViewMatrix() const;
    Matrix4 GetLightProjectionMatrix() const;
    Matrix4 GetLightSpaceMatrix() const;

    // Get direction from owner's rotation
    Vec3 GetDirection() const;

private:
    void UpdateLightMatrices() const;

private:
    LightType m_lightType;
    Vec3 m_color;
    float m_intensity;
    float m_ambient;

    // Point/Spot
    float m_range;
    float m_innerAngle;  // radians
    float m_outerAngle;  // radians

    // Shadow
    bool m_castShadow;
    float m_shadowOrthoSize;
    float m_shadowNear;
    float m_shadowFar;
    float m_shadowBias;

    // Camera follow
    bool m_followCamera;
    Vec3 m_cameraPosition;

    // Cached matrices
    mutable Matrix4 m_lightView;
    mutable Matrix4 m_lightProj;
    mutable bool m_matricesDirty;
};
