#pragma once
#ifndef PLATFORM_WEB
#include <GL/glew.h>
#endif

// GLFW and OpenGL
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Type aliases for convenience
using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Matrix4 = glm::mat4;
using Color = glm::vec4;
