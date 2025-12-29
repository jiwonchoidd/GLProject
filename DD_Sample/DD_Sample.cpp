#pragma once
#include <windows.h>
#include "../DD_Engine/DD_WinApplication.h"
#include "../DD_Engine/DD_Core.h"

_Use_decl_annotations_
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    DD_Core sample(1280, 720, L"Test Resource");
    return DD_WinApplication::Run(&sample, hInstance, nCmdShow);
}