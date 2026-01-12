#pragma once
#include "DD_GLHelper.h"
#include "DD_Transform.h"

class DD_Camera
{
public:
    DD_Camera();
    ~DD_Camera();

    void SetPosition(const Vec3& pos) { m_transform.position = pos; }
    Vec3 GetPosition() const { return m_transform.position; }

    void SetRotationQuat(const Quaternion& q) { SetQuat(m_transform, q); }
    Quaternion GetRotationQuat() const { return GetQuat(m_transform); }

    void SetRotationEuler(const Vec3& euler) { m_transform.rotation = euler; }
    Vec3 GetRotationEuler() const { return m_transform.rotation; }

    void SetScale(const Vec3& s) { m_transform.scale = s; }
    Vec3 GetScale() const { return m_transform.scale; }

    void UpdateProjection(int width, int height);
    void UpdateView();

    const Matrix4& GetViewMatrix() const { return m_view; }
    const Matrix4& GetProjectionMatrix() const { return m_projection; }

private:
    Transform m_transform;
    Matrix4 m_view;
    Matrix4 m_projection;
    float m_fov;
};