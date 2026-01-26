#include "DD_DeferredRenderer.h"
#include "DD_GBuffer.h"
#include "DD_Actor.h"
#include "DD_MeshComponent.h"
#include "DD_Mesh.h"
#include "DD_Material.h"
#include "DD_Texture.h"
#include "DD_LightComponent.h"
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>

// Static shader variables
GLuint DD_DeferredRenderer::s_geometryProgram = 0;
GLint DD_DeferredRenderer::s_geoModelLoc = -1;
GLint DD_DeferredRenderer::s_geoViewLoc = -1;
GLint DD_DeferredRenderer::s_geoProjLoc = -1;
GLint DD_DeferredRenderer::s_geoAlbedoLoc = -1;
GLint DD_DeferredRenderer::s_geoMetallicLoc = -1;
GLint DD_DeferredRenderer::s_geoRoughnessLoc = -1;
GLint DD_DeferredRenderer::s_geoAOLoc = -1;
GLint DD_DeferredRenderer::s_geoHasAlbedoTexLoc = -1;
GLint DD_DeferredRenderer::s_geoAlbedoTexLoc = -1;

GLuint DD_DeferredRenderer::s_lightingProgram = 0;
GLint DD_DeferredRenderer::s_litPositionTexLoc = -1;
GLint DD_DeferredRenderer::s_litNormalTexLoc = -1;
GLint DD_DeferredRenderer::s_litAlbedoTexLoc = -1;
GLint DD_DeferredRenderer::s_litMaterialTexLoc = -1;
GLint DD_DeferredRenderer::s_litShadowMapLoc = -1;
GLint DD_DeferredRenderer::s_litCameraPosLoc = -1;
GLint DD_DeferredRenderer::s_litLightSpaceLoc = -1;
GLint DD_DeferredRenderer::s_litDirLightDirLoc = -1;
GLint DD_DeferredRenderer::s_litDirLightColorLoc = -1;
GLint DD_DeferredRenderer::s_litDirLightAmbientLoc = -1;
GLint DD_DeferredRenderer::s_litShadowBiasLoc = -1;
GLint DD_DeferredRenderer::s_litPointLightCountLoc = -1;
GLint DD_DeferredRenderer::s_litPointLightsLoc = -1;
bool DD_DeferredRenderer::s_shadersReady = false;

