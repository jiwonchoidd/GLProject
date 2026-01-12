#include "DD_SimpleBox.h"
#include "DD_GLDevice.h"
#include "DD_Transform.h"
#include <glm/gtc/type_ptr.hpp>

struct SimpleVertex
{
    glm::vec3 Pos;
    glm::vec2 Tex;
};

GLuint DD_SimpleBox::s_shaderProgram = 0;
GLint DD_SimpleBox::s_modelLoc = -1;
GLint DD_SimpleBox::s_viewLoc = -1;
GLint DD_SimpleBox::s_projLoc = -1;
bool DD_SimpleBox::s_initialized = false;

bool DD_SimpleBox::CachePipline()
{
    if (s_initialized) return true;

#ifdef __EMSCRIPTEN__
    // WebGL1 / GLSL ES 1.00 fallback (emscripten default)
    const char* vertexShaderSource = 
        "precision highp float;\n"
        "attribute vec3 aPos;\n"
        "attribute vec2 aTexCoord;\n"
        "uniform mat4 uModel;\n"
        "uniform mat4 uView;\n"
        "uniform mat4 uProjection;\n"
        "varying vec2 TexCoord;\n"
        "void main() {\n"
        "  gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);\n"
        "  TexCoord = aTexCoord;\n"
        "}\n";

    const char* fragmentShaderSource = 
        "precision mediump float;\n"
        "varying vec2 TexCoord;\n"
        "void main() {\n"
        "  gl_FragColor = vec4(TexCoord, 0.5, 1.0);\n"
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

#ifdef __EMSCRIPTEN__
    // Bind attribute locations for ES1 shaders
    glBindAttribLocation(s_shaderProgram, 0, "aPos");
    glBindAttribLocation(s_shaderProgram, 1, "aTexCoord");
#endif

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

void DD_SimpleBox::CreateMesh()
{
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

#ifndef __EMSCRIPTEN__
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
#endif

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

#ifndef __EMSCRIPTEN__
    glBindVertexArray(0);
#endif
}

void DD_SimpleBox::PrepareForRender(const Matrix4& model, const Matrix4& view, const Matrix4& projection)
{
    if (!s_initialized) CachePipline();

    glUseProgram(s_shaderProgram);
#ifndef __EMSCRIPTEN__
    if (m_vao) glBindVertexArray(m_vao);
#else
    // Bind buffers and attributes manually for WebGL1
    glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(sizeof(glm::vec3)));
#endif

    // Set uniforms
    glUniformMatrix4fv(s_viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(s_projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(s_modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_SHORT, 0);

#ifndef __EMSCRIPTEN__
    if (m_vao) glBindVertexArray(0);
#else
    // disable attributes if needed
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
#endif
}

DD_SimpleBox::DD_SimpleBox() { }
DD_SimpleBox::~DD_SimpleBox() { }
