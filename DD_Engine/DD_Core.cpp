#include "DD_Core.h"
#include "DD_Device.h"
#include "DD_WinApplication.h"
#include "DD_TextureMananger.h"
#include "DD_UIMananger.h"
#include "DD_SimpleBox.h"
#include "DD_DXHelper.h"

using namespace Microsoft::WRL;

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
ComPtr<ID3D11Buffer> g_pCBNeverChanges;
ComPtr<ID3D11Buffer> g_pCBChangeOnResize;

XMMATRIX g_View;
XMMATRIX g_Projection;

struct CBNeverChanges
{
	XMMATRIX mView;
};

struct CBChangeOnResize
{
	XMMATRIX mProjection;
};

std::vector<std::wstring> g_texture;
std::vector<DD_SimpleBox*> g_box;

// DD_Engine ----------------------------------------------------------------------------

void DD_Core::OnInit()
{
	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
	{
		return;
	}

	m_systems.push_back(DD_Device::GetInstance());
	m_systems.push_back(DD_UIMananger::GetInstance());
	m_systems.push_back(DD_TextureMananger::GetInstance());

	gDevice.CreateDevice(DD_WinApplication::GetHwnd());

	for (const auto& system : m_systems)
	{
		system->Initialize();
	}

	gDevice.GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Create the constant buffers
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBNeverChanges);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bd.CPUAccessFlags = 0;
	hr = gDevice.GetDevice()->CreateBuffer(&bd, NULL, &g_pCBNeverChanges);
	if (FAILED(hr))
		return;

	bd.ByteWidth = sizeof(CBChangeOnResize);
	hr = gDevice.GetDevice()->CreateBuffer(&bd, NULL, &g_pCBChangeOnResize);
	if (FAILED(hr))
		return;

	XMVECTOR Eye = XMVectorSet(0.0f, 10.0f, -10.0f, 0.0f);
	XMVECTOR At = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	XMVECTOR Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	g_View = XMMatrixLookAtLH(Eye, At, Up);

	CBNeverChanges cbNeverChanges;
	cbNeverChanges.mView = XMMatrixTranspose(g_View);
	gDevice.GetDeviceContext()->UpdateSubresource(g_pCBNeverChanges.Get(), 0, NULL, &cbNeverChanges, 0, 0);

	std::vector<std::wstring> imgList;
	if (DD_DXHelper::FindDirectoryItem(L"", L".jpg", imgList))
	{
		g_texture = imgList;
	}

	DD_SimpleBox::CachePipline();
	int32_t maxBoxCount = 10000;
	for (int32_t i = 0; i < maxBoxCount; ++i)
	{
		DD_SimpleBox* box = new DD_SimpleBox();
		box->Create({ 0.f,0.f, 0.f });
		g_box.push_back(box);
	}
}

void DD_Core::OnUpdate()
{
	using clock = std::chrono::steady_clock;

	static double elapsedTime = 0;
	static auto prevTime = clock::now();
	auto currentTime = clock::now();

	double deltaTime = std::chrono::duration<double>(currentTime - prevTime).count();
	prevTime = currentTime;
	elapsedTime += deltaTime;

	for (const auto& box : g_box)
	{
		box->AddPos({ 0.f, static_cast<float>(0.5f * deltaTime), 0.f });
	}

	for (const auto& system : m_systems)
	{
		system->Tick(deltaTime);
	}
}

void DD_Core::OnRender()
{
	gDevice.PreRender();
	gUIMng.PreRender();

	{
		ID3D11Buffer* cbs[] = { g_pCBNeverChanges.Get(), g_pCBChangeOnResize.Get() };
		gDevice.GetDeviceContext()->VSSetConstantBuffers(0, 1, &cbs[0]);
		gDevice.GetDeviceContext()->VSSetConstantBuffers(1, 1, &cbs[1]);

		for (const auto& box : g_box)
		{
			box->Render();
		}
	}
	gUIMng.PostRender();
	gDevice.PostRender();
}

void DD_Core::OnDestroy()
{
	for (auto& box : g_box)
	{
		if (!box) continue;

		delete box;
		box = nullptr;
	}
	g_box.clear();

	DD_SimpleBox::ClearPipline();

	g_pCBNeverChanges.Reset();
	g_pCBChangeOnResize.Reset();

	for (const auto& system : m_systems)
	{
		system->Finalize();
	}
	CoUninitialize();
}

void DD_Core::OnResize(UINT width, UINT height)
{
	m_width = width;
	m_height = height;

	g_Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(80.0f), GetWidth() / (FLOAT)GetHeight(), 0.1f, 1000.0f);
	CBChangeOnResize cbChangesOnResize;
	cbChangesOnResize.mProjection = XMMatrixTranspose(g_Projection);
	gDevice.GetDeviceContext()->UpdateSubresource(g_pCBChangeOnResize.Get(), 0, NULL, &cbChangesOnResize, 0, 0);
	gDevice.Resize(GetWidth(), GetHeight());
}

DD_Core::DD_Core(UINT width, UINT height, std::wstring name) : DXSample(width, height, name)
{
}

DD_Core::~DD_Core()
{
}