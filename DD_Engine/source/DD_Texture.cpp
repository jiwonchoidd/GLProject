#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "DD_Texture.h"
#include <cstdio>
#include <cstring>

// Simple STB-like image loading (basic implementation)
// For production, use stb_image.h

DD_Texture::DD_Texture()
    : m_texture(0)
    , m_width(0)
    , m_height(0)
    , m_channels(0)
    , m_type(TextureType::Custom)
{
}

DD_Texture::~DD_Texture()
{
    Destroy();
}

bool DD_Texture::LoadFromFile(const std::string& filepath)
{
    m_filepath = filepath;
    
    int width, height, channels;
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    
    if (!data)
    {
        printf("DD_Texture::LoadFromFile - Failed to load: %s (%s)\n", 
               filepath.c_str(), stbi_failure_reason());
        // Create placeholder checkerboard
        return CreateCheckerboard(64, Vec4(1.0f, 0.0f, 1.0f, 1.0f), Vec4(0.2f, 0.2f, 0.2f, 1.0f));
    }
    
    printf("DD_Texture::LoadFromFile - Loaded: %s (%dx%d, %d channels)\n", 
           filepath.c_str(), width, height, channels);
    
    bool result = Create(width, height, channels, data);
    stbi_image_free(data);
    
    return result;
}

bool DD_Texture::Create(int width, int height, int channels, const unsigned char* data)
{
    if (m_texture)
    {
        Destroy();
    }

    m_width = width;
    m_height = height;
    m_channels = channels;

    glGenTextures(1, &m_texture);
    glBindTexture(GL_TEXTURE_2D, m_texture);

    GLenum format = GL_RGBA;
    GLenum internalFormat = GL_RGBA;
    
    switch (channels)
    {
    case 1:
        format = GL_RED;
        internalFormat = GL_R8;
        break;
    case 2:
        format = GL_RG;
        internalFormat = GL_RG8;
        break;
    case 3:
        format = GL_RGB;
        internalFormat = GL_RGB8;
        break;
    case 4:
        format = GL_RGBA;
        internalFormat = GL_RGBA8;
        break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // Default settings
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}

bool DD_Texture::CreateSolidColor(const Vec4& color)
{
    unsigned char data[4] = {
        static_cast<unsigned char>(color.r * 255.0f),
        static_cast<unsigned char>(color.g * 255.0f),
        static_cast<unsigned char>(color.b * 255.0f),
        static_cast<unsigned char>(color.a * 255.0f)
    };
    return Create(1, 1, 4, data);
}

bool DD_Texture::CreateCheckerboard(int size, const Vec4& color1, const Vec4& color2)
{
    std::vector<unsigned char> data(size * size * 4);

    unsigned char c1[4] = {
        static_cast<unsigned char>(color1.r * 255.0f),
        static_cast<unsigned char>(color1.g * 255.0f),
        static_cast<unsigned char>(color1.b * 255.0f),
        static_cast<unsigned char>(color1.a * 255.0f)
    };
    unsigned char c2[4] = {
        static_cast<unsigned char>(color2.r * 255.0f),
        static_cast<unsigned char>(color2.g * 255.0f),
        static_cast<unsigned char>(color2.b * 255.0f),
        static_cast<unsigned char>(color2.a * 255.0f)
    };

    int checkerSize = size / 8;
    if (checkerSize < 1) checkerSize = 1;

    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            int idx = (y * size + x) * 4;
            bool isWhite = ((x / checkerSize) + (y / checkerSize)) % 2 == 0;
            unsigned char* color = isWhite ? c1 : c2;
            data[idx + 0] = color[0];
            data[idx + 1] = color[1];
            data[idx + 2] = color[2];
            data[idx + 3] = color[3];
        }
    }

    return Create(size, size, 4, data.data());
}

void DD_Texture::Destroy()
{
    if (m_texture)
    {
        glDeleteTextures(1, &m_texture);
        m_texture = 0;
    }
    m_width = 0;
    m_height = 0;
    m_channels = 0;
}

