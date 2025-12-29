#include "DD_UIMananger.h"
#include "DD_WinApplication.h"
#include "DD_Device.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx11.h"
#include "DD_TextureMananger.h"

void DD_UIMananger::Initialize()
{
    ImGui_ImplWin32_EnableDpiAwareness();

    const HWND hwnd = DD_WinApplication::GetHwnd();
    const float dpiScale = ImGui_ImplWin32_GetDpiScaleForHwnd(hwnd);

    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(1.0f);

    RECT rc{}; GetClientRect(hwnd, &rc);
    io.DisplaySize = ImVec2(float(rc.right - rc.left), float(rc.bottom - rc.top));
    io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

    io.Fonts->AddFontDefault();
    ///io.Fonts->AddFontFromFileTTF("assets\\font.ttf", 16.0f * dpiScale);

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(gDevice.GetDevice(), gDevice.GetDeviceContext());
}

void DD_UIMananger::Tick(float deltaTime)
{
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    float msPerFrame = deltaTime * 1000.0f;
    float fps = (deltaTime > 0.0f) ? (1.0f / deltaTime) : 0.0f;

    ImGui::Begin("stat");
    ImGui::Text("average %.3f ms/frame (%.1f FPS)", msPerFrame, fps);
    ImGui::End();

    std::vector<std::wstring> infos =  gTextureMng.GetOriginInfo();

    ImGui::Begin("Texture Manager");
    ImGui::Text("Loaded Textures");
    ImGui::Separator();

    if (ImGui::BeginTable("TextureTable", 1, ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollY))
    {
        ImGui::TableSetupColumn("Info", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        for (const auto& info : infos)
        {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%ls", info.c_str());
        }

        ImGui::EndTable();
    }

    ImGui::End();
}

void DD_UIMananger::Finalize()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void DD_UIMananger::PreRender()
{
}

void DD_UIMananger::PostRender()
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}
