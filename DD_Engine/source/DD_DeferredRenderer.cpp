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

    const char* geoVertSrc = nullptr;
    const char* geoFragSrc = nullptr;
    const char* litVertSrc = nullptr;
    const char* litFragSrc = nullptr;

#ifdef __EMSCRIPTEN__
    // WebGL2 / GLSL ES 3.00
    geoVertSrc =
        "#version 300 es\n"
        "precision highp float;\n"
        "layout(location = 0) in vec3 aPos;\n"
        "layout(location = 1) in vec3 aNormal;\n"
        "layout(location = 2) in vec2 aTexCoord;\n"
        "uniform mat4 uModel;\n"
        "uniform mat4 uView;\n"
        "uniform mat4 uProjection;\n"
        "out vec3 vWorldPos;\n"
        "out vec3 vNormal;\n"
        "out vec2 vTexCoord;\n"
        "void main() {\n"
        "    vec4 worldPos = uModel * vec4(aPos, 1.0);\n"
        "    vWorldPos = worldPos.xyz;\n"
        "    mat3 normalMatrix = transpose(inverse(mat3(uModel)));\n"
        "    vNormal = normalize(normalMatrix * aNormal);\n"
        "    vTexCoord = aTexCoord;\n"
        "    gl_Position = uProjection * uView * worldPos;\n"
        "}\n";

    geoFragSrc =
        "#version 300 es\n"
        "precision highp float;\n"
        "layout(location = 0) out vec4 gPosition;\n"
        "layout(location = 1) out vec4 gNormal;\n"
        "layout(location = 2) out vec4 gAlbedo;\n"
        "layout(location = 3) out vec4 gMaterial;\n"
        "in vec3 vWorldPos;\n"
        "in vec3 vNormal;\n"
        "in vec2 vTexCoord;\n"
        "uniform vec3 uAlbedo;\n"
        "uniform float uMetallic;\n"
        "uniform float uRoughness;\n"
        "uniform float uAO;\n"
        "uniform int uHasAlbedoTex;\n"
        "uniform sampler2D uAlbedoTex;\n"
        "void main() {\n"
        "    gPosition = vec4(vWorldPos, 1.0);\n"
        "    gNormal = vec4(normalize(vNormal), 0.0);\n"
        "    vec3 albedo = uAlbedo;\n"
        "    if (uHasAlbedoTex == 1) { albedo *= texture(uAlbedoTex, vTexCoord).rgb; }\n"
        "    gAlbedo = vec4(albedo, uAO);\n"
        "    gMaterial = vec4(uMetallic, uRoughness, 0.0, 1.0);\n"
        "}\n";

    litVertSrc =
        "#version 300 es\n"
        "precision highp float;\n"
        "layout(location = 0) in vec2 aPos;\n"
        "layout(location = 1) in vec2 aTexCoord;\n"
        "out vec2 vTexCoord;\n"
        "void main() { vTexCoord = aTexCoord; gl_Position = vec4(aPos, 0.0, 1.0); }\n";

    litFragSrc =
        "#version 300 es\n"
        "precision highp float;\n"
        "out vec4 fragColor;\n"
        "in vec2 vTexCoord;\n"
        "uniform sampler2D gPosition;\n"
        "uniform sampler2D gNormal;\n"
        "uniform sampler2D gAlbedo;\n"
        "uniform sampler2D gMaterial;\n"
        "uniform sampler2D uShadowMap;\n"
        "uniform vec3 uCameraPos;\n"
        "uniform mat4 uLightSpace;\n"
        "uniform vec3 uDirLightDir;\n"
        "uniform vec3 uDirLightColor;\n"
        "uniform float uAmbient;\n"
        "uniform float uShadowBias;\n"
        "#define MAX_POINT_LIGHTS 32\n"
        "struct PointLight { vec3 position; float radius; vec3 color; float intensity; };\n"
        "uniform int uPointLightCount;\n"
        "uniform PointLight uPointLights[MAX_POINT_LIGHTS];\n"
        "const float PI = 3.14159265359;\n"
        "float DistributionGGX(vec3 N, vec3 H, float r) { float a=r*r; float a2=a*a; float NdotH=max(dot(N,H),0.0); float d=(NdotH*NdotH*(a2-1.0)+1.0); return a2/(PI*d*d+0.0001); }\n"
        "float GeomSchlick(float NdotV, float r) { float k=(r+1.0)*(r+1.0)/8.0; return NdotV/(NdotV*(1.0-k)+k+0.0001); }\n"
        "float GeomSmith(vec3 N, vec3 V, vec3 L, float r) { return GeomSchlick(max(dot(N,V),0.0),r)*GeomSchlick(max(dot(N,L),0.0),r); }\n"
        "vec3 FresnelSchlick(float c, vec3 F0) { return F0+(1.0-F0)*pow(clamp(1.0-c,0.0,1.0),5.0); }\n"
        "float Shadow(vec4 lsp) { vec3 p=lsp.xyz/lsp.w*0.5+0.5; if(p.z>1.0)return 0.0; float d=texture(uShadowMap,p.xy).r; return p.z-uShadowBias>d?1.0:0.0; }\n"
        "vec3 CalcPBR(vec3 N, vec3 V, vec3 L, vec3 alb, float met, float rou, vec3 rad) {\n"
        "    vec3 H=normalize(V+L); vec3 F0=mix(vec3(0.04),alb,met);\n"
        "    float NdotL=max(dot(N,L),0.0);\n"
        "    float NDF=DistributionGGX(N,H,rou); float G=GeomSmith(N,V,L,rou); vec3 F=FresnelSchlick(max(dot(H,V),0.0),F0);\n"
        "    vec3 spec=(NDF*G*F)/(4.0*max(dot(N,V),0.0)*NdotL+0.0001);\n"
        "    vec3 kD=(1.0-F)*(1.0-met);\n"
        "    return (kD*alb/PI+spec)*rad*NdotL;\n"
        "}\n"
        "void main() {\n"
        "    vec3 pos=texture(gPosition,vTexCoord).rgb;\n"
        "    vec3 N=normalize(texture(gNormal,vTexCoord).rgb);\n"
        "    vec3 alb=texture(gAlbedo,vTexCoord).rgb;\n"
        "    float ao=texture(gAlbedo,vTexCoord).a;\n"
        "    float met=texture(gMaterial,vTexCoord).r;\n"
        "    float rou=max(texture(gMaterial,vTexCoord).g,0.04);\n"
        "    if(length(pos)<0.001){fragColor=vec4(0.1,0.1,0.15,1.0);return;}\n"
        "    vec3 V=normalize(uCameraPos-pos);\n"
        "    vec3 L=normalize(-uDirLightDir);\n"
        "    float sh=Shadow(uLightSpace*vec4(pos,1.0));\n"
        "    vec3 Lo=CalcPBR(N,V,L,alb,met,rou,uDirLightColor)*(1.0-sh);\n"
        "    for(int i=0;i<uPointLightCount;++i){\n"
        "        vec3 pL=normalize(uPointLights[i].position-pos);\n"
        "        float dist=length(uPointLights[i].position-pos);\n"
        "        if(dist<uPointLights[i].radius){\n"
        "            float att=1.0/(1.0+0.09*dist+0.032*dist*dist);\n"
        "            att*=1.0-smoothstep(uPointLights[i].radius*0.5,uPointLights[i].radius,dist);\n"
        "            vec3 rad=uPointLights[i].color*uPointLights[i].intensity*att;\n"
        "            Lo+=CalcPBR(N,V,pL,alb,met,rou,rad);\n"
        "        }\n"
        "    }\n"
        "    vec3 ambient=uAmbient*alb*ao;\n"
        "    vec3 color=ambient+Lo;\n"
        "    color=color/(color+vec3(1.0));\n"
        "    color=pow(color,vec3(1.0/2.2));\n"
        "    fragColor=vec4(color,1.0);\n"
        "}\n";
