#pragma once
#include "DD_WinApplication.h"
#include "framework.h"

using Microsoft::WRL::ComPtr;

// DD_Engine -----------------------------------------------------------------------------

class DD_ENGINE_API DD_Core : public DXSample
{
protected:
	virtual void OnInit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;
	virtual void OnDestroy() override;
	virtual void OnResize(UINT width, UINT height) override;
public:
	DD_Core(UINT width, UINT height, std::wstring name);
	virtual ~DD_Core();

private:
	std::vector<class ISystem*> m_systems;
};
