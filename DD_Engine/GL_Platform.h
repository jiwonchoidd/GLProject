#pragma once

// GLFW + OpenGL ES Áö¿ø
#ifdef __EMSCRIPTEN__
    #define PLATFORM_WEB 1
    #include <emscripten/emscripten.h>
    #include <emscripten/html5.h>
    #define GLFW_INCLUDE_ES3
    #include <GLFW/glfw3.h>
#else
    #define PLATFORM_DESKTOP 1
    #define GLFW_INCLUDE_ES3
    #include <GLFW/glfw3.h>
#endif

#include <cmath>
#include <cstring>

// Simple vector types (replacing DirectXMath)
struct Vec2 {
    float x, y;
    Vec2() : x(0), y(0) {}
    Vec2(float x_, float y_) : x(x_), y(y_) {}
};

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
    
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    
    float length() const { return sqrtf(x*x + y*y + z*z); }
    Vec3 normalized() const {
        float len = length();
        return len > 0 ? Vec3(x/len, y/len, z/len) : Vec3(0, 0, 0);
    }
    
    static Vec3 cross(const Vec3& a, const Vec3& b) {
        return Vec3(
            a.y * b.z - a.z * b.y,
            a.z * b.x - a.x * b.z,
            a.x * b.y - a.y * b.x
        );
    }
    
    static float dot(const Vec3& a, const Vec3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }
};

struct Vec4 {
    float x, y, z, w;
    Vec4() : x(0), y(0), z(0), w(0) {}
    Vec4(float x_, float y_, float z_, float w_) : x(x_), y(y_), z(z_), w(w_) {}
};

// 4x4 Matrix (column-major like OpenGL)
struct Matrix4 {
    float m[16];
    
    Matrix4() {
        identity();
    }
    
    void identity() {
        memset(m, 0, sizeof(m));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }
    
    static Matrix4 perspective(float fovY, float aspect, float nearZ, float farZ) {
        Matrix4 result;
        memset(result.m, 0, sizeof(result.m));
        
        float f = 1.0f / tanf(fovY * 0.5f);
        result.m[0] = f / aspect;
        result.m[5] = f;
        result.m[10] = (farZ + nearZ) / (nearZ - farZ);
        result.m[11] = -1.0f;
        result.m[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
        
        return result;
    }
    
    static Matrix4 lookAt(const Vec3& eye, const Vec3& center, const Vec3& up) {
        Matrix4 result;
        
        Vec3 f = (center - eye).normalized();
        Vec3 s = Vec3::cross(f, up).normalized();
        Vec3 u = Vec3::cross(s, f);
        
        result.m[0] = s.x;
        result.m[4] = s.y;
        result.m[8] = s.z;
        
        result.m[1] = u.x;
        result.m[5] = u.y;
        result.m[9] = u.z;
        
        result.m[2] = -f.x;
        result.m[6] = -f.y;
        result.m[10] = -f.z;
        
        result.m[12] = -Vec3::dot(s, eye);
        result.m[13] = -Vec3::dot(u, eye);
        result.m[14] = Vec3::dot(f, eye);
        result.m[15] = 1.0f;
        
        return result;
    }
    
    static Matrix4 translation(const Vec3& v) {
        Matrix4 result;
        result.identity();
        result.m[12] = v.x;
        result.m[13] = v.y;
        result.m[14] = v.z;
        return result;
    }
    
    static Matrix4 rotationX(float angle) {
        Matrix4 result;
        float c = cosf(angle);
        float s = sinf(angle);
        result.m[5] = c;
        result.m[6] = s;
        result.m[9] = -s;
        result.m[10] = c;
        return result;
    }
    
    static Matrix4 rotationY(float angle) {
        Matrix4 result;
        float c = cosf(angle);
        float s = sinf(angle);
        result.m[0] = c;
        result.m[2] = -s;
        result.m[8] = s;
        result.m[10] = c;
        return result;
    }
    
    static Matrix4 rotationZ(float angle) {
        Matrix4 result;
        float c = cosf(angle);
        float s = sinf(angle);
        result.m[0] = c;
        result.m[1] = s;
        result.m[4] = -s;
        result.m[5] = c;
        return result;
    }
    
    Matrix4 operator*(const Matrix4& other) const {
        Matrix4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i*4 + j] = 0;
                for (int k = 0; k < 4; ++k) {
                    result.m[i*4 + j] += m[k*4 + j] * other.m[i*4 + k];
                }
            }
        }
        return result;
    }
};

// Utility functions
inline float DegreesToRadians(float degrees) {
    return degrees * 3.14159265358979323846f / 180.0f;
}

// Color helper
struct Color {
    float r, g, b, a;
    Color() : r(1), g(1), b(1), a(1) {}
    Color(float r_, float g_, float b_, float a_ = 1.0f) 
        : r(r_), g(g_), b(b_), a(a_) {}
    
    static Color Random() {
        return Color(
            static_cast<float>(rand()) / RAND_MAX,
            static_cast<float>(rand()) / RAND_MAX,
            static_cast<float>(rand()) / RAND_MAX,
            1.0f
        );
    }
};