#else
    // Desktop OpenGL 3.3 Core
    geoVertSrc = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aTexCoord;
        uniform mat4 uModel, uView, uProjection;
        out vec3 vWorldPos, vNormal;
        out vec2 vTexCoord;
        void main() {
            vec4 wp = uModel * vec4(aPos, 1.0);
            vWorldPos = wp.xyz;
            vNormal = normalize(transpose(inverse(mat3(uModel))) * aNormal);
            vTexCoord = aTexCoord;
            gl_Position = uProjection * uView * wp;
        }
    )";

    geoFragSrc = R"(
        #version 330 core
        layout(location = 0) out vec4 gPosition;
        layout(location = 1) out vec4 gNormal;
        layout(location = 2) out vec4 gAlbedo;
        layout(location = 3) out vec4 gMaterial;
        in vec3 vWorldPos, vNormal;
        in vec2 vTexCoord;
        uniform vec3 uAlbedo;
        uniform float uMetallic, uRoughness, uAO;
        uniform int uHasAlbedoTex;
        uniform sampler2D uAlbedoTex;
        void main() {
            gPosition = vec4(vWorldPos, 1.0);
            gNormal = vec4(normalize(vNormal), 0.0);
            vec3 alb = uAlbedo;
            if (uHasAlbedoTex == 1) alb *= texture(uAlbedoTex, vTexCoord).rgb;
            gAlbedo = vec4(alb, uAO);
            gMaterial = vec4(uMetallic, uRoughness, 0.0, 1.0);
        }
    )";

    litVertSrc = R"(
        #version 330 core
        layout(location = 0) in vec2 aPos;
        layout(location = 1) in vec2 aTexCoord;
        out vec2 vTexCoord;
        void main() { vTexCoord = aTexCoord; gl_Position = vec4(aPos, 0.0, 1.0); }
    )";

    litFragSrc = R"(
        #version 330 core
        out vec4 fragColor;
        in vec2 vTexCoord;
        uniform sampler2D gPosition, gNormal, gAlbedo, gMaterial;
        uniform sampler2DShadow uShadowMap;
        uniform vec3 uCameraPos, uDirLightDir, uDirLightColor;
        uniform mat4 uLightSpace;
        uniform float uAmbient, uShadowBias;
        #define MAX_POINT_LIGHTS 32
        struct PointLight { vec3 position; float radius; vec3 color; float intensity; };
        uniform int uPointLightCount;
        uniform PointLight uPointLights[MAX_POINT_LIGHTS];
        const float PI = 3.14159265359;
        float DistGGX(vec3 N, vec3 H, float r) { float a=r*r,a2=a*a,NdotH=max(dot(N,H),0.0),d=(NdotH*NdotH*(a2-1.0)+1.0); return a2/(PI*d*d+0.0001); }
        float GeomSchlick(float NdotV, float r) { float k=(r+1.0)*(r+1.0)/8.0; return NdotV/(NdotV*(1.0-k)+k+0.0001); }
        float GeomSmith(vec3 N, vec3 V, vec3 L, float r) { return GeomSchlick(max(dot(N,V),0.0),r)*GeomSchlick(max(dot(N,L),0.0),r); }
        vec3 Fresnel(float c, vec3 F0) { return F0+(1.0-F0)*pow(clamp(1.0-c,0.0,1.0),5.0); }
        float Shadow(vec4 lsp) {
            vec3 p=lsp.xyz/lsp.w*0.5+0.5; if(p.z>1.0)return 0.0;
            float sh=0.0; vec2 ts=1.0/textureSize(uShadowMap,0);
            for(int x=-1;x<=1;++x) for(int y=-1;y<=1;++y) sh+=1.0-texture(uShadowMap,vec3(p.xy+vec2(x,y)*ts,p.z-uShadowBias));
            return sh/9.0;
        }
        vec3 CalcPBR(vec3 N, vec3 V, vec3 L, vec3 alb, float met, float rou, vec3 rad) {
            vec3 H=normalize(V+L),F0=mix(vec3(0.04),alb,met);
            float NdotL=max(dot(N,L),0.0);
            vec3 F=Fresnel(max(dot(H,V),0.0),F0);
            vec3 spec=(DistGGX(N,H,rou)*GeomSmith(N,V,L,rou)*F)/(4.0*max(dot(N,V),0.0)*NdotL+0.0001);
            return ((1.0-F)*(1.0-met)*alb/PI+spec)*rad*NdotL;
        }
        void main() {
            vec3 pos=texture(gPosition,vTexCoord).rgb,N=normalize(texture(gNormal,vTexCoord).rgb);
            vec3 alb=texture(gAlbedo,vTexCoord).rgb; float ao=texture(gAlbedo,vTexCoord).a;
            float met=texture(gMaterial,vTexCoord).r,rou=max(texture(gMaterial,vTexCoord).g,0.04);
            if(length(pos)<0.001){fragColor=vec4(0.1,0.1,0.15,1.0);return;}
            vec3 V=normalize(uCameraPos-pos),L=normalize(-uDirLightDir);
            float sh=Shadow(uLightSpace*vec4(pos,1.0));
            vec3 Lo=CalcPBR(N,V,L,alb,met,rou,uDirLightColor)*(1.0-sh);
            for(int i=0;i<uPointLightCount;++i){
                vec3 pL=normalize(uPointLights[i].position-pos);
                float dist=length(uPointLights[i].position-pos);
                if(dist<uPointLights[i].radius){
                    float att=1.0/(1.0+0.09*dist+0.032*dist*dist)*(1.0-smoothstep(uPointLights[i].radius*0.5,uPointLights[i].radius,dist));
                    Lo+=CalcPBR(N,V,pL,alb,met,rou,uPointLights[i].color*uPointLights[i].intensity*att);
                }
            }
            vec3 color=uAmbient*alb*ao+Lo;
            color=color/(color+vec3(1.0));
            fragColor=vec4(pow(color,vec3(1.0/2.2)),1.0);
        }
    )";
