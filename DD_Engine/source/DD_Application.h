#pragma once
#include "framework.h"
#include <functional>
#include <vector>

// Input callback types
using KeyCallback = std::function<void(int key, int action)>;
using MouseButtonCallback = std::function<void(int button, int action, int x, int y)>;
using MouseMoveCallback = std::function<void(int x, int y)>;
using ScrollCallback = std::function<void(float delta)>;

class DD_ENGINE_API AppBase
{
public:
    AppBase(int width, int height, const char* name);
    virtual ~AppBase();
public:
    virtual void OnInit() = 0;
    virtual void OnUpdate() = 0;
    virtual void OnRender() = 0;
    virtual void OnDestroy() = 0;

    virtual void OnTouchStart(int x, int y) {}
    virtual void OnTouchEnd(int x, int y) {}
    virtual void OnResize(int w, int h) {}

    // Input event dispatchers (called by platform layer)
    void DispatchKeyEvent(int key, int action);
    void DispatchMouseButton(int button, int action, int x, int y);
    void DispatchMouseMove(int x, int y);
    void DispatchScroll(float delta);

    // Register callbacks
    void AddKeyCallback(KeyCallback callback) { m_keyCallbacks.push_back(callback); }
    void AddMouseButtonCallback(MouseButtonCallback callback) { m_mouseButtonCallbacks.push_back(callback); }
    void AddMouseMoveCallback(MouseMoveCallback callback) { m_mouseMoveCallbacks.push_back(callback); }
    void AddScrollCallback(ScrollCallback callback) { m_scrollCallbacks.push_back(callback); }

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    const char* GetTitle() const { return m_title; }

protected:
    int m_width;
    int m_height;
    float m_aspectRatio;

private:
    const char* m_title;

    std::vector<KeyCallback> m_keyCallbacks;
    std::vector<MouseButtonCallback> m_mouseButtonCallbacks;
    std::vector<MouseMoveCallback> m_mouseMoveCallbacks;
    std::vector<ScrollCallback> m_scrollCallbacks;
};

class DD_ENGINE_API DD_Application
{
public:
    static int Run(AppBase* app);
    static void MainLoop();
};
