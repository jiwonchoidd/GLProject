#include "../DD_Engine/DD_Core.h"
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    DD_Core app(1280, 720, "DD_Engine Desktop");
    return DD_Application::Run(&app);
}
