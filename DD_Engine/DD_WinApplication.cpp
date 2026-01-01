#include "DD_WinApplication.h"
#include "DD_GLDevice.h"
#include <stdio.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

AppBase* DD_Application::s_app = nullptr;

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

int DD_Application::Run(AppBase* app)
{
    s_app = app;

    app->OnInit();

#ifdef __EMSCRIPTEN__
    // Emscripten: 프레임 콜백 등록
    emscripten_set_main_loop(MainLoop, 0, 1);
#else
    // 네이티브: 일반 루프
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
    if (s_app)
    {
        s_app->OnUpdate();
        s_app->OnRender();
    }
}

void DD_Application::OnResize()
{
    // GLFW 콜백에서 호출될 리사이즈 핸들러 (필요 시 구현)
}
