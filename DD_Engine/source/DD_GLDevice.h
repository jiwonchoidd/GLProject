#pragma once
#include "DD_Core.h"
#include "DD_GLHelper.h"
#include "framework.h"

class DD_GLDevice : public ISingleton<DD_GLDevice>
{
public:
	virtual void Initialize() override;
	virtual void Tick(float deltaTime) override;
	virtual void Finalize() override;

public:
	bool CreateDevice(int width, int height);
	void Resize(int width, int height);
	void SetViewport(int width, int height);

private:
	void DestroyDevice();

public:
	void PreRender();
	void Render();
	void PostRender();
	
	bool ShouldClose();
	GLFWwindow* GetWindow() const { return m_window; }
	
	// Shader management
	bool CompileShaders();
	GLuint GetProgram() const { return m_program; }
	
	// Uniform locations
	GLint GetWorldLocation() const { return m_worldLoc; }
	GLint GetViewLocation() const { return m_viewLoc; }
	GLint GetProjectionLocation() const { return m_projLoc; }
	GLint GetColorLocation() const { return m_colorLoc; }

private:
	GLuint CompileShader(const char* source, GLenum type);
	GLuint LinkProgram(GLuint vs, GLuint fs);

private:
	GLFWwindow* m_window;
	GLuint m_program;
	
	GLint m_worldLoc;
	GLint m_viewLoc;
	GLint m_projLoc;
	GLint m_colorLoc;
	
	int m_width;
	int m_height;
	bool m_initialized;
};

#define gGLDevice (*DD_GLDevice::GetInstance())
