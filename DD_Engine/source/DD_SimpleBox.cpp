#include "DD_SimpleBox.h"
#include "DD_GLDevice.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct SimpleVertex
{
	glm::vec3 Pos;
	glm::vec2 Tex;
};

namespace
{
	GLuint s_shaderProgram = 0;
	GLint s_modelLoc = -1;
	GLint s_viewLoc = -1;
	GLint s_projLoc = -1;
	bool s_initialized = false;
}

bool DD_SimpleBox::CachePipline()
{
	if (s_initialized) return true;

#ifdef __EMSCRIPTEN__
	const char* vertexShaderSource = 
		"#version 300 es\n"
		"precision highp float;\n"
		"layout (location = 0) in vec3 aPos;\n"
		"layout (location = 1) in vec2 aTexCoord;\n"
		"\n"
		"uniform mat4 uModel;\n"
		"uniform mat4 uView;\n"
		"uniform mat4 uProjection;\n"
		"\n"
		"out vec2 TexCoord;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);\n"
		"    TexCoord = aTexCoord;\n"
		"}\n";

	const char* fragmentShaderSource = 
		"#version 300 es\n"
		"precision highp float;\n"
		"in vec2 TexCoord;\n"
		"out vec4 FragColor;\n"
		"\n"
		"void main()\n"
		"{\n"
		"    FragColor = vec4(TexCoord, 0.5, 1.0);\n"
		"}\n";
#else
	// Desktop OpenGL 3.3 Core
	const char* vertexShaderSource = R"(
		#version 330 core
		layout (location = 0) in vec3 aPos;
		layout (location = 1) in vec2 aTexCoord;
		
		uniform mat4 uModel;
		uniform mat4 uView;
		uniform mat4 uProjection;
		
		out vec2 TexCoord;
		
		void main()
		{
			gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
			TexCoord = aTexCoord;
		}
	)";

	const char* fragmentShaderSource = R"(
		#version 330 core
		in vec2 TexCoord;
		out vec4 FragColor;
		
		void main()
		{
			FragColor = vec4(TexCoord, 0.5, 1.0);
		}
	)";
#endif

	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	
	// Check vertex shader compilation
	GLint success;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
		printf("Vertex shader compilation failed:\n%s\n", infoLog);
		return false;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	
	// Check fragment shader compilation
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
		printf("Fragment shader compilation failed:\n%s\n", infoLog);
		glDeleteShader(vertexShader);
		return false;
	}

	s_shaderProgram = glCreateProgram();
	glAttachShader(s_shaderProgram, vertexShader);
	glAttachShader(s_shaderProgram, fragmentShader);
	glLinkProgram(s_shaderProgram);
	
	// Check program linking
	glGetProgramiv(s_shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		char infoLog[512];
		glGetProgramInfoLog(s_shaderProgram, 512, nullptr, infoLog);
		printf("Shader program linking failed:\n%s\n", infoLog);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return false;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	s_modelLoc = glGetUniformLocation(s_shaderProgram, "uModel");
	s_viewLoc = glGetUniformLocation(s_shaderProgram, "uView");
	s_projLoc = glGetUniformLocation(s_shaderProgram, "uProjection");
	
	printf("SimpleBox shader program created successfully (model=%d, view=%d, proj=%d)\n", s_modelLoc, s_viewLoc, s_projLoc);

	s_initialized = true;
	return true;
}

void DD_SimpleBox::ClearPipline()
{
	if (s_shaderProgram) glDeleteProgram(s_shaderProgram);
	s_shaderProgram = 0;
	s_initialized = false;
}

void DD_SimpleBox::Create(const Vec3& pos)
{
	m_position = pos;

	// Cube vertices (8 vertices)
	SimpleVertex vertices[] = {
		// Front face (Z+)
		{ glm::vec3(-1.0f, -1.0f,  1.0f), glm::vec2(0.0f, 0.0f) }, // 0
		{ glm::vec3( 1.0f, -1.0f,  1.0f), glm::vec2(1.0f, 0.0f) }, // 1
		{ glm::vec3( 1.0f,  1.0f,  1.0f), glm::vec2(1.0f, 1.0f) }, // 2
		{ glm::vec3(-1.0f,  1.0f,  1.0f), glm::vec2(0.0f, 1.0f) }, // 3
		
		// Back face (Z-)
		{ glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec2(0.0f, 0.0f) }, // 4
		{ glm::vec3( 1.0f, -1.0f, -1.0f), glm::vec2(1.0f, 0.0f) }, // 5
		{ glm::vec3( 1.0f,  1.0f, -1.0f), glm::vec2(1.0f, 1.0f) }, // 6
		{ glm::vec3(-1.0f,  1.0f, -1.0f), glm::vec2(0.0f, 1.0f) }, // 7
	};

	// 36 indices for 12 triangles (6 faces * 2 triangles)
	GLushort indices[] = {
		// Front
		0, 1, 2,  2, 3, 0,
		// Back
		5, 4, 7,  7, 6, 5,
		// Left
		4, 0, 3,  3, 7, 4,
		// Right
		1, 5, 6,  6, 2, 1,
		// Top
		3, 2, 6,  6, 7, 3,
		// Bottom
		4, 5, 1,  1, 0, 4
	};

	m_indexCount = 36;

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &m_indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);
}

void DD_SimpleBox::Render()
{
	glUseProgram(s_shaderProgram);
	glBindVertexArray(m_vao);

	// Get view and projection matrices from DD_Core
	extern Matrix4 g_View;
	extern Matrix4 g_Projection;

	// Set uniforms
	glUniformMatrix4fv(s_viewLoc, 1, GL_FALSE, glm::value_ptr(g_View));
	glUniformMatrix4fv(s_projLoc, 1, GL_FALSE, glm::value_ptr(g_Projection));

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, m_position);
	model = glm::rotate(model, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));

	glUniformMatrix4fv(s_modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_SHORT, 0);
	glBindVertexArray(0);
}

void DD_SimpleBox::AddPos(const Vec3& addpos)
{
	m_position += addpos;
}

void DD_SimpleBox::AddRot(const Vec3& addrot)
{
	m_rotation += addrot;
}

DD_SimpleBox::DD_SimpleBox()
	: m_vao(0)
	, m_vertexBuffer(0)
	, m_indexBuffer(0)
	, m_indexCount(0)
	, m_rotation(0.0f)
	, m_position(0.0f)
	, m_color(1.0f)
{
}

DD_SimpleBox::~DD_SimpleBox()
{
	if (m_vao) glDeleteVertexArrays(1, &m_vao);
	if (m_vertexBuffer) glDeleteBuffers(1, &m_vertexBuffer);
	if (m_indexBuffer) glDeleteBuffers(1, &m_indexBuffer);
}
