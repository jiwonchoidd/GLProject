#include "DD_DebugDraw.h"
#include <vector>

namespace DebugDraw
{
    bool Enabled = true;

    static GLuint s_vao = 0;
    static GLuint s_vbo = 0;
    static GLuint s_program = 0;

    static const char* vsSrc = R"(
    #version 330 core
    layout(location = 0) in vec3 aPos;
    uniform mat4 uMVP;
    void main() { gl_Position = uMVP * vec4(aPos, 1.0); }
    )";

    static const char* fsSrc = R"(
    #version 330 core
    out vec4 FragColor;
    uniform vec4 uColor;
    void main() { FragColor = uColor; }
    )";

    static GLuint CompileShader(GLenum type, const char* src)
    {
        GLuint s = glCreateShader(type);
        glShaderSource(s, 1, &src, nullptr);
        glCompileShader(s);
        GLint ok = 0; glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) { char buf[512]; glGetShaderInfoLog(s, 512, nullptr, buf); printf("Shader compile error: %s\n", buf); }
        return s;
    }

    void Init()
    {
        if (s_program) return;
        GLuint vs = CompileShader(GL_VERTEX_SHADER, vsSrc);
        GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsSrc);
        s_program = glCreateProgram();
        glAttachShader(s_program, vs);
        glAttachShader(s_program, fs);
        glLinkProgram(s_program);
        glDeleteShader(vs); glDeleteShader(fs);

        // unit box lines (12 edges * 2 vertices = 24)
        std::vector<glm::vec3> verts = {
            {-1,-1,-1},{1,-1,-1},
            {1,-1,-1},{1,1,-1},
            {1,1,-1},{-1,1,-1},
            {-1,1,-1},{-1,-1,-1},

            {-1,-1,1},{1,-1,1},
            {1,-1,1},{1,1,1},
            {1,1,1},{-1,1,1},
            {-1,1,1},{-1,-1,1},

            {-1,-1,-1},{-1,-1,1},
            {1,-1,-1},{1,-1,1},
            {1,1,-1},{1,1,1},
            {-1,1,-1},{-1,1,1}
        };

        glGenVertexArrays(1, &s_vao);
        glGenBuffers(1, &s_vbo);
        glBindVertexArray(s_vao);
        glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(glm::vec3), verts.data(), GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
        glBindVertexArray(0);
    }

    void Shutdown()
    {
        if (s_vbo) glDeleteBuffers(1, &s_vbo);
        if (s_vao) glDeleteVertexArrays(1, &s_vao);
        if (s_program) glDeleteProgram(s_program);
        s_vbo = s_vao = s_program = 0;
    }

    void DrawAABBWire(const AABB& aabb, const Matrix4& view, const Matrix4& proj, const Vec4& color)
    {
        if (!Enabled) return;
        if (!s_program) Init();

        // Build model: translate to center, scale by half extents
        Matrix4 model = glm::translate(Matrix4(1.0f), aabb.center);
        model = glm::scale(model, aabb.halfExtents);

        Matrix4 mvp = proj * view * model;
        glUseProgram(s_program);
        GLint loc = glGetUniformLocation(s_program, "uMVP");
        glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mvp));
        GLint colLoc = glGetUniformLocation(s_program, "uColor");
        glUniform4f(colLoc, color.r, color.g, color.b, color.a);

        glBindVertexArray(s_vao);
        glDrawArrays(GL_LINES, 0, 24);
        glBindVertexArray(0);
    }
}