void DD_Texture::Bind(int slot) const
{
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_texture);
}

void DD_Texture::Unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DD_Texture::SetWrap(TextureWrap wrap)
{
    if (!m_texture) return;

    glBindTexture(GL_TEXTURE_2D, m_texture);
    
    GLenum glWrap = GL_REPEAT;
    switch (wrap)
    {
    case TextureWrap::Repeat: glWrap = GL_REPEAT; break;
    case TextureWrap::ClampToEdge: glWrap = GL_CLAMP_TO_EDGE; break;
    case TextureWrap::MirroredRepeat: glWrap = GL_MIRRORED_REPEAT; break;
    }
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, glWrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, glWrap);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DD_Texture::SetFilter(TextureFilter filter)
{
    if (!m_texture) return;

    glBindTexture(GL_TEXTURE_2D, m_texture);
    
    switch (filter)
    {
    case TextureFilter::Nearest:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;
    case TextureFilter::Linear:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    case TextureFilter::LinearMipmap:
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void DD_Texture::GenerateMipmaps()
{
    if (!m_texture) return;

    glBindTexture(GL_TEXTURE_2D, m_texture);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
}

// TextureManager implementation
DD_TextureManager& DD_TextureManager::Instance()
{
    static DD_TextureManager instance;
    return instance;
}

DD_TextureManager::DD_TextureManager()
    : m_whiteTexture(nullptr)
    , m_blackTexture(nullptr)
    , m_normalTexture(nullptr)
{
}

DD_TextureManager::~DD_TextureManager()
{
    ReleaseAll();
}

DD_Texture* DD_TextureManager::LoadTexture(const std::string& filepath)
{
    // Check if already loaded
    for (auto& pair : m_textures)
    {
        if (pair.first == filepath)
        {
            return pair.second.get();
        }
    }

    auto texture = std::make_unique<DD_Texture>();
    if (texture->LoadFromFile(filepath))
    {
        DD_Texture* ptr = texture.get();
        m_textures.emplace_back(filepath, std::move(texture));
        return ptr;
    }
    return nullptr;
}

DD_Texture* DD_TextureManager::GetTexture(const std::string& name)
{
    for (auto& pair : m_textures)
    {
        if (pair.first == name)
        {
            return pair.second.get();
        }
    }
    return nullptr;
}

DD_Texture* DD_TextureManager::CreateSolidColor(const std::string& name, const Vec4& color)
{
    // Check if already exists
    if (DD_Texture* existing = GetTexture(name))
    {
        return existing;
    }

    auto texture = std::make_unique<DD_Texture>();
    if (texture->CreateSolidColor(color))
    {
        DD_Texture* ptr = texture.get();
        m_textures.emplace_back(name, std::move(texture));
        return ptr;
    }
    return nullptr;
}

void DD_TextureManager::ReleaseTexture(const std::string& name)
{
    auto it = std::find_if(m_textures.begin(), m_textures.end(),
        [&name](const auto& pair) { return pair.first == name; });
    
    if (it != m_textures.end())
    {
        m_textures.erase(it);
    }
}

void DD_TextureManager::ReleaseAll()
{
    m_textures.clear();
    m_whiteTexture = nullptr;
    m_blackTexture = nullptr;
    m_normalTexture = nullptr;
}

DD_Texture* DD_TextureManager::GetWhiteTexture()
{
    if (!m_whiteTexture)
    {
        m_whiteTexture = CreateSolidColor("__white", Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    }
    return m_whiteTexture;
}

DD_Texture* DD_TextureManager::GetBlackTexture()
{
    if (!m_blackTexture)
    {
        m_blackTexture = CreateSolidColor("__black", Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
    return m_blackTexture;
}

DD_Texture* DD_TextureManager::GetNormalTexture()
{
    if (!m_normalTexture)
    {
        // Flat normal: (0.5, 0.5, 1.0) in tangent space
        m_normalTexture = CreateSolidColor("__normal", Vec4(0.5f, 0.5f, 1.0f, 1.0f));
    }
    return m_normalTexture;
}