bool DD_DeferredRenderer::CacheShaders()
{
    if (s_shadersReady) return true;

    // Geometry Pass Shader
    const char* geoVertSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aTexCoord;
        
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;
        
        out vec3 vWorldPos;
        out vec3 vNormal;
        out vec2 vTexCoord;
        
        void main()
        {
            vec4 worldPos = uModel * vec4(aPos, 1.0);
            vWorldPos = worldPos.xyz;
            // Transform normal properly (handle non-uniform scale)
            mat3 normalMatrix = transpose(inverse(mat3(uModel)));
            vNormal = normalize(normalMatrix * aNormal);
            vTexCoord = aTexCoord;
            gl_Position = uProjection * uView * worldPos;
        }
    )";

    const char* geoFragSrc = R"(
        #version 330 core
        layout(location = 0) out vec4 gPosition;
        layout(location = 1) out vec4 gNormal;
        layout(location = 2) out vec4 gAlbedo;
        layout(location = 3) out vec4 gMaterial;
        
        in vec3 vWorldPos;
        in vec3 vNormal;
        in vec2 vTexCoord;
        
        uniform vec3 uAlbedo;
        uniform float uMetallic;
        uniform float uRoughness;
        uniform float uAO;
        uniform int uHasAlbedoTex;
        uniform sampler2D uAlbedoTex;
        
        void main()
        {
            gPosition = vec4(vWorldPos, 1.0);
            gNormal = vec4(normalize(vNormal), 0.0);
            
            vec3 albedo = uAlbedo;
            if (uHasAlbedoTex == 1) {
                albedo *= texture(uAlbedoTex, vTexCoord).rgb;
            }
            gAlbedo = vec4(albedo, uAO);
            gMaterial = vec4(uMetallic, uRoughness, 0.0, 1.0);
        }
    )";

    // Compile geometry shader
    GLuint geoVert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(geoVert, 1, &geoVertSrc, nullptr);
    glCompileShader(geoVert);

    GLint success;
    glGetShaderiv(geoVert, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(geoVert, 512, nullptr, log);
        printf("Geometry vertex shader error: %s\n", log);
        return false;
    }

    GLuint geoFrag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(geoFrag, 1, &geoFragSrc, nullptr);
    glCompileShader(geoFrag);

    glGetShaderiv(geoFrag, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(geoFrag, 512, nullptr, log);
        printf("Geometry fragment shader error: %s\n", log);
        return false;
    }

    s_geometryProgram = glCreateProgram();
    glAttachShader(s_geometryProgram, geoVert);
    glAttachShader(s_geometryProgram, geoFrag);
    glLinkProgram(s_geometryProgram);
    glDeleteShader(geoVert);
    glDeleteShader(geoFrag);

    s_geoModelLoc = glGetUniformLocation(s_geometryProgram, "uModel");
    s_geoViewLoc = glGetUniformLocation(s_geometryProgram, "uView");
    s_geoProjLoc = glGetUniformLocation(s_geometryProgram, "uProjection");
    s_geoAlbedoLoc = glGetUniformLocation(s_geometryProgram, "uAlbedo");
    s_geoMetallicLoc = glGetUniformLocation(s_geometryProgram, "uMetallic");
    s_geoRoughnessLoc = glGetUniformLocation(s_geometryProgram, "uRoughness");
    s_geoAOLoc = glGetUniformLocation(s_geometryProgram, "uAO");
    s_geoHasAlbedoTexLoc = glGetUniformLocation(s_geometryProgram, "uHasAlbedoTex");
    s_geoAlbedoTexLoc = glGetUniformLocation(s_geometryProgram, "uAlbedoTex");

    // Lighting Pass Shader
    const char* litVertSrc = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 vTexCoord;
        void main() {
            vTexCoord = aTexCoord;
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* litFragSrc = R"(
        #version 330 core
        out vec4 fragColor;
        in vec2 vTexCoord;
        
        uniform sampler2D gPosition;
        uniform sampler2D gNormal;
        uniform sampler2D gAlbedo;
        uniform sampler2D gMaterial;
        uniform sampler2DShadow uShadowMap;
        
        uniform vec3 uCameraPos;
        uniform mat4 uLightSpace;
        
        // Directional light
        uniform vec3 uDirLightDir;
        uniform vec3 uDirLightColor;
        uniform float uAmbient;
        uniform float uShadowBias;
        
        // Point lights
        #define MAX_POINT_LIGHTS 32
        struct PointLight {
            vec3 position;
            float radius;
            vec3 color;
            float intensity;
        };
        uniform int uPointLightCount;
        uniform PointLight uPointLights[MAX_POINT_LIGHTS];
        
        const float PI = 3.14159265359;
        
        // Improved GGX Distribution - Disney/Epic style
        float DistributionGGX(vec3 N, vec3 H, float roughness) {
            float a = roughness * roughness;
            float a2 = a * a;
            float NdotH = max(dot(N, H), 0.0);
            float NdotH2 = NdotH * NdotH;
            
            float nom = a2;
            float denom = (NdotH2 * (a2 - 1.0) + 1.0);
            denom = PI * denom * denom;
            
            return nom / max(denom, 0.0001);
        }
        
        // Schlick-GGX Geometry function
        float GeometrySchlickGGX(float NdotV, float roughness) {
            float r = (roughness + 1.0);
            float k = (r * r) / 8.0;
            
            float nom = NdotV;
            float denom = NdotV * (1.0 - k) + k;
            
            return nom / max(denom, 0.0001);
        }
        
        float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
            float NdotV = max(dot(N, V), 0.0);
            float NdotL = max(dot(N, L), 0.0);
            float ggx2 = GeometrySchlickGGX(NdotV, roughness);
            float ggx1 = GeometrySchlickGGX(NdotL, roughness);
            
            return ggx1 * ggx2;
        }
        
        // Fresnel with roughness consideration
        vec3 FresnelSchlick(float cosTheta, vec3 F0) {
            return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
        }
        
        vec3 FresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
            return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
        }
        
        float ShadowCalculation(vec4 fragPosLightSpace) {
            vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
            projCoords = projCoords * 0.5 + 0.5;
            if (projCoords.z > 1.0) return 0.0;
            
            float shadow = 0.0;
            vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
            for (int x = -1; x <= 1; ++x) {
                for (int y = -1; y <= 1; ++y) {
                    vec3 coord = vec3(projCoords.xy + vec2(x, y) * texelSize, projCoords.z - uShadowBias);
                    shadow += 1.0 - texture(uShadowMap, coord);
                }
            }
            return shadow / 9.0;
        }
        
        vec3 CalcPBRLighting(vec3 N, vec3 V, vec3 L, vec3 albedo, float metallic, float roughness, vec3 radiance) {
            vec3 H = normalize(V + L);
            
            vec3 F0 = vec3(0.04);
            F0 = mix(F0, albedo, metallic);
            
            float NdotL = max(dot(N, L), 0.0);
            float NdotV = max(dot(N, V), 0.0);
            float HdotV = max(dot(H, V), 0.0);
            
            // Cook-Torrance BRDF
            float NDF = DistributionGGX(N, H, roughness);
            float G = GeometrySmith(N, V, L, roughness);
            vec3 F = FresnelSchlick(HdotV, F0);
            
            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * NdotV * NdotL + 0.0001;
            vec3 specular = numerator / denominator;
            
            // Energy conservation
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - metallic;
            
            return (kD * albedo / PI + specular) * radiance * NdotL;
        }
        
        vec3 CalcPointLight(PointLight light, vec3 N, vec3 V, vec3 fragPos, vec3 albedo, float metallic, float roughness) {
            vec3 L = normalize(light.position - fragPos);
            float distance = length(light.position - fragPos);
            
            if (distance > light.radius) return vec3(0.0);
            
            // Smooth attenuation
            float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * distance * distance);
            float falloff = 1.0 - smoothstep(light.radius * 0.5, light.radius, distance);
            attenuation *= falloff;
            
            vec3 radiance = light.color * light.intensity * attenuation;
            
            return CalcPBRLighting(N, V, L, albedo, metallic, roughness, radiance);
        }
        
        void main() {
            vec3 fragPos = texture(gPosition, vTexCoord).rgb;
            vec3 normal = normalize(texture(gNormal, vTexCoord).rgb);
            vec3 albedo = texture(gAlbedo, vTexCoord).rgb;
            float ao = texture(gAlbedo, vTexCoord).a;
            float metallic = texture(gMaterial, vTexCoord).r;
            float roughness = max(texture(gMaterial, vTexCoord).g, 0.04); // Minimum roughness to avoid artifacts
            
            // Skip background pixels (no geometry)
            if (length(fragPos) < 0.001) {
                fragColor = vec4(0.1, 0.1, 0.15, 1.0);
                return;
            }
            
            vec3 V = normalize(uCameraPos - fragPos);
            vec3 L = normalize(-uDirLightDir);
            
            // Shadow
            vec4 fragPosLightSpace = uLightSpace * vec4(fragPos, 1.0);
            float shadow = ShadowCalculation(fragPosLightSpace);
            
            // Directional light PBR
            vec3 Lo = CalcPBRLighting(normal, V, L, albedo, metallic, roughness, uDirLightColor) * (1.0 - shadow);
            
            // Point lights
            for (int i = 0; i < uPointLightCount; ++i) {
                Lo += CalcPointLight(uPointLights[i], normal, V, fragPos, albedo, metallic, roughness);
            }
            
            // Ambient with AO
            vec3 ambient = uAmbient * albedo * ao;
            vec3 color = ambient + Lo;
            
            // Tone mapping (ACES approximation)
            color = color / (color + vec3(1.0));
            
            // Gamma correction
            color = pow(color, vec3(1.0/2.2));
            
            fragColor = vec4(color, 1.0);
        }
    )";

    GLuint litVert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(litVert, 1, &litVertSrc, nullptr);
    glCompileShader(litVert);

    glGetShaderiv(litVert, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(litVert, 512, nullptr, log);
        printf("Lighting vertex shader error: %s\n", log);
        return false;
    }

    GLuint litFrag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(litFrag, 1, &litFragSrc, nullptr);
    glCompileShader(litFrag);

    glGetShaderiv(litFrag, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(litFrag, 512, nullptr, log);
        printf("Lighting fragment shader error: %s\n", log);
        return false;
    }

    s_lightingProgram = glCreateProgram();
    glAttachShader(s_lightingProgram, litVert);
    glAttachShader(s_lightingProgram, litFrag);
    glLinkProgram(s_lightingProgram);
    glDeleteShader(litVert);
    glDeleteShader(litFrag);

    // Get lighting uniforms
    s_litPositionTexLoc = glGetUniformLocation(s_lightingProgram, "gPosition");
    s_litNormalTexLoc = glGetUniformLocation(s_lightingProgram, "gNormal");
    s_litAlbedoTexLoc = glGetUniformLocation(s_lightingProgram, "gAlbedo");
    s_litMaterialTexLoc = glGetUniformLocation(s_lightingProgram, "gMaterial");
    s_litShadowMapLoc = glGetUniformLocation(s_lightingProgram, "uShadowMap");
    s_litCameraPosLoc = glGetUniformLocation(s_lightingProgram, "uCameraPos");
    s_litLightSpaceLoc = glGetUniformLocation(s_lightingProgram, "uLightSpace");
    s_litDirLightDirLoc = glGetUniformLocation(s_lightingProgram, "uDirLightDir");
    s_litDirLightColorLoc = glGetUniformLocation(s_lightingProgram, "uDirLightColor");
    s_litDirLightAmbientLoc = glGetUniformLocation(s_lightingProgram, "uAmbient");
    s_litShadowBiasLoc = glGetUniformLocation(s_lightingProgram, "uShadowBias");
    s_litPointLightCountLoc = glGetUniformLocation(s_lightingProgram, "uPointLightCount");
    s_litPointLightsLoc = glGetUniformLocation(s_lightingProgram, "uPointLights");

    printf("Deferred shaders compiled successfully\n");
    s_shadersReady = true;
    return true;
}

