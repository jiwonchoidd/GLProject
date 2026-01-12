#include "DD_Camera.h"
#include "DD_GLHelper.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

DD_Camera::DD_Camera()
    : m_fov(80.0f * 3.14159265358979323846f / 180.0f)
{
    m_transform.position = Vec3(0.0f, 0.0f, 10.0f);
    UpdateView();
}

DD_Camera::~DD_Camera()
{
}

void DD_Camera::UpdateProjection(int width, int height)
{
    float aspect = static_cast<float>(width) / static_cast<float>(height);
    m_projection = glm::perspective(m_fov, aspect, 0.1f, 1000.0f);
}

void DD_Camera::UpdateView()
{
    Quaternion q = GetQuat(m_transform);
    glm::vec3 forward = q * glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 pos = glm::vec3(m_transform.position.x, m_transform.position.y, m_transform.position.z);
    glm::vec3 target = pos + forward;
    m_view = glm::lookAt(pos, target, glm::vec3(0.f, 1.f, 0.f));
}