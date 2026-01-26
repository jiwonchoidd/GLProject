#include "DD_SceneRenderer.h"
#include "DD_LightComponent.h"
#include "DD_Actor.h"
#include "DD_MeshComponent.h"
#include "DD_Mesh.h"
#include "DD_Material.h"
#include "DD_Texture.h"
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>

GLuint DD_SceneRenderer::s_sceneProgram = 0;
GLint DD_SceneRenderer::s_modelLoc = -1;
GLint DD_SceneRenderer::s_viewLoc = -1;
GLint DD_SceneRenderer::s_projLoc = -1;
GLint DD_SceneRenderer::s_lightSpaceLoc = -1;
GLint DD_SceneRenderer::s_shadowMapLoc = -1;
GLint DD_SceneRenderer::s_lightDirLoc = -1;
GLint DD_SceneRenderer::s_lightColorLoc = -1;
GLint DD_SceneRenderer::s_ambientLoc = -1;
GLint DD_SceneRenderer::s_shadowBiasLoc = -1;
GLint DD_SceneRenderer::s_cameraPosLoc = -1;
GLint DD_SceneRenderer::s_albedoLoc = -1;
GLint DD_SceneRenderer::s_metallicLoc = -1;
GLint DD_SceneRenderer::s_roughnessLoc = -1;
GLint DD_SceneRenderer::s_aoLoc = -1;
GLint DD_SceneRenderer::s_hasAlbedoTexLoc = -1;
GLint DD_SceneRenderer::s_albedoTexLoc = -1;
bool DD_SceneRenderer::s_shadersReady = false;

