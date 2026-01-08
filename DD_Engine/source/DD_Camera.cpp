#include "DD_Camera.h"
#include "DD_GLHelper.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

DD_Camera::DD_Camera()
    : m_position(0.0f, 10.0f, -10.0f)
    , m_target(0.0f, 0.0f, 0.0f)
    , m_up(0.0f, 1.0f, 0.0f)
    , m_fov(80.0f * 3.14159265358979323846f / 180.0f)
{
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
    m_view = glm::lookAt(m_position, m_target, m_up);
}