#pragma once
#include "DD_Core.h"
#include "DD_DXHelper.h"

using Microsoft::WRL::ComPtr;

class DD_Device : public ISingleton<DD_Device>
{
public:
	virtual void Initialize() override;
	virtual void Tick(float deltaTime) override;
	virtual void Finalize() override;

public:
	HRESULT CreateDevice(HWND winHandle);
	ID3D11Device* GetDevice() { return m_device.Get(); }
	ID3D11DeviceContext* GetDeviceContext() { return m_deviceContext.Get(); }

	void Resize(UINT width, UINT height);
	void SetViewport(UINT width, UINT height);

private:
	void DestroyDevice();

public:
	void PreRender();
	void Render();
	void PostRender();

private:
	ComPtr<ID3D11Device>		m_device;
	ComPtr<ID3D11DeviceContext> m_deviceContext;
	ComPtr<IDXGISwapChain>		m_swapChain;
	ComPtr<ID3D11RenderTargetView> m_renderTargetView;
	ComPtr<ID3D11Texture2D>			m_depthStencil;
	ComPtr<ID3D11DepthStencilView> m_depthStencilView;

	DXGI_SWAP_CHAIN_DESC		m_swapChainDesc;
	D3D11_VIEWPORT				m_viewPort;
	D3D_DRIVER_TYPE				m_driverType = D3D_DRIVER_TYPE_NULL;
	D3D_FEATURE_LEVEL			m_featureLevel = D3D_FEATURE_LEVEL_11_0;
};

#define gDevice (*DD_Device::GetInstance())