bool DD_SceneRenderer::CacheShaders()
{
    if (s_shadersReady) return true;

#ifdef __EMSCRIPTEN__
    // WebGL2 / GLSL ES 3.00 with PBR
    const char* vertexShaderSource =
        "#version 300 es\n"
        "precision highp float;\n"
        "layout(location = 0) in vec3 aPos;\n"
        "layout(location = 1) in vec3 aNormal;\n"
        "layout(location = 2) in vec2 aTexCoord;\n"
        "\n"
        "uniform mat4 uModel;\n"
        "uniform mat4 uView;\n"
        "uniform mat4 uProjection;\n"
        "uniform mat4 uLightSpace;\n"
        "\n"
        "out vec3 vWorldPos;\n"
        "out vec3 vNormal;\n"
        "out vec2 vTexCoord;\n"
        "out vec4 vLightSpacePos;\n"
        "\n"
        "void main() {\n"
        "    vec4 worldPos = uModel * vec4(aPos, 1.0);\n"
        "    vWorldPos = worldPos.xyz;\n"
        "    mat3 normalMatrix = transpose(inverse(mat3(uModel)));\n"
        "    vNormal = normalize(normalMatrix * aNormal);\n"
        "    vTexCoord = aTexCoord;\n"
        "    vLightSpacePos = uLightSpace * worldPos;\n"
        "    gl_Position = uProjection * uView * worldPos;\n"
        "}\n";

    const char* fragmentShaderSource =
        "#version 300 es\n"
        "precision highp float;\n"
        "\n"
        "in vec3 vWorldPos;\n"
        "in vec3 vNormal;\n"
        "in vec2 vTexCoord;\n"
        "in vec4 vLightSpacePos;\n"
        "\n"
        "uniform sampler2D uShadowMap;\n"
        "uniform sampler2D uAlbedoTex;\n"
        "uniform vec3 uLightDir;\n"
        "uniform vec3 uLightColor;\n"
        "uniform vec3 uCameraPos;\n"
        "uniform float uAmbient;\n"
        "uniform float uShadowBias;\n"
        "uniform vec3 uAlbedo;\n"
        "uniform float uMetallic;\n"
        "uniform float uRoughness;\n"
        "uniform float uAO;\n"
        "uniform int uHasAlbedoTex;\n"
        "\n"
        "out vec4 fragColor;\n"
        "\n"
        "const float PI = 3.14159265359;\n"
        "\n"
        "float ShadowCalculation(vec4 lightSpacePos) {\n"
        "    vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;\n"
        "    projCoords = projCoords * 0.5 + 0.5;\n"
        "    if (projCoords.z > 1.0) return 0.0;\n"
        "    float closestDepth = texture(uShadowMap, projCoords.xy).r;\n"
        "    float currentDepth = projCoords.z;\n"
        "    float shadow = currentDepth - uShadowBias > closestDepth ? 1.0 : 0.0;\n"
        "    return shadow;\n"
        "}\n"
        "\n"
        "float DistributionGGX(vec3 N, vec3 H, float roughness) {\n"
        "    float a = roughness * roughness;\n"
        "    float a2 = a * a;\n"
        "    float NdotH = max(dot(N, H), 0.0);\n"
        "    float NdotH2 = NdotH * NdotH;\n"
        "    float denom = (NdotH2 * (a2 - 1.0) + 1.0);\n"
        "    return a2 / (PI * denom * denom + 0.0001);\n"
        "}\n"
        "\n"
        "float GeometrySchlickGGX(float NdotV, float roughness) {\n"
        "    float r = (roughness + 1.0);\n"
        "    float k = (r * r) / 8.0;\n"
        "    return NdotV / (NdotV * (1.0 - k) + k + 0.0001);\n"
        "}\n"
        "\n"
        "float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {\n"
        "    float NdotV = max(dot(N, V), 0.0);\n"
        "    float NdotL = max(dot(N, L), 0.0);\n"
        "    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);\n"
        "}\n"
        "\n"
        "vec3 FresnelSchlick(float cosTheta, vec3 F0) {\n"
        "    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);\n"
        "}\n"
        "\n"
        "void main() {\n"
        "    vec3 albedo = uAlbedo;\n"
        "    if (uHasAlbedoTex == 1) {\n"
        "        albedo *= texture(uAlbedoTex, vTexCoord).rgb;\n"
        "    }\n"
        "    float roughness = max(uRoughness, 0.04);\n"
        "    \n"
        "    vec3 N = normalize(vNormal);\n"
        "    vec3 V = normalize(uCameraPos - vWorldPos);\n"
        "    vec3 L = normalize(-uLightDir);\n"
        "    vec3 H = normalize(V + L);\n"
        "    \n"
        "    float shadow = ShadowCalculation(vLightSpacePos);\n"
        "    \n"
        "    vec3 F0 = vec3(0.04);\n"
        "    F0 = mix(F0, albedo, uMetallic);\n"
        "    \n"
        "    float NDF = DistributionGGX(N, H, roughness);\n"
        "    float G = GeometrySmith(N, V, L, roughness);\n"
        "    vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);\n"
        "    \n"
        "    vec3 numerator = NDF * G * F;\n"
        "    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;\n"
        "    vec3 specular = numerator / denominator;\n"
        "    \n"
        "    vec3 kS = F;\n"
        "    vec3 kD = vec3(1.0) - kS;\n"
        "    kD *= 1.0 - uMetallic;\n"
        "    \n"
        "    float NdotL = max(dot(N, L), 0.0);\n"
        "    vec3 Lo = (kD * albedo / PI + specular) * uLightColor * NdotL * (1.0 - shadow);\n"
        "    vec3 ambient = uAmbient * albedo * uAO;\n"
        "    vec3 color = ambient + Lo;\n"
        "    \n"
        "    color = color / (color + vec3(1.0));\n"
        "    color = pow(color, vec3(1.0/2.2));\n"
        "    fragColor = vec4(color, 1.0);\n"
        "}\n";
#else
    // Desktop OpenGL 3.3 Core with PBR/BSDF
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aTexCoord;
        
        uniform mat4 uModel;
        uniform mat4 uView;
        uniform mat4 uProjection;
        uniform mat4 uLightSpace;
        
        out vec3 vWorldPos;
        out vec3 vNormal;
        out vec2 vTexCoord;
        out vec4 vLightSpacePos;
        
        void main()
        {
            vec4 worldPos = uModel * vec4(aPos, 1.0);
            vWorldPos = worldPos.xyz;
            mat3 normalMatrix = transpose(inverse(mat3(uModel)));
            vNormal = normalize(normalMatrix * aNormal);
            vTexCoord = aTexCoord;
            vLightSpacePos = uLightSpace * worldPos;
            gl_Position = uProjection * uView * worldPos;
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        
        in vec3 vWorldPos;
        in vec3 vNormal;
        in vec2 vTexCoord;
        in vec4 vLightSpacePos;
        
        uniform sampler2DShadow uShadowMap;
        uniform sampler2D uAlbedoTex;
        uniform vec3 uLightDir;
        uniform vec3 uLightColor;
        uniform vec3 uCameraPos;
        uniform float uAmbient;
        uniform float uShadowBias;
        
        // PBR Material
        uniform vec3 uAlbedo;
        uniform float uMetallic;
        uniform float uRoughness;
        uniform float uAO;
        uniform int uHasAlbedoTex;
        
        out vec4 fragColor;
        
        const float PI = 3.14159265359;
        
        float ShadowCalculation(vec4 lightSpacePos)
        {
            vec3 projCoords = lightSpacePos.xyz / lightSpacePos.w;
            projCoords = projCoords * 0.5 + 0.5;
            if (projCoords.z > 1.0) return 0.0;
            
            float shadow = 0.0;
            vec2 texelSize = 1.0 / textureSize(uShadowMap, 0);
            for (int x = -1; x <= 1; ++x)
            {
                for (int y = -1; y <= 1; ++y)
                {
                    vec3 sampleCoord = vec3(projCoords.xy + vec2(x, y) * texelSize, projCoords.z - uShadowBias);
                    shadow += 1.0 - texture(uShadowMap, sampleCoord);
                }
            }
            return shadow / 9.0;
        }
        
        // GGX/Trowbridge-Reitz NDF
        float DistributionGGX(vec3 N, vec3 H, float roughness)
        {
            float a = roughness * roughness;
            float a2 = a * a;
            float NdotH = max(dot(N, H), 0.0);
            float NdotH2 = NdotH * NdotH;
            float denom = (NdotH2 * (a2 - 1.0) + 1.0);
            return a2 / (PI * denom * denom);
        }
        
        // Schlick-GGX Geometry
        float GeometrySchlickGGX(float NdotV, float roughness)
        {
            float r = roughness + 1.0;
            float k = (r * r) / 8.0;
            return NdotV / (NdotV * (1.0 - k) + k);
        }
        
        float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
        {
            float NdotV = max(dot(N, V), 0.0);
            float NdotL = max(dot(N, L), 0.0);
            return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
        }
        
        // Fresnel-Schlick
        vec3 FresnelSchlick(float cosTheta, vec3 F0)
        {
            return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
        }
        
        void main()
        {
            // Get albedo
            vec3 albedo = uAlbedo;
            if (uHasAlbedoTex == 1)
            {
                albedo = texture(uAlbedoTex, vTexCoord).rgb;
            }
            
            vec3 N = normalize(vNormal);
            vec3 V = normalize(uCameraPos - vWorldPos);
            vec3 L = normalize(-uLightDir);
            vec3 H = normalize(V + L);
            
            // Shadow
            float shadow = ShadowCalculation(vLightSpacePos);
            
            // PBR calculations
            vec3 F0 = vec3(0.04);
            F0 = mix(F0, albedo, uMetallic);
            
            float NDF = DistributionGGX(N, H, uRoughness);
            float G = GeometrySmith(N, V, L, uRoughness);
            vec3 F = FresnelSchlick(max(dot(H, V), 0.0), F0);
            
            vec3 numerator = NDF * G * F;
            float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
            vec3 specular = numerator / denominator;
            
            vec3 kS = F;
            vec3 kD = vec3(1.0) - kS;
            kD *= 1.0 - uMetallic;
            
            float NdotL = max(dot(N, L), 0.0);
            
            // Combine
            vec3 Lo = (kD * albedo / PI + specular) * uLightColor * NdotL * (1.0 - shadow);
            vec3 ambient = uAmbient * albedo * uAO;
            vec3 color = ambient + Lo;
            
            // HDR tonemapping
            color = color / (color + vec3(1.0));
            // Gamma correction
            color = pow(color, vec3(1.0/2.2));
            
            fragColor = vec4(color, 1.0);
        }
    )";
#endif

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    GLint success;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        printf("Scene vertex shader compilation failed:\n%s\n", infoLog);
        return false;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        printf("Scene fragment shader compilation failed:\n%s\n", infoLog);
        glDeleteShader(vertexShader);
        return false;
    }

    s_sceneProgram = glCreateProgram();
    glAttachShader(s_sceneProgram, vertexShader);
    glAttachShader(s_sceneProgram, fragmentShader);

