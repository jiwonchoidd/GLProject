#include "DD_Device.h"

void DD_Device::Initialize()
{
}

void DD_Device::Tick(float deltaTime)
{
}

void DD_Device::Finalize()
{
	DestroyDevice();
}

HRESULT DD_Device::CreateDevice(HWND winHandle)
{
	HRESULT hr = S_OK;

	RECT rc;
	GetClientRect(winHandle, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#ifdef _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	ZeroMemory(&m_swapChainDesc, sizeof(m_swapChainDesc));
	m_swapChainDesc.BufferCount = 1;
	m_swapChainDesc.BufferDesc.Width = width;
	m_swapChainDesc.BufferDesc.Height = height;
	m_swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	m_swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	m_swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	m_swapChainDesc.OutputWindow = winHandle;
	m_swapChainDesc.SampleDesc.Count = 1;
	m_swapChainDesc.SampleDesc.Quality = 0;
	m_swapChainDesc.Windowed = TRUE;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		m_driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			NULL,
			m_driverType,
			NULL,
			createDeviceFlags,
			featureLevels,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&m_swapChainDesc,
			&m_swapChain,
			&m_device,
			&m_featureLevel,
			&m_deviceContext);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	hr = m_device->CreateRenderTargetView(pBackBuffer, NULL, &m_renderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
		return hr;

	// Create depth stencil texture
	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = m_device->CreateTexture2D(&descDepth, NULL, &m_depthStencil);
	if (FAILED(hr))
		return hr;

	// Create the depth stencil view
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;
	hr = m_device->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, &m_depthStencilView);
	if (FAILED(hr))
		return hr;

	ID3D11RenderTargetView* rtvs[] =
	{
		m_renderTargetView.Get()
	};

	m_deviceContext->OMSetRenderTargets(1, rtvs, m_depthStencilView.Get());

	SetViewport(width, height);

	return hr;
}

void DD_Device::Resize(UINT width, UINT height)
{
	if (!m_device || !m_deviceContext || !m_swapChain)
		return;

	ID3D11RenderTargetView* nullRTV[1] = { nullptr };
	m_deviceContext->OMSetRenderTargets(1, nullRTV, nullptr);

	m_renderTargetView.Reset();
	m_depthStencilView.Reset();
	m_depthStencil.Reset();

	HRESULT hr = m_swapChain->ResizeBuffers(
		m_swapChainDesc.BufferCount,
		width,
		height,
		m_swapChainDesc.BufferDesc.Format,
		0);
	if (FAILED(hr))
	{
		return;
	}

	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
	{
		return;
	}

	hr = m_device->CreateRenderTargetView(pBackBuffer, NULL, &m_renderTargetView);
	pBackBuffer->Release();
	if (FAILED(hr))
	{
		return;
	}

	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = m_swapChainDesc.SampleDesc.Count;
	descDepth.SampleDesc.Quality = m_swapChainDesc.SampleDesc.Quality;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;

	hr = m_device->CreateTexture2D(&descDepth, NULL, &m_depthStencil);
	if (FAILED(hr))
	{
		return;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	hr = m_device->CreateDepthStencilView(m_depthStencil.Get(), &descDSV, &m_depthStencilView);
	if (FAILED(hr))
	{
		return;
	}

	ID3D11RenderTargetView* rtvs[] = { m_renderTargetView.Get() };
	m_deviceContext->OMSetRenderTargets(1, rtvs, m_depthStencilView.Get());

	SetViewport(width, height);
}

void DD_Device::SetViewport(UINT width, UINT height)
{
	m_viewPort.TopLeftX = 0.0f;
	m_viewPort.TopLeftY = 0.0f;
	m_viewPort.Width = static_cast<FLOAT>(width);
	m_viewPort.Height = static_cast<FLOAT>(height);
	m_viewPort.MinDepth = 0.0f;
	m_viewPort.MaxDepth = 1.0f;

	if (m_deviceContext)
	{
		m_deviceContext->RSSetViewports(1, &m_viewPort);
	}
}

void DD_Device::DestroyDevice()
{
	if (m_deviceContext)
	{
		ID3D11RenderTargetView* nullRTV[1] = { nullptr };
		m_deviceContext->OMSetRenderTargets(1, nullRTV, nullptr);
		m_deviceContext->ClearState();
		m_deviceContext->Flush();
	}

	// Important: Set windowed mode before releasing swap chain
	if (m_swapChain)
	{
		m_swapChain->SetFullscreenState(FALSE, nullptr);
	}

	m_renderTargetView.Reset();
	m_depthStencilView.Reset();
	m_depthStencil.Reset();
	m_swapChain.Reset();
	m_deviceContext.Reset();

#ifdef _DEBUG
	//if (m_device)
	//{
	//	ID3D11Debug* pDebug = nullptr;
	//	OutputDebugStringW(L"Starting Live Direct3D Object Dump:\r\n");
	//	if (SUCCEEDED(m_device->QueryInterface(__uuidof(ID3D11Debug), (void**)&pDebug)) && pDebug)
	//	{
	//		pDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);
	//		pDebug->Release();
	//	}
	//	OutputDebugStringW(L"Ending Live Direct3D Object Dump:\r\n");
	//}
#endif

	m_device.Reset();
}

void DD_Device::PreRender()
{
	static float ClearColor[4] = { 0.68f, 0.68f, 0.7f, 1.0f }; // red, green, blue, alpha
	m_deviceContext->ClearRenderTargetView(m_renderTargetView.Get(), ClearColor);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void DD_Device::Render()
{
}

void DD_Device::PostRender()
{
	m_swapChain->Present(0, 0);
}