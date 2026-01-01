#pragma once
#include "DD_WinApplication.h"
#include "framework.h"

class DD_ENGINE_API DD_Core : public AppBase
{
protected:
	virtual void OnInit() override;
	virtual void OnUpdate() override;
	virtual void OnRender() override;
	virtual void OnDestroy() override;
	virtual void OnResize(int width, int height) override;
public:
	DD_Core(int width, int height, const char* name);
	virtual ~DD_Core();

private:
	std::vector<class ISystem*> m_systems;
};