#ifdef __EMSCRIPTEN__
    glBindAttribLocation(s_sceneProgram, 0, "aPos");
    glBindAttribLocation(s_sceneProgram, 1, "aNormal");
    glBindAttribLocation(s_sceneProgram, 2, "aTexCoord");
#endif

    glLinkProgram(s_sceneProgram);

    glGetProgramiv(s_sceneProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(s_sceneProgram, 512, nullptr, infoLog);
        printf("Scene program linking failed:\n%s\n", infoLog);
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Get uniform locations
    s_modelLoc = glGetUniformLocation(s_sceneProgram, "uModel");
    s_viewLoc = glGetUniformLocation(s_sceneProgram, "uView");
    s_projLoc = glGetUniformLocation(s_sceneProgram, "uProjection");
    s_lightSpaceLoc = glGetUniformLocation(s_sceneProgram, "uLightSpace");
    s_shadowMapLoc = glGetUniformLocation(s_sceneProgram, "uShadowMap");
    s_lightDirLoc = glGetUniformLocation(s_sceneProgram, "uLightDir");
    s_lightColorLoc = glGetUniformLocation(s_sceneProgram, "uLightColor");
    s_ambientLoc = glGetUniformLocation(s_sceneProgram, "uAmbient");
    s_shadowBiasLoc = glGetUniformLocation(s_sceneProgram, "uShadowBias");
    s_cameraPosLoc = glGetUniformLocation(s_sceneProgram, "uCameraPos");
    
    // Material uniforms
    s_albedoLoc = glGetUniformLocation(s_sceneProgram, "uAlbedo");
    s_metallicLoc = glGetUniformLocation(s_sceneProgram, "uMetallic");
    s_roughnessLoc = glGetUniformLocation(s_sceneProgram, "uRoughness");
    s_aoLoc = glGetUniformLocation(s_sceneProgram, "uAO");
    s_hasAlbedoTexLoc = glGetUniformLocation(s_sceneProgram, "uHasAlbedoTex");
    s_albedoTexLoc = glGetUniformLocation(s_sceneProgram, "uAlbedoTex");

    printf("PBR Scene shader created:\n");
    printf("  model=%d, view=%d, proj=%d, lightSpace=%d, cameraPos=%d\n", 
           s_modelLoc, s_viewLoc, s_projLoc, s_lightSpaceLoc, s_cameraPosLoc);
    printf("  albedo=%d, metallic=%d, roughness=%d, ao=%d\n",
           s_albedoLoc, s_metallicLoc, s_roughnessLoc, s_aoLoc);

    s_shadersReady = true;
}

