#pragma once
#include "DD_GLHelper.h"
#include <string>

class DD_Texture;

// PBR/BSDF Material properties
struct MaterialProperties
{
    Vec3 albedo;           // Base color
    float metallic;        // Metalness (0 = dielectric, 1 = metal)
    float roughness;       // Surface roughness (0 = smooth, 1 = rough)
    float ao;              // Ambient occlusion
    Vec3 emissive;         // Emissive color
    float emissiveStrength;
    float opacity;         // Transparency
    float ior;             // Index of refraction (for glass/water)
    
    MaterialProperties()
        : albedo(0.8f, 0.8f, 0.8f)
        , metallic(0.0f)
        , roughness(0.5f)
        , ao(1.0f)
        , emissive(0.0f)
        , emissiveStrength(0.0f)
        , opacity(1.0f)
        , ior(1.5f)
    {}
};

class DD_Material
{
public:
    DD_Material();
    DD_Material(const std::string& name);
    ~DD_Material();

    // Name
    void SetName(const std::string& name) { m_name = name; }
    const std::string& GetName() const { return m_name; }

    // Properties
    void SetAlbedo(const Vec3& color) { m_properties.albedo = color; }
    Vec3 GetAlbedo() const { return m_properties.albedo; }

    void SetMetallic(float value) { m_properties.metallic = glm::clamp(value, 0.0f, 1.0f); }
    float GetMetallic() const { return m_properties.metallic; }

    void SetRoughness(float value) { m_properties.roughness = glm::clamp(value, 0.04f, 1.0f); }
    float GetRoughness() const { return m_properties.roughness; }

    void SetAO(float value) { m_properties.ao = value; }
    float GetAO() const { return m_properties.ao; }

    void SetEmissive(const Vec3& color, float strength = 1.0f);
    Vec3 GetEmissive() const { return m_properties.emissive; }
    float GetEmissiveStrength() const { return m_properties.emissiveStrength; }

    void SetOpacity(float value) { m_properties.opacity = glm::clamp(value, 0.0f, 1.0f); }
    float GetOpacity() const { return m_properties.opacity; }

    void SetIOR(float value) { m_properties.ior = value; }
    float GetIOR() const { return m_properties.ior; }

    const MaterialProperties& GetProperties() const { return m_properties; }

    // Textures
    void SetAlbedoTexture(DD_Texture* texture) { m_albedoTex = texture; }
    void SetNormalTexture(DD_Texture* texture) { m_normalTex = texture; }
    void SetMetallicTexture(DD_Texture* texture) { m_metallicTex = texture; }
    void SetRoughnessTexture(DD_Texture* texture) { m_roughnessTex = texture; }
    void SetAOTexture(DD_Texture* texture) { m_aoTex = texture; }
    void SetEmissiveTexture(DD_Texture* texture) { m_emissiveTex = texture; }

    DD_Texture* GetAlbedoTexture() const { return m_albedoTex; }
    DD_Texture* GetNormalTexture() const { return m_normalTex; }
    DD_Texture* GetMetallicTexture() const { return m_metallicTex; }
    DD_Texture* GetRoughnessTexture() const { return m_roughnessTex; }
    DD_Texture* GetAOTexture() const { return m_aoTex; }
    DD_Texture* GetEmissiveTexture() const { return m_emissiveTex; }

    bool HasAlbedoTexture() const { return m_albedoTex != nullptr; }
    bool HasNormalTexture() const { return m_normalTex != nullptr; }

    // Bind textures to shader slots
    void BindTextures() const;
    void UnbindTextures() const;

    // Preset materials
    static DD_Material* CreateDefault();
    static DD_Material* CreateMetal(const Vec3& color, float roughness = 0.3f);
    static DD_Material* CreatePlastic(const Vec3& color, float roughness = 0.4f);
    static DD_Material* CreateWood(const Vec3& color);
    static DD_Material* CreateGround();

private:
    std::string m_name;
    MaterialProperties m_properties;

    // Texture maps (non-owning pointers)
    DD_Texture* m_albedoTex;
    DD_Texture* m_normalTex;
    DD_Texture* m_metallicTex;
    DD_Texture* m_roughnessTex;
    DD_Texture* m_aoTex;
    DD_Texture* m_emissiveTex;
};
