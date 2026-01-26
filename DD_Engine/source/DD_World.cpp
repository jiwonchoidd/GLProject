#include "DD_World.h"
#include "DD_Core.h"
#include "DD_Camera.h"
#include "DD_SimpleBox.h"
#include "DD_Actor.h"
#include "DD_CollisionComponent.h"
#include "DD_MeshComponent.h"
#include "DD_CollisionUtils.h"
#include "DD_DebugDraw.h"
#include "DD_LightActor.h"
#include "DD_LightComponent.h"
#include "DD_ShadowRenderer.h"
#include "DD_SceneRenderer.h"
#include "DD_DeferredRenderer.h"
#include "DD_GLDevice.h"
#include "DD_Material.h"
#include "DD_Texture.h"
#include <cstdio>
#include <glm/gtc/type_ptr.hpp>

DD_World::DD_World()
    : m_camera(std::make_unique<DD_Camera>())
    , m_sharedMesh(nullptr)
    , m_mainLight(nullptr)
    , m_shadowRenderer(std::make_unique<DD_ShadowRenderer>())
    , m_sceneRenderer(std::make_unique<DD_SceneRenderer>())
    , m_deferredRenderer(std::make_unique<DD_DeferredRenderer>())
    , m_shadowEnabled(true)
    , m_useDeferredRendering(true)
    , m_viewportWidth(1280)
    , m_viewportHeight(720)
    , m_simTime(0.0f)
{
}

DD_World::~DD_World()
{
    for (auto& mat : m_materials) delete mat;
    m_materials.clear();
    for (auto& tex : m_textures) delete tex;
    m_textures.clear();
}

void DD_World::Init(int width, int height)
{
    m_viewportWidth = width;
    m_viewportHeight = height;
    m_camera->UpdateProjection(width, height);
    m_camera->SetPosition(Vec3(15.0f, 12.0f, 15.0f));
    m_camera->SetRotationEuler(Vec3(-0.5f, -0.78f, 0.0f));

    DD_SimpleBox::CachePipline();
    DD_ShadowRenderer::CacheShaders();
    DD_SceneRenderer::CacheShaders();

#ifdef __EMSCRIPTEN__
    // WebGL2: Smaller shadow map
    if (!m_shadowRenderer->Initialize(1024))
#else
    if (!m_shadowRenderer->Initialize(4096))
#endif
    {
        printf("Warning: Shadow renderer initialization failed\n");
        m_shadowEnabled = false;
    }

    // Try deferred rendering on both platforms
    if (!m_deferredRenderer->Initialize(width, height))
    {
        printf("Warning: Deferred renderer initialization failed, using forward rendering\n");
        m_useDeferredRendering = false;
    }
    else
    {
        printf("Deferred rendering initialized successfully\n");
    }

    // Create sun light
    DD_LightActor* mainLight = CreateDirectionalLight();
    mainLight->SetName("Sun");
    mainLight->SetDirection(Vec3(-0.5f, -1.0f, -0.3f));
    mainLight->SetColor(Vec3(1.0f, 0.98f, 0.95f));
    mainLight->SetIntensity(1.2f);
    mainLight->SetAmbient(0.1f);
    mainLight->SetShadowOrthoSize(40.0f);
    mainLight->SetShadowNearFar(1.0f, 100.0f);
    mainLight->SetShadowBias(0.002f);
    mainLight->GetLightComponent()->SetFollowCamera(true);

    auto box = std::make_unique<DD_SimpleBox>();
    box->CreateMesh();
    m_sharedMesh = std::move(box);

    // Textures
    DD_Texture* checkerTex = new DD_Texture();
    checkerTex->CreateCheckerboard(256, Vec4(0.9f, 0.9f, 0.9f, 1.0f), Vec4(0.3f, 0.3f, 0.3f, 1.0f));
    m_textures.push_back(checkerTex);

    // Materials
    DD_Material* groundMat = DD_Material::CreateGround();
    groundMat->SetAlbedo(Vec3(0.5f, 0.5f, 0.5f));
    groundMat->SetRoughness(0.9f);
    groundMat->SetAlbedoTexture(checkerTex);
    m_materials.push_back(groundMat);

    DD_Material* redPlastic = DD_Material::CreatePlastic(Vec3(0.9f, 0.2f, 0.15f), 0.3f);
    m_materials.push_back(redPlastic);

    DD_Material* blueMetal = DD_Material::CreateMetal(Vec3(0.3f, 0.4f, 0.9f), 0.2f);
    m_materials.push_back(blueMetal);

    DD_Material* goldMetal = DD_Material::CreateMetal(Vec3(1.0f, 0.85f, 0.4f), 0.25f);
    m_materials.push_back(goldMetal);

    DD_Material* whitePlastic = DD_Material::CreatePlastic(Vec3(0.95f, 0.95f, 0.95f), 0.4f);
    m_materials.push_back(whitePlastic);

    // Ground
    {
        auto comp = std::make_unique<DD_MeshComponent>();
        comp->SetMesh(m_sharedMesh.get());
        comp->SetMaterial(groundMat);
        comp->SetCastShadow(false);
        m_meshComponents.push_back(std::move(comp));

        auto actor = std::make_unique<DD_Actor>();
        actor->SetName("Ground");
        actor->SetMeshComponent(m_meshComponents.back().get());
        actor->SetPosition(Vec3(0.0f, -1.5f, 0.0f));
        actor->SetScale(Vec3(30.0f, 0.2f, 30.0f));
        m_actors.push_back(std::move(actor));
    }

    // Scene objects
    struct ObjectDef { Vec3 pos; Vec3 scale; DD_Material* mat; const char* name; };
    std::vector<ObjectDef> objects = {
        { Vec3(0.0f, 2.0f, 0.0f), Vec3(1.5f, 3.0f, 1.5f), whitePlastic, "Tower" },
        { Vec3(-4.0f, 0.5f, -4.0f), Vec3(1.2f), redPlastic, "RedCube" },
        { Vec3(4.0f, 0.5f, -4.0f), Vec3(1.2f), blueMetal, "BlueCube" },
        { Vec3(-4.0f, 0.5f, 4.0f), Vec3(1.2f), goldMetal, "GoldCube" },
        { Vec3(4.0f, 0.5f, 4.0f), Vec3(1.2f), whitePlastic, "WhiteCube" },
        { Vec3(-2.0f, 3.0f, 0.0f), Vec3(0.8f), redPlastic, "FloatingRed" },
        { Vec3(2.0f, 4.0f, 0.0f), Vec3(0.8f), blueMetal, "FloatingBlue" },
        { Vec3(0.0f, 5.5f, -2.0f), Vec3(0.6f), goldMetal, "FloatingGold" },
    };

    m_movers.resize(objects.size());
    for (size_t i = 0; i < objects.size(); ++i)
    {
        const auto& obj = objects[i];
        auto meshComp = std::make_unique<DD_MeshComponent>();
        meshComp->SetMesh(m_sharedMesh.get());
        meshComp->SetMaterial(obj.mat);
        m_meshComponents.push_back(std::move(meshComp));

        auto actor = std::make_unique<DD_Actor>();
        actor->SetName(obj.name);
        actor->SetMeshComponent(m_meshComponents.back().get());
        actor->SetPosition(obj.pos);
        actor->SetScale(obj.scale);

        if (std::string(obj.name).find("Floating") != std::string::npos)
        {
            auto& mv = m_movers[i];
            mv.enabled = true;
            mv.basePosition = obj.pos;
            mv.amplitude = 0.5f;
            mv.frequency = 0.8f + i * 0.1f;
            mv.phase = i * 0.5f;
            mv.axis = Vec3(0.0f, 1.0f, 0.0f);
        }
        m_actors.push_back(std::move(actor));
    }

    // Add point lights around the scene
    CreatePointLight(Vec3(-5.0f, 2.0f, -5.0f), Vec3(1.0f, 0.3f, 0.1f), 3.0f, 8.0f);  // Red-orange
    CreatePointLight(Vec3(5.0f, 2.0f, -5.0f), Vec3(0.1f, 0.5f, 1.0f), 3.0f, 8.0f);   // Blue
    CreatePointLight(Vec3(-5.0f, 2.0f, 5.0f), Vec3(0.1f, 1.0f, 0.3f), 3.0f, 8.0f);   // Green
    CreatePointLight(Vec3(5.0f, 2.0f, 5.0f), Vec3(1.0f, 0.9f, 0.3f), 3.0f, 8.0f);    // Yellow

    printf("Created %zu actors, %zu lights (1 directional + %zu point), deferred=%s\n", 
           m_actors.size(), m_lights.size(), m_lights.size() - 1,
           m_useDeferredRendering ? "ON" : "OFF");
}

DD_LightActor* DD_World::CreateDirectionalLight()
{
    auto lightActor = std::make_unique<DD_LightActor>();
    lightActor->SetLightType(LightType::Directional);
    lightActor->SetCastShadow(true);
    DD_LightActor* ptr = lightActor.get();
    m_lights.push_back(ptr);
    if (!m_mainLight) m_mainLight = ptr;
    m_actors.push_back(std::move(lightActor));
    return ptr;
}

DD_LightActor* DD_World::CreatePointLight(const Vec3& position, const Vec3& color, float intensity, float radius)
{
    auto lightActor = std::make_unique<DD_LightActor>();
    lightActor->SetLightType(LightType::Point);
    lightActor->SetPosition(position);
    lightActor->SetColor(color);
    lightActor->SetIntensity(intensity);
    lightActor->GetLightComponent()->SetRange(radius);
    lightActor->SetCastShadow(false);
    DD_LightActor* ptr = lightActor.get();
    m_lights.push_back(ptr);
    m_actors.push_back(std::move(lightActor));
    return ptr;
}

void DD_World::AddActor(DD_Actor* actor) {}

void DD_World::RemoveActor(DD_Actor* actor)
{
    auto lightIt = std::find(m_lights.begin(), m_lights.end(), actor);
    if (lightIt != m_lights.end())
    {
        if (m_mainLight == *lightIt) m_mainLight = nullptr;
        m_lights.erase(lightIt);
    }
    auto it = std::find_if(m_actors.begin(), m_actors.end(),
        [actor](const std::unique_ptr<DD_Actor>& ptr) { return ptr.get() == actor; });
    if (it != m_actors.end()) m_actors.erase(it);
}

void DD_World::Update(float deltaTime)
{
    m_simTime += deltaTime;
    for (auto& actorPtr : m_actors) actorPtr->Update(deltaTime);

    size_t actorOffset = 2;  // Skip ground and light
    for (size_t i = 0; i < m_movers.size(); ++i)
    {
        size_t actorIdx = actorOffset + i;
        if (actorIdx >= m_actors.size()) break;
        auto& mv = m_movers[i];
        if (!mv.enabled) continue;
        float v = cosf(m_simTime * mv.frequency + mv.phase);
        m_actors[actorIdx]->SetPosition(mv.basePosition + mv.axis * (mv.amplitude * v));
    }
    PostTick(deltaTime);
}

void DD_World::PostTick(float deltaTime)
{
    m_camera->UpdateView();
    
    // Update directional light to follow camera
    if (m_mainLight)
    {
        m_mainLight->GetLightComponent()->SetCameraPosition(m_camera->GetPosition());
    }
    
    ProcessCollisions(deltaTime);
}

void DD_World::Resize(int width, int height)
{
    m_viewportWidth = width;
    m_viewportHeight = height;
    m_camera->UpdateProjection(width, height);
    if (m_deferredRenderer) m_deferredRenderer->Resize(width, height);
}

void DD_World::Render()
{
    if (m_shadowEnabled && m_mainLight && m_mainLight->GetCastShadow())
    {
        RenderShadowPass();
    }

    if (m_useDeferredRendering)
    {
        RenderDeferred();
    }
    else if (m_shadowEnabled && m_mainLight)
    {
        RenderScenePass();
    }
    else
    {
        RenderLegacy();
    }

    if (DebugDraw::Enabled)
    {
        const Matrix4 view = m_camera->GetViewMatrix();
        const Matrix4 proj = m_camera->GetProjectionMatrix();
        for (auto& actorPtr : m_actors)
        {
            DD_CollisionComponent* col = actorPtr->GetCollisionComponent();
            if (col && col->m_type == CollisionShapeType::AABB)
            {
                AABB aabb{actorPtr->GetPosition(), col->m_aabbHalfExtents};
                DebugDraw::DrawAABBWire(aabb, view, proj, Vec4(1.0f, 0.0f, 0.0f, 1.0f));
            }
        }
    }
}

