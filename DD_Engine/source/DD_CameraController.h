#pragma once
#include "DD_Transform.h"

class DD_Camera;

class DD_CameraController
{
public:
    DD_CameraController(DD_Camera* camera);

    void OnPointerDown(int x, int y);
    void OnPointerUp(int x, int y);
    void OnPointerMove(int x, int y);
    void OnScroll(float delta);

private:
    DD_Camera* m_camera;
    bool m_dragging = false;
    int m_lastX = 0;
    int m_lastY = 0;
    float m_sensitivity = 0.001f;
    float m_zoomSpeed = 0.1f;
};
