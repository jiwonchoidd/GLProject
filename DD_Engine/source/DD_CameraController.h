#pragma once
#include "DD_Transform.h"

class DD_Camera;

class DD_CameraController
{
public:
    DD_CameraController(DD_Camera* camera);

    void Update(float deltaTime);

    void OnPointerDown(int x, int y);
    void OnPointerUp(int x, int y);
    void OnPointerMove(int x, int y);
    void OnScroll(float delta);
    void OnKeyDown(int key);
    void OnKeyUp(int key);

    // Movement settings
    void SetMoveSpeed(float speed) { m_moveSpeed = speed; }
    float GetMoveSpeed() const { return m_moveSpeed; }

private:
    DD_Camera* m_camera;
    bool m_dragging = false;
    int m_lastX = 0;
    int m_lastY = 0;
    float m_sensitivity = 0.003f;
    float m_zoomSpeed = 0.5f;
    float m_moveSpeed = 10.0f;

    // Key states
    bool m_keyW = false;
    bool m_keyA = false;
    bool m_keyS = false;
    bool m_keyD = false;
    bool m_keyQ = false;  // Down
    bool m_keyE = false;  // Up
    bool m_keyShift = false;  // Speed boost
};
