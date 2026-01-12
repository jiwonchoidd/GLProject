#pragma once
#include "DD_GLHelper.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

struct Transform
{
    Vec3 position;
	Vec3 rotation; // Euler angles in radians
    Vec3 scale;

    Transform()
        : position(0.0f), rotation(0.0f), scale(1.0f) {}
};

using Quaternion = glm::quat;

inline Matrix4 BuildModelMatrix(const Transform& t)
{
    Matrix4 model = glm::translate(Matrix4(1.0f), t.position);
    Quaternion q = glm::quat(t.rotation);
    model *= glm::toMat4(q);
    model = glm::scale(model, t.scale);
    return model;
}

inline Quaternion QuatFromEuler(const Vec3& eulerRadians)
{
    return glm::quat(eulerRadians);
}

inline Vec3 EulerFromQuat(const Quaternion& q)
{
    return glm::eulerAngles(q);
}

inline Quaternion GetQuat(const Transform& t)
{
    return QuatFromEuler(t.rotation);
}

inline void SetQuat(Transform& t, const Quaternion& q)
{
    t.rotation = EulerFromQuat(q);
}

inline void Translate(Transform& t, const Vec3& delta) { t.position += delta; }

// Rotate by Euler angles (radians) - apply as q = q_delta * q_current
inline void Rotate(Transform& t, const Vec3& eulerDelta)
{
    Quaternion q = GetQuat(t);
    Quaternion dq = QuatFromEuler(eulerDelta);
    Quaternion r = dq * q;
    SetQuat(t, r);
}

// Rotate by axis-angle (angle in radians)
inline void RotateAxis(Transform& t, float angleRadians, const Vec3& axis)
{
    Quaternion q = GetQuat(t);
    Quaternion dq = glm::angleAxis(angleRadians, glm::normalize(axis));
    Quaternion r = dq * q;
    SetQuat(t, r);
}

// Set scale
inline void Scale(Transform& t, const Vec3& s) { t.scale = s; }
