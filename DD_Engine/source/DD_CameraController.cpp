#include "DD_CameraController.h"
#include "DD_Camera.h"
#include "DD_Transform.h"
#include "DD_GLHelper.h"
#include <glm/gtx/quaternion.hpp>

DD_CameraController::DD_CameraController(DD_Camera* camera)
    : m_camera(camera)
{}

void DD_CameraController::Update(float deltaTime)
{
    if (!m_camera) return;

    // Calculate movement direction based on pressed keys
    Vec3 moveDir(0.0f);

    Quaternion q = m_camera->GetRotationQuat();
    Vec3 forward = q * Vec3(0.0f, 0.0f, -1.0f);
    Vec3 right = q * Vec3(1.0f, 0.0f, 0.0f);
    Vec3 up = Vec3(0.0f, 1.0f, 0.0f);

    if (m_keyW) moveDir += forward;
    if (m_keyS) moveDir -= forward;
    if (m_keyD) moveDir += right;
    if (m_keyA) moveDir -= right;
    if (m_keyE) moveDir += up;
    if (m_keyQ) moveDir -= up;

    if (glm::length(moveDir) > 0.001f)
    {
        moveDir = glm::normalize(moveDir);
        float speed = m_moveSpeed * (m_keyShift ? 2.5f : 1.0f);
        Vec3 pos = m_camera->GetPosition();
        pos += moveDir * speed * deltaTime;
        m_camera->SetPosition(pos);
    }
}

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

void DD_CameraController::OnKeyDown(int key)
{
    // GLFW key codes
    switch (key)
    {
    case GLFW_KEY_W: m_keyW = true; break;
    case GLFW_KEY_A: m_keyA = true; break;
    case GLFW_KEY_S: m_keyS = true; break;
    case GLFW_KEY_D: m_keyD = true; break;
    case GLFW_KEY_Q: m_keyQ = true; break;
    case GLFW_KEY_E: m_keyE = true; break;
    case GLFW_KEY_LEFT_SHIFT:
    case GLFW_KEY_RIGHT_SHIFT:
        m_keyShift = true; break;
    }
}

void DD_CameraController::OnKeyUp(int key)
{
    switch (key)
    {
    case GLFW_KEY_W: m_keyW = false; break;
    case GLFW_KEY_A: m_keyA = false; break;
    case GLFW_KEY_S: m_keyS = false; break;
    case GLFW_KEY_D: m_keyD = false; break;
    case GLFW_KEY_Q: m_keyQ = false; break;
    case GLFW_KEY_E: m_keyE = false; break;
    case GLFW_KEY_LEFT_SHIFT:
    case GLFW_KEY_RIGHT_SHIFT:
        m_keyShift = false; break;
    }
}
