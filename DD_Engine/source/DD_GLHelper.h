#pragma once

#ifdef __EMSCRIPTEN__
#include <GLES3/gl3.h>
#else
#ifdef _WIN32
#include <GL/glew.h>
#endif
#endif

#include <GLFW/glfw3.h>

// Enable experimental extensions used by GLM (gtx)
#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/component_wise.hpp>

#include <vector>
#include <memory>

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Matrix4 = glm::mat4;
using Color = glm::vec4;
