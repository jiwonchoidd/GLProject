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

#ifdef __EMSCRIPTEN__

EM_BOOL touch_start_callback(int eventType, const EmscriptenTouchEvent* touchEvent, void* userData)
{
    if (g_app && touchEvent->numTouches > 0)
    {
        int x = touchEvent->touches[0].clientX;
        int y = touchEvent->touches[0].clientY;
        g_app->OnTouchStart(x, y);
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
#else

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    if (g_app)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        int x = static_cast<int>(xpos);
        int y = static_cast<int>(ypos);

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

    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    GLFWwindow* window = gGLDevice.GetWindow();
    if (window)
    {
        glfwSetMouseButtonCallback(window, mouse_button_callback);
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