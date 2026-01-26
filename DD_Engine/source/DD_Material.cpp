#include "DD_Material.h"
#include "DD_Texture.h"

DD_Material::DD_Material()
    : m_name("Default")
    , m_albedoTex(nullptr)
    , m_normalTex(nullptr)
    , m_metallicTex(nullptr)
    , m_roughnessTex(nullptr)
    , m_aoTex(nullptr)
    , m_emissiveTex(nullptr)
{
}

DD_Material::DD_Material(const std::string& name)
    : m_name(name)
    , m_albedoTex(nullptr)
    , m_normalTex(nullptr)
    , m_metallicTex(nullptr)
    , m_roughnessTex(nullptr)
    , m_aoTex(nullptr)
    , m_emissiveTex(nullptr)
{
}

DD_Material::~DD_Material()
{
}

void DD_Material::SetEmissive(const Vec3& color, float strength)
{
    m_properties.emissive = color;
    m_properties.emissiveStrength = strength;
}

void DD_Material::BindTextures() const
{
    // Slot 0 is reserved for shadow map
    // Textures start from slot 1
    
    if (m_albedoTex)
    {
        glActiveTexture(GL_TEXTURE1);
        m_albedoTex->Bind();
    }
    
    if (m_normalTex)
    {
        glActiveTexture(GL_TEXTURE2);
        m_normalTex->Bind();
    }
    
    if (m_metallicTex)
    {
        glActiveTexture(GL_TEXTURE3);
        m_metallicTex->Bind();
    }
    
    if (m_roughnessTex)
    {
        glActiveTexture(GL_TEXTURE4);
        m_roughnessTex->Bind();
    }
    
    if (m_aoTex)
    {
        glActiveTexture(GL_TEXTURE5);
        m_aoTex->Bind();
    }
    
    if (m_emissiveTex)
    {
        glActiveTexture(GL_TEXTURE6);
        m_emissiveTex->Bind();
    }
}

void DD_Material::UnbindTextures() const
{
    for (int i = 1; i <= 6; ++i)
    {
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    glActiveTexture(GL_TEXTURE0);
}

DD_Material* DD_Material::CreateDefault()
{
    auto* mat = new DD_Material("Default");
    mat->SetAlbedo(Vec3(0.8f, 0.8f, 0.8f));
    mat->SetMetallic(0.0f);
    mat->SetRoughness(0.5f);
    return mat;
}

DD_Material* DD_Material::CreateMetal(const Vec3& color, float roughness)
{
    auto* mat = new DD_Material("Metal");
    mat->SetAlbedo(color);
    mat->SetMetallic(1.0f);
    mat->SetRoughness(roughness);
    return mat;
}

DD_Material* DD_Material::CreatePlastic(const Vec3& color, float roughness)
{
    auto* mat = new DD_Material("Plastic");
    mat->SetAlbedo(color);
    mat->SetMetallic(0.0f);
    mat->SetRoughness(roughness);
    return mat;
}

DD_Material* DD_Material::CreateWood(const Vec3& color)
{
    auto* mat = new DD_Material("Wood");
    mat->SetAlbedo(color);
    mat->SetMetallic(0.0f);
    mat->SetRoughness(0.7f);
    return mat;
}

DD_Material* DD_Material::CreateGround()
{
    auto* mat = new DD_Material("Ground");
    mat->SetAlbedo(Vec3(0.3f, 0.35f, 0.25f));
    mat->SetMetallic(0.0f);
    mat->SetRoughness(0.9f);
    return mat;
}
