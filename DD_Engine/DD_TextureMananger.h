#pragma once
#include "framework.h"

using Microsoft::WRL::ComPtr;

struct FDD_TextureResource
{
    FDD_TextureResource(class ID3D11ShaderResourceView* srv, std::wstring path);
    virtual ~FDD_TextureResource();

    class ID3D11ShaderResourceView* srv;
    std::wstring path;
    int32_t refCount;
};

struct FDD_TextureInstance
{
    FDD_TextureInstance(std::wstring filename, int32_t handle);
    virtual ~FDD_TextureInstance();

    std::shared_ptr<FDD_TextureResource> GetOrgin() const;
    std::wstring filename;
    int32_t handle = 0;
};

class DD_TextureMananger : public ISingleton<DD_TextureMananger>
{
public:
    virtual void Initialize();
    virtual void Tick(float deltaTime);
    virtual void Finalize();

public:
    std::shared_ptr<FDD_TextureInstance> CreateTextureInstance(const std::wstring& path);
public:
    std::shared_ptr<FDD_TextureResource> GetTextureOrigin(const FDD_TextureInstance* target);
    void ReleaseTextureOrigin(const FDD_TextureResource* target);
private:
    std::wstring GetExtention(std::wstring str);

    HRESULT CreateSRV(const std::wstring& path, class ID3D11ShaderResourceView** srvOut);
public:
    std::vector<std::wstring> GetOriginInfo();
private:
    std::unordered_map<std::wstring, std::shared_ptr<FDD_TextureResource>> m_textureCache;
    int32_t m_nextHandle = 0;

public:
    DD_TextureMananger();
    virtual ~DD_TextureMananger();
};

#define gTextureMng (*DD_TextureMananger::GetInstance())