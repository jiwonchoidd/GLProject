#include "DD_GLDevice.h"
#include <cstdio>

#ifndef PLATFORM_WEB
#include <GL/glew.h>
#endif

#ifdef PLATFORM_WEB
static const char* vertexShaderSource = R"(
#version 300 es
precision highp float;

in vec3 aPosition;
in vec2 aTexCoord;

uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 vTexCoord;

void main()
{
    vec4 worldPos = uWorld * vec4(aPosition, 1.0);
    vec4 viewPos = uView * worldPos;
    gl_Position = uProjection * viewPos;
    vTexCoord = aTexCoord;
}
)";

static const char* fragmentShaderSource = R"(
#version 300 es
precision mediump float;

in vec2 vTexCoord;
uniform vec4 uColor;

out vec4 fragColor;

void main()
{
    fragColor = uColor;
}
)";
#else
// Desktop OpenGL 3.3 Core
static const char* vertexShaderSource = R"(
#version 330 core

in vec3 aPosition;
in vec2 aTexCoord;

uniform mat4 uWorld;
uniform mat4 uView;
uniform mat4 uProjection;

out vec2 vTexCoord;

void main()
{
    vec4 worldPos = uWorld * vec4(aPosition, 1.0);
    vec4 viewPos = uView * worldPos;
    gl_Position = uProjection * viewPos;
    vTexCoord = aTexCoord;
}
)";

static const char* fragmentShaderSource = R"(
#version 330 core

in vec2 vTexCoord;
uniform vec4 uColor;

out vec4 fragColor;

void main()
{
    fragColor = uColor;
}
)";
#endif

void DD_GLDevice::Initialize()
{
	// Singleton 초기화 - 멤버 변수만 초기화
	m_initialized = false;
	m_program = 0;
	m_width = 1280;
	m_height = 720;
	m_window = nullptr;
	m_worldLoc = -1;
	m_viewLoc = -1;
	m_projLoc = -1;
	m_colorLoc = -1;
	
	printf("DD_GLDevice::Initialize() - members initialized\n");
}

void DD_GLDevice::Tick(float deltaTime)
{
	// Nothing to update per frame
}

void DD_GLDevice::Finalize()
{
	DestroyDevice();
}

bool DD_GLDevice::CreateDevice(int width, int height)
{
	m_width = width;
	m_height = height;
	
	// Initialize GLFW
	if (!glfwInit()) {
		printf("Failed to initialize GLFW\n");
		return false;
	}
	
#ifdef PLATFORM_WEB
	// GLFW window hints for OpenGL ES 3.0 (WebGL 2.0)
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#else
	// Desktop OpenGL 3.3 Core
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x MSAA
	
	// Create window
	m_window = glfwCreateWindow(width, height, "DD_Engine", nullptr, nullptr);
	if (!m_window) {
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return false;
	}
	
	glfwMakeContextCurrent(m_window);
	
#ifndef PLATFORM_WEB
	// Initialize GLEW (Desktop only)
	glewExperimental = GL_TRUE;
	GLenum err = glewInit();
	if (err != GLEW_OK) {
		printf("Failed to initialize GLEW: %s\n", glewGetErrorString(err));
		glfwDestroyWindow(m_window);
		glfwTerminate();
		return false;
	}
	printf("Running on Desktop - GLEW %s\n", glewGetString(GLEW_VERSION));
#else
	printf("Running on Web (Emscripten)\n");
#endif
	
	printf("GL Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	
	// OpenGL state setup
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	
	SetViewport(width, height);
	
	// Compile shaders
	if (!CompileShaders()) {
		printf("Failed to compile shaders\n");
		return false;
	}
	
	m_initialized = true;
	printf("DD_GLDevice initialized successfully\n");
	return true;
}

void DD_GLDevice::DestroyDevice()
{
	if (m_program) {
		glDeleteProgram(m_program);
		m_program = 0;
	}
	
	if (m_window) {
		glfwDestroyWindow(m_window);
		m_window = nullptr;
	}
	
	glfwTerminate();
	m_initialized = false;
}

void DD_GLDevice::Resize(int width, int height)
{
	m_width = width;
	m_height = height;
	SetViewport(width, height);
}

void DD_GLDevice::SetViewport(int width, int height)
{
	glViewport(0, 0, width, height);
}

void DD_GLDevice::PreRender()
{
	glClearColor(0.0f, 0.125f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if (m_program) {
		glUseProgram(m_program);
	}
}

void DD_GLDevice::Render()
{
	// Main rendering happens in SimpleBox::Render()
}

void DD_GLDevice::PostRender()
{
	if (!m_window)
	{
		printf("Error: m_window is null in PostRender()\n");
		return;
	}
	
	glfwSwapBuffers(m_window);
	glfwPollEvents();
}

bool DD_GLDevice::ShouldClose()
{
	if (!m_window)
	{
		printf("Warning: m_window is null in ShouldClose()\n");
		return true;
	}
	return glfwWindowShouldClose(m_window);
}

GLuint DD_GLDevice::CompileShader(const char* source, GLenum type)
{
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, nullptr);
	glCompileShader(shader);
	
	GLint success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(shader, 512, nullptr, infoLog);
		printf("Shader compilation error (%s):\n%s\n", 
			   type == GL_VERTEX_SHADER ? "vertex" : "fragment", infoLog);
		glDeleteShader(shader);
		return 0;
	}
	
	return shader;
}

GLuint DD_GLDevice::LinkProgram(GLuint vs, GLuint fs)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, fs);
	glLinkProgram(program);
	
	GLint success;
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(program, 512, nullptr, infoLog);
		printf("Program link error:\n%s\n", infoLog);
		glDeleteProgram(program);
		return 0;
	}
	
	return program;
}

bool DD_GLDevice::CompileShaders()
{
	GLuint vs = CompileShader(vertexShaderSource, GL_VERTEX_SHADER);
	if (!vs) return false;
	
	GLuint fs = CompileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
	if (!fs) {
		glDeleteShader(vs);
		return false;
	}
	
	m_program = LinkProgram(vs, fs);
	
	glDeleteShader(vs);
	glDeleteShader(fs);
	
	if (!m_program) return false;
	
	// Get uniform locations
	m_worldLoc = glGetUniformLocation(m_program, "uWorld");
	m_viewLoc = glGetUniformLocation(m_program, "uView");
	m_projLoc = glGetUniformLocation(m_program, "uProjection");
	m_colorLoc = glGetUniformLocation(m_program, "uColor");
	
	printf("Shader uniforms: world=%d, view=%d, proj=%d, color=%d\n",
		   m_worldLoc, m_viewLoc, m_projLoc, m_colorLoc);
	
	return true;
}
