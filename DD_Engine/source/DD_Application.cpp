#include "DD_Application.h"
#include "DD_GLDevice.h"
#include <stdio.h>

AppBase* g_app = nullptr;

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#endif


AppBase::AppBase(int width, int height, const char* name)
    : m_width(width)
    , m_height(height)
    , m_title(name)
{
    m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

AppBase::~AppBase()
{
}

void AppBase::DispatchKeyEvent(int key, int action)
{
    for (auto& callback : m_keyCallbacks)
    {
        callback(key, action);
    }
}

void AppBase::DispatchMouseButton(int button, int action, int x, int y)
{
    for (auto& callback : m_mouseButtonCallbacks)
    {
        callback(button, action, x, y);
    }
}

void AppBase::DispatchMouseMove(int x, int y)
{
    for (auto& callback : m_mouseMoveCallbacks)
    {
        callback(x, y);
    }
}

void AppBase::DispatchScroll(float delta)
{
    for (auto& callback : m_scrollCallbacks)
    {
        callback(delta);
    }
}

#ifdef __EMSCRIPTEN__

EM_BOOL touch_start_callback(int eventType, const EmscriptenTouchEvent* touchEvent, void* userData)
{
    if (g_app && touchEvent->numTouches > 0)
    {
        int x = touchEvent->touches[0].clientX;
        int y = touchEvent->touches[0].clientY;
        g_app->OnTouchStart(x, y);
        g_app->DispatchMouseButton(0, 1, x, y);
    }
    return EM_TRUE;
}

EM_BOOL touch_end_callback(int eventType, const EmscriptenTouchEvent* touchEvent, void* userData)
{
    if (g_app && touchEvent->numTouches > 0)
    {
        int x = touchEvent->touches[0].clientX;
        int y = touchEvent->touches[0].clientY;
        g_app->OnTouchEnd(x, y);
        g_app->DispatchMouseButton(0, 0, x, y);
    }
    return EM_TRUE;
}

EM_BOOL resize_callback(int eventType, const EmscriptenUiEvent* uiEvent, void* userData)
{
    if (g_app)
    {
        int width = uiEvent->windowInnerWidth;
        int height = uiEvent->windowInnerHeight;
        g_app->OnResize(width, height);
    }
    return EM_TRUE;
}

EM_BOOL mouse_move_callback(int eventType, const EmscriptenMouseEvent* mouseEvent, void* userData)
{
    if (g_app)
    {
        g_app->DispatchMouseMove(mouseEvent->clientX, mouseEvent->clientY);
    }
    return EM_TRUE;
}

EM_BOOL wheel_callback(int eventType, const EmscriptenWheelEvent* wheelEvent, void* userData)
{
    if (g_app)
    {
        g_app->DispatchScroll(static_cast<float>(-wheelEvent->deltaY));
    }
    return EM_TRUE;
}

EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData)
{
    if (g_app)
    {
        int action = (eventType == EMSCRIPTEN_EVENT_KEYDOWN) ? 1 : 0;
        g_app->DispatchKeyEvent(keyEvent->keyCode, action);
    }
    return EM_TRUE;
}

#else

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (g_app && (action == GLFW_PRESS || action == GLFW_RELEASE))
    {
        g_app->DispatchKeyEvent(key, action);
    }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (g_app)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int x = static_cast<int>(xpos);
        int y = static_cast<int>(ypos);

        g_app->DispatchMouseButton(button, action, x, y);

        if (action == GLFW_PRESS)
        {
            g_app->OnTouchStart(x, y);
        }
        else if (action == GLFW_RELEASE)
        {
            g_app->OnTouchEnd(x, y);
        }
    }
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (g_app)
    {
        g_app->DispatchMouseMove(static_cast<int>(xpos), static_cast<int>(ypos));
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (g_app)
    {
        g_app->DispatchScroll(static_cast<float>(yoffset));
    }
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    if (g_app)
    {
        g_app->OnResize(width, height);
    }
}
#endif

int DD_Application::Run(AppBase* app)
{
    g_app = app;

    app->OnInit();

#ifdef __EMSCRIPTEN__
    emscripten_set_touchstart_callback("#canvas", nullptr, EM_TRUE, touch_start_callback);
    emscripten_set_touchend_callback("#canvas", nullptr, EM_TRUE, touch_end_callback);

    emscripten_set_resize_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_FALSE, resize_callback);
    emscripten_set_mousemove_callback("#canvas", nullptr, EM_TRUE, mouse_move_callback);
    emscripten_set_wheel_callback("#canvas", nullptr, EM_TRUE, wheel_callback);
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, key_callback);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, nullptr, EM_TRUE, key_callback);

    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    GLFWwindow* window = gGLDevice.GetWindow();
    if (window)
    {
        glfwSetKeyCallback(window, key_callback);
        glfwSetMouseButtonCallback(window, mouse_button_callback);
        glfwSetCursorPosCallback(window, cursor_pos_callback);
        glfwSetScrollCallback(window, scroll_callback);
        glfwSetWindowSizeCallback(window, window_size_callback);
    }

    while (!gGLDevice.ShouldClose())
    {
        MainLoop();
    }
#endif

    app->OnDestroy();

    return 0;
}

void DD_Application::MainLoop()
{
    if (g_app)
    {
        g_app->OnUpdate();
        g_app->OnRender();
    }
}