#pragma once
#include "framework.h"

class DD_ENGINE_API AppBase
{
public:
	AppBase(int width, int height, const char* name);
	virtual ~AppBase();
public:
	virtual void OnInit() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnRender() = 0;
	virtual void OnDestroy() = 0;

	virtual void OnKeyDown(unsigned char /*key*/) {}
	virtual void OnKeyUp(unsigned char /*key*/) {}
	virtual void OnResize(int w, int h) {}

	// Accessors
	int GetWidth() const { return m_width; }
	int GetHeight() const { return m_height; }
	const char* GetTitle() const { return m_title; }

protected:
	int m_width;
	int m_height;
	float m_aspectRatio;

private:
	const char* m_title;
};

// GLFW-based application (works on desktop and web with Emscripten)
class DD_ENGINE_API DD_Application
{
public:
    static int Run(AppBase* app);
    static void MainLoop();
    static void OnResize();
    
private:
    static AppBase* s_app;
};