void DD_DeferredRenderer::ClearShaders()
{
    if (s_geometryProgram) { glDeleteProgram(s_geometryProgram); s_geometryProgram = 0; }
    if (s_lightingProgram) { glDeleteProgram(s_lightingProgram); s_lightingProgram = 0; }
    s_shadersReady = false;
}

DD_DeferredRenderer::DD_DeferredRenderer()
    : m_view(1.0f)
    , m_projection(1.0f)
    , m_pointLightCount(0)
    , m_quadVAO(0)
    , m_quadVBO(0)
{
}

DD_DeferredRenderer::~DD_DeferredRenderer()
{
    Shutdown();
}

bool DD_DeferredRenderer::Initialize(int width, int height)
{
    if (!CacheShaders()) return false;

    m_gBuffer = std::make_unique<DD_GBuffer>();
    if (!m_gBuffer->Initialize(width, height)) return false;

    // Create fullscreen quad
    float quadVertices[] = {
        -1.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, 1.0f, 1.0f
    };

    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    printf("Deferred Renderer initialized\n");
    return true;
}

void DD_DeferredRenderer::Shutdown()
{
    if (m_quadVAO) { glDeleteVertexArrays(1, &m_quadVAO); m_quadVAO = 0; }
    if (m_quadVBO) { glDeleteBuffers(1, &m_quadVBO); m_quadVBO = 0; }
    m_gBuffer.reset();
}