void DD_SceneRenderer::ClearShaders()
{
    if (s_sceneProgram)
    {
        glDeleteProgram(s_sceneProgram);
        s_sceneProgram = 0;
    }
    s_shadersReady = false;
}

DD_SceneRenderer::DD_SceneRenderer()
    : m_view(1.0f)
    , m_projection(1.0f)
    , m_lightSpaceMatrix(1.0f)
    , m_cameraPos(0.0f)
{
}

DD_SceneRenderer::~DD_SceneRenderer()
{
}

void DD_SceneRenderer::BeginScenePass(const Matrix4& view, const Matrix4& projection,
                                       const DD_LightComponent& light, GLuint shadowMap,
                                       const Matrix4& lightSpaceMatrix,
                                       const Vec3& cameraPos)
{
    m_view = view;
    m_projection = projection;
    m_lightSpaceMatrix = lightSpaceMatrix;
    m_cameraPos = cameraPos;

    glUseProgram(s_sceneProgram);

    // Set view/projection uniforms
    glUniformMatrix4fv(s_viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(s_projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(s_lightSpaceLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    glUniform3fv(s_cameraPosLoc, 1, glm::value_ptr(cameraPos));

    // Set light uniforms
    Vec3 lightDir = light.GetDirection();
    Vec3 lightColor = light.GetColor() * light.GetIntensity();
    glUniform3fv(s_lightDirLoc, 1, glm::value_ptr(lightDir));
    glUniform3fv(s_lightColorLoc, 1, glm::value_ptr(lightColor));
    glUniform1f(s_ambientLoc, light.GetAmbient());
    glUniform1f(s_shadowBiasLoc, light.GetShadowBias());

    // Bind shadow map to slot 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
    glUniform1i(s_shadowMapLoc, 0);
    
    // Albedo texture slot 1
    glUniform1i(s_albedoTexLoc, 1);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void DD_SceneRenderer::RenderActor(DD_Actor* actor)
{
    if (!actor) return;

    DD_MeshComponent* meshComp = actor->GetMeshComponent();
    if (!meshComp || !meshComp->IsVisible()) return;

    DD_Material* material = meshComp->GetMaterial();
    RenderActorWithMaterial(actor, material);
}

void DD_SceneRenderer::RenderActorWithMaterial(DD_Actor* actor, DD_Material* material)
{
    if (!actor) return;

    DD_MeshComponent* meshComp = actor->GetMeshComponent();
    if (!meshComp) return;

    DD_Mesh* mesh = meshComp->GetMesh();
    if (!mesh) return;

    Matrix4 model = actor->GetModelMatrix();
    glUniformMatrix4fv(s_modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // Set material uniforms
    if (material)
    {
        glUniform3fv(s_albedoLoc, 1, glm::value_ptr(material->GetAlbedo()));
        glUniform1f(s_metallicLoc, material->GetMetallic());
        glUniform1f(s_roughnessLoc, material->GetRoughness());
        glUniform1f(s_aoLoc, material->GetAO());
        
        if (material->HasAlbedoTexture())
        {
            glUniform1i(s_hasAlbedoTexLoc, 1);
            glActiveTexture(GL_TEXTURE1);
            material->GetAlbedoTexture()->Bind();
        }
        else
        {
            glUniform1i(s_hasAlbedoTexLoc, 0);
        }
    }
    else
    {
        // Default material
        glUniform3f(s_albedoLoc, 0.8f, 0.8f, 0.8f);
        glUniform1f(s_metallicLoc, 0.0f);
        glUniform1f(s_roughnessLoc, 0.5f);
        glUniform1f(s_aoLoc, 1.0f);
        glUniform1i(s_hasAlbedoTexLoc, 0);
    }

    // Bind mesh buffers and draw
#ifndef __EMSCRIPTEN__
    GLuint vao = mesh->GetVAO();
    if (vao) glBindVertexArray(vao);
#else
    glBindBuffer(GL_ARRAY_BUFFER, mesh->GetVertexBuffer());
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->GetIndexBuffer());
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(glm::vec2), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec3) + sizeof(glm::vec2), (void*)(sizeof(glm::vec3)));
#endif

    glDrawElements(GL_TRIANGLES, mesh->GetIndexCount(), GL_UNSIGNED_SHORT, 0);

#ifndef __EMSCRIPTEN__
    if (vao) glBindVertexArray(0);
#else
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
#endif
}

void DD_SceneRenderer::EndScenePass()
{
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