#endif

    // Compile geometry shader
    GLuint geoVert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(geoVert, 1, &geoVertSrc, nullptr);
    glCompileShader(geoVert);
    GLint success;
    glGetShaderiv(geoVert, GL_COMPILE_STATUS, &success);
    if (!success) { char log[512]; glGetShaderInfoLog(geoVert, 512, nullptr, log); printf("Geo vert error: %s\n", log); return false; }

    GLuint geoFrag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(geoFrag, 1, &geoFragSrc, nullptr);
    glCompileShader(geoFrag);
    glGetShaderiv(geoFrag, GL_COMPILE_STATUS, &success);
    if (!success) { char log[512]; glGetShaderInfoLog(geoFrag, 512, nullptr, log); printf("Geo frag error: %s\n", log); return false; }

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

    // Compile lighting shader
    GLuint litVert = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(litVert, 1, &litVertSrc, nullptr);
    glCompileShader(litVert);
    glGetShaderiv(litVert, GL_COMPILE_STATUS, &success);
    if (!success) { char log[512]; glGetShaderInfoLog(litVert, 512, nullptr, log); printf("Lit vert error: %s\n", log); return false; }

    GLuint litFrag = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(litFrag, 1, &litFragSrc, nullptr);
    glCompileShader(litFrag);
    glGetShaderiv(litFrag, GL_COMPILE_STATUS, &success);
    if (!success) { char log[512]; glGetShaderInfoLog(litFrag, 512, nullptr, log); printf("Lit frag error: %s\n", log); return false; }

    s_lightingProgram = glCreateProgram();
    glAttachShader(s_lightingProgram, litVert);
    glAttachShader(s_lightingProgram, litFrag);
    glLinkProgram(s_lightingProgram);
    glDeleteShader(litVert);
    glDeleteShader(litFrag);

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