void DD_World::RenderShadowPass()
{
    if (!m_mainLight) return;
    DD_LightComponent* lightComp = m_mainLight->GetLightComponent();
    m_shadowRenderer->BeginShadowPass(*lightComp);

    for (auto& actorPtr : m_actors)
    {
        if (dynamic_cast<DD_LightActor*>(actorPtr.get())) continue;
        DD_MeshComponent* meshComp = actorPtr->GetMeshComponent();
        if (meshComp && !meshComp->GetCastShadow()) continue;
        m_shadowRenderer->RenderActor(actorPtr.get());
    }
    m_shadowRenderer->EndShadowPass();
}

void DD_World::RenderDeferred()
{
    const Matrix4 view = m_camera->GetViewMatrix();
    const Matrix4 proj = m_camera->GetProjectionMatrix();
    Vec3 cameraPos = m_camera->GetPosition();

    // Geometry pass
    m_deferredRenderer->BeginGeometryPass(view, proj);
    for (auto& actorPtr : m_actors)
    {
        if (dynamic_cast<DD_LightActor*>(actorPtr.get())) continue;
        m_deferredRenderer->RenderActor(actorPtr.get());
    }
    m_deferredRenderer->EndGeometryPass();

    // Lighting pass
    glViewport(0, 0, m_viewportWidth, m_viewportHeight);
    m_deferredRenderer->ClearPointLights();

    // Add point lights
    for (auto* light : m_lights)
    {
        if (light->GetLightComponent()->GetLightType() == LightType::Point)
        {
            DD_LightComponent* lc = light->GetLightComponent();
            m_deferredRenderer->AddPointLight(
                light->GetPosition(),
                lc->GetColor(),
                lc->GetIntensity(),
                lc->GetRange()
            );
        }
    }

    m_deferredRenderer->BeginLightingPass(cameraPos);
    if (m_mainLight)
    {
        DD_LightComponent* lightComp = m_mainLight->GetLightComponent();
        m_deferredRenderer->SetDirectionalLight(
            lightComp,
            m_shadowRenderer->GetShadowMap(),
            m_shadowRenderer->GetLightSpaceMatrix()
        );
    }
    m_deferredRenderer->EndLightingPass();
}

void DD_World::RenderScenePass()
{
    if (!m_mainLight) return;
    glViewport(0, 0, m_viewportWidth, m_viewportHeight);

    const Matrix4 view = m_camera->GetViewMatrix();
    const Matrix4 proj = m_camera->GetProjectionMatrix();
    Vec3 cameraPos = m_camera->GetPosition();

    DD_LightComponent* lightComp = m_mainLight->GetLightComponent();
    m_sceneRenderer->BeginScenePass(view, proj, *lightComp, 
        m_shadowRenderer->GetShadowMap(), m_shadowRenderer->GetLightSpaceMatrix(), cameraPos);

    for (auto& actorPtr : m_actors)
    {
        if (dynamic_cast<DD_LightActor*>(actorPtr.get())) continue;
        m_sceneRenderer->RenderActor(actorPtr.get());
    }
    m_sceneRenderer->EndScenePass();
}

void DD_World::RenderLegacy()
{
    const Matrix4 view = m_camera->GetViewMatrix();
    const Matrix4 proj = m_camera->GetProjectionMatrix();
    for (auto& actorPtr : m_actors) actorPtr->Render(view, proj);
}

void DD_World::ProcessCollisions(float deltaTime)
{
    size_t n = m_actors.size();
    for (size_t i = 0; i < n; ++i)
    {
        for (size_t j = i + 1; j < n; ++j)
        {
            DD_CollisionComponent* a = m_actors[i]->GetCollisionComponent();
            DD_CollisionComponent* b = m_actors[j]->GetCollisionComponent();
            if (!a || !b) continue;

            if (a->m_type == CollisionShapeType::AABB && b->m_type == CollisionShapeType::AABB)
            {
                AABB A{m_actors[i]->GetPosition(), a->m_aabbHalfExtents};
                AABB B{m_actors[j]->GetPosition(), b->m_aabbHalfExtents};
                Vec3 mtv;
                if (CollisionUtils::TestAABBvsAABB(A, B, mtv))
                {
                    float ma = a->m_mass, mb = b->m_mass;
                    float total = ma + mb;
                    if (total < DD_SMALL_NUMBER) continue;
                    m_actors[i]->AddPosition(-mtv * (mb / total));
                    m_actors[j]->AddPosition(mtv * (ma / total));
                }
            }
        }
    }
}

void DD_World::SetDebugDraw(bool enabled) { DebugDraw::Enabled = enabled; }
