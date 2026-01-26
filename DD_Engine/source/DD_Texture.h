#pragma once
#include "DD_GLHelper.h"
#include <string>

enum class TextureType
{
    Albedo,
    Normal,
    Metallic,
    Roughness,
    AO,
    Emissive,
    Height,
    Custom
};

enum class TextureWrap
{
    Repeat,
    ClampToEdge,
    MirroredRepeat
};

enum class TextureFilter
{
    Nearest,
    Linear,
    LinearMipmap
};

class DD_Texture
{
public:
    DD_Texture();
    ~DD_Texture();

    // Load from file
    bool LoadFromFile(const std::string& filepath);
    
    // Create from raw data
    bool Create(int width, int height, int channels, const unsigned char* data);
    
    // Create solid color texture
    bool CreateSolidColor(const Vec4& color);
    
    // Create checkerboard pattern (useful for testing)
    bool CreateCheckerboard(int size, const Vec4& color1, const Vec4& color2);

    void Destroy();

    void Bind(int slot = 0) const;
    void Unbind() const;

    // Settings
    void SetWrap(TextureWrap wrap);
    void SetFilter(TextureFilter filter);
    void SetType(TextureType type) { m_type = type; }

    // Getters
    GLuint GetHandle() const { return m_texture; }
    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    int GetChannels() const { return m_channels; }
    TextureType GetType() const { return m_type; }
    const std::string& GetFilePath() const { return m_filepath; }
    bool IsValid() const { return m_texture != 0; }

    // Generate mipmaps
    void GenerateMipmaps();

private:
    GLuint m_texture;
    int m_width;
    int m_height;
    int m_channels;
    TextureType m_type;
    std::string m_filepath;
};

// Simple texture manager for caching
class DD_TextureManager
{
public:
    static DD_TextureManager& Instance();

    DD_Texture* LoadTexture(const std::string& filepath);
    DD_Texture* GetTexture(const std::string& name);
    DD_Texture* CreateSolidColor(const std::string& name, const Vec4& color);
    
    void ReleaseTexture(const std::string& name);
    void ReleaseAll();

    // Built-in textures
    DD_Texture* GetWhiteTexture();
    DD_Texture* GetBlackTexture();
    DD_Texture* GetNormalTexture();  // Flat normal (0.5, 0.5, 1.0)

private:
    DD_TextureManager();
    ~DD_TextureManager();
    DD_TextureManager(const DD_TextureManager&) = delete;
    DD_TextureManager& operator=(const DD_TextureManager&) = delete;

    std::vector<std::pair<std::string, std::unique_ptr<DD_Texture>>> m_textures;
    
    DD_Texture* m_whiteTexture;
    DD_Texture* m_blackTexture;
    DD_Texture* m_normalTexture;
};

#define gTextureManager DD_TextureManager::Instance()