DD_DeferredRenderer::DD_DeferredRenderer() : m_view(1.0f), m_projection(1.0f), m_pointLightCount(0), m_quadVAO(0), m_quadVBO(0) {}
DD_DeferredRenderer::~DD_DeferredRenderer() { Shutdown(); }

bool DD_DeferredRenderer::Initialize(int width, int height)
{
    if (!CacheShaders()) return false;
    m_gBuffer = std::make_unique<DD_GBuffer>();
    if (!m_gBuffer->Initialize(width, height)) return false;

    float quadVertices[] = { -1,1,0,1, -1,-1,0,0, 1,-1,1,0, -1,1,0,1, 1,-1,1,0, 1,1,1,1 };
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(2*sizeof(float)));
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

void DD_DeferredRenderer::Resize(int width, int height) { if (m_gBuffer) m_gBuffer->Resize(width, height); }

void DD_DeferredRenderer::BeginGeometryPass(const Matrix4& view, const Matrix4& projection)
{
    m_view = view; m_projection = projection;
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

    glUniformMatrix4fv(s_geoModelLoc, 1, GL_FALSE, glm::value_ptr(actor->GetModelMatrix()));

    DD_Material* mat = meshComp->GetMaterial();
    if (mat) {
        glUniform3fv(s_geoAlbedoLoc, 1, glm::value_ptr(mat->GetAlbedo()));
        glUniform1f(s_geoMetallicLoc, mat->GetMetallic());
        glUniform1f(s_geoRoughnessLoc, mat->GetRoughness());
        glUniform1f(s_geoAOLoc, mat->GetAO());
        if (mat->HasAlbedoTexture()) { glUniform1i(s_geoHasAlbedoTexLoc, 1); glActiveTexture(GL_TEXTURE0); mat->GetAlbedoTexture()->Bind(); }
        else { glUniform1i(s_geoHasAlbedoTexLoc, 0); }
    } else {
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

void DD_DeferredRenderer::EndGeometryPass() { m_gBuffer->UnbindGeometryPass(); }

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
    glUniform3fv(s_litDirLightDirLoc, 1, glm::value_ptr(light->GetDirection()));
    glUniform3fv(s_litDirLightColorLoc, 1, glm::value_ptr(light->GetColor() * light->GetIntensity()));
    glUniform1f(s_litDirLightAmbientLoc, light->GetAmbient());
    glUniform1f(s_litShadowBiasLoc, light->GetShadowBias());
    glUniformMatrix4fv(s_litLightSpaceLoc, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, shadowMap);
}

void DD_DeferredRenderer::AddPointLight(const Vec3& position, const Vec3& color, float intensity, float radius)
{
    if (m_pointLightCount >= MAX_POINT_LIGHTS) return;
    m_pointLights[m_pointLightCount] = {position, radius, color, intensity};
    m_pointLightCount++;
}

void DD_DeferredRenderer::EndLightingPass()
{
    glUniform1i(s_litPointLightCountLoc, m_pointLightCount);
    for (int i = 0; i < m_pointLightCount; ++i) {
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
