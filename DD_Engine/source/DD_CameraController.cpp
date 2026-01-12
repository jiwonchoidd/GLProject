#include "DD_CameraController.h"
#include "DD_Camera.h"
#include "DD_Transform.h"
#include <glm/gtx/quaternion.hpp>

DD_CameraController::DD_CameraController(DD_Camera* camera)
    : m_camera(camera)
{}

void DD_CameraController::OnPointerDown(int x, int y)
{
    m_dragging = true;
    m_lastX = x; m_lastY = y;
}

void DD_CameraController::OnPointerUp(int x, int y)
{
    m_dragging = false;
}

void DD_CameraController::OnPointerMove(int x, int y)
{
    if (!m_dragging || !m_camera) return;
    int dx = x - m_lastX;
    int dy = y - m_lastY;
    m_lastX = x; m_lastY = y;

    float yaw = -dx * m_sensitivity;
    float pitch = -dy * m_sensitivity;

    Quaternion current = m_camera->GetRotationQuat();
    glm::quat qyaw = glm::angleAxis(yaw, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::vec3 right = current * glm::vec3(1.0f, 0.0f, 0.0f);
    glm::quat qpitch = glm::angleAxis(pitch, right);

    Quaternion result = qyaw * qpitch * current;
    m_camera->SetRotationQuat(result);
}

void DD_CameraController::OnScroll(float delta)
{
    if (!m_camera) return;
    Quaternion q = m_camera->GetRotationQuat();
    glm::vec3 forward = q * glm::vec3(0.0f, 0.0f, -1.0f);
    Vec3 pos = m_camera->GetPosition();
    pos += Vec3(forward) * (delta * m_zoomSpeed);
    m_camera->SetPosition(pos);
}