void DD_DeferredRenderer::Resize(int width, int height)
{
    if (m_gBuffer) m_gBuffer->Resize(width, height);
}

void DD_DeferredRenderer::BeginGeometryPass(const Matrix4& view, const Matrix4& projection)
{
    m_view = view;
    m_projection = projection;

    m_gBuffer->BindForGeometryPass();
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glUseProgram(s_geometryProgram);
    glUniformMatrix4fv(s_geoViewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(s_geoProjLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform1i(s_geoAlbedoTexLoc, 0);
}

void DD_DeferredRenderer::RenderActor(DD_Actor* actor)
{
    if (!actor) return;

    DD_MeshComponent* meshComp = actor->GetMeshComponent();
    if (!meshComp || !meshComp->IsVisible()) return;

    DD_Mesh* mesh = meshComp->GetMesh();
    if (!mesh) return;

    Matrix4 model = actor->GetModelMatrix();
    glUniformMatrix4fv(s_geoModelLoc, 1, GL_FALSE, glm::value_ptr(model));

    DD_Material* material = meshComp->GetMaterial();
    if (material)
    {
        glUniform3fv(s_geoAlbedoLoc, 1, glm::value_ptr(material->GetAlbedo()));
        glUniform1f(s_geoMetallicLoc, material->GetMetallic());
        glUniform1f(s_geoRoughnessLoc, material->GetRoughness());
        glUniform1f(s_geoAOLoc, material->GetAO());

        if (material->HasAlbedoTexture())
        {
            glUniform1i(s_geoHasAlbedoTexLoc, 1);
            glActiveTexture(GL_TEXTURE0);
            material->GetAlbedoTexture()->Bind();
        }
        else
        {
            glUniform1i(s_geoHasAlbedoTexLoc, 0);
        }
    }
    else
    {
        glUniform3f(s_geoAlbedoLoc, 0.8f, 0.8f, 0.8f);
        glUniform1f(s_geoMetallicLoc, 0.0f);
        glUniform1f(s_geoRoughnessLoc, 0.5f);
        glUniform1f(s_geoAOLoc, 1.0f);
        glUniform1i(s_geoHasAlbedoTexLoc, 0);
    }

    GLuint vao = mesh->GetVAO();
    if (vao) glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_SHORT, 0);
    if (vao) glBindVertexArray(0);
}

void DD_DeferredRenderer::EndGeometryPass()
{
    m_gBuffer->UnbindGeometryPass();
}

void DD_DeferredRenderer::BeginLightingPass(const Vec3& cameraPos)
{
    glDisable(GL_DEPTH_TEST);
    glUseProgram(s_lightingProgram);

    m_gBuffer->BindTexturesForLightingPass();
    glUniform1i(s_litPositionTexLoc, 0);
    glUniform1i(s_litNormalTexLoc, 1);
    glUniform1i(s_litAlbedoTexLoc, 2);
    glUniform1i(s_litMaterialTexLoc, 3);
    glUniform1i(s_litShadowMapLoc, 4);

    glUniform3fv(s_litCameraPosLoc, 1, glm::value_ptr(cameraPos));
}

void DD_DeferredRenderer::SetDirectionalLight(const DD_LightComponent* light, GLuint shadowMap, const Matrix4& lightSpaceMatrix)
{
    Vec3 dir = light->GetDirection();
    Vec3 color = light->GetColor() * light->GetIntensity();
    
    glUniform3fv(s_litDirLightDirLoc, 1, glm::value_ptr(dir));
    glUniform3fv(s_litDirLightColorLoc, 1, glm::value_ptr(color));
    glUniform1f(s_litDirLightAmbientLoc, light->GetAmbient());
    glUniform1f(s_litShadowBiasLoc, light->GetShadowBias());
    glUniformMatrix4fv(s_litLightSpaceLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
}

void DD_DeferredRenderer::AddPointLight(const Vec3& position, const Vec3& color, float intensity, float radius)
{
    if (m_pointLightCount >= MAX_POINT_LIGHTS) return;

    m_pointLights[m_pointLightCount].position = position;
    m_pointLights[m_pointLightCount].color = color;
    m_pointLights[m_pointLightCount].intensity = intensity;
    m_pointLights[m_pointLightCount].radius = radius;
    m_pointLightCount++;
}

void DD_DeferredRenderer::EndLightingPass()
{
    glUniform1i(s_litPointLightCountLoc, m_pointLightCount);

    // Upload point lights
    for (int i = 0; i < m_pointLightCount; ++i)
    {
        char buf[64];
        snprintf(buf, sizeof(buf), "uPointLights[%d].position", i);
        glUniform3fv(glGetUniformLocation(s_lightingProgram, buf), 1, glm::value_ptr(m_pointLights[i].position));
        snprintf(buf, sizeof(buf), "uPointLights[%d].radius", i);
        glUniform1f(glGetUniformLocation(s_lightingProgram, buf), m_pointLights[i].radius);
        snprintf(buf, sizeof(buf), "uPointLights[%d].color", i);
        glUniform3fv(glGetUniformLocation(s_lightingProgram, buf), 1, glm::value_ptr(m_pointLights[i].color));
        snprintf(buf, sizeof(buf), "uPointLights[%d].intensity", i);
        glUniform1f(glGetUniformLocation(s_lightingProgram, buf), m_pointLights[i].intensity);
    }

    RenderFullscreenQuad();

    m_gBuffer->UnbindTextures();
    glEnable(GL_DEPTH_TEST);
}

void DD_DeferredRenderer::RenderFullscreenQuad()
{
    glBindVertexArray(m_quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
