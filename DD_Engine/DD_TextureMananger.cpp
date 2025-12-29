#include "DD_TextureMananger.h"
#include <DirectXTex.h>
#include "DD_Device.h"
#include "DD_DXHelper.h"

namespace TEXTUREMNG
{
	constexpr float cleanupTime = 8.f;
	constexpr int32_t cleanupSize = 30.f;
}

void DD_TextureMananger::Initialize()
{
	m_textureCache.clear();
	m_nextHandle = 0;
}

static float elapsedCleanTime = 0;
void DD_TextureMananger::Tick(float deltaTime)
{
	elapsedCleanTime += deltaTime;

	if (elapsedCleanTime > TEXTUREMNG::cleanupTime || m_textureCache.size() > TEXTUREMNG::cleanupSize)
	{
		for (auto it = m_textureCache.begin(); it != m_textureCache.end(); )
		{
			if (it->second->refCount == 0)
			{
				it = m_textureCache.erase(it);
			}
			else
			{
				++it;
			}
		}

		elapsedCleanTime = 0.f;
	}
}

void DD_TextureMananger::Finalize()
{
	m_textureCache.clear();
}

std::shared_ptr<FDD_TextureInstance> DD_TextureMananger::CreateTextureInstance(const std::wstring& path)
{
	std::shared_ptr<FDD_TextureResource> origin;

	auto it = m_textureCache.find(path);
	if (it != m_textureCache.end())
	{
		origin = it->second;
	}

	if (!origin)
	{
		ID3D11ShaderResourceView* srv;
		HRESULT result = CreateSRV(path, &srv);
		if (FAILED(result))
		{
			return nullptr;
		}

		origin = std::make_shared<FDD_TextureResource>(srv, path);
		m_textureCache[path] = origin;
	}

	std::shared_ptr<FDD_TextureInstance> instance = std::make_shared<FDD_TextureInstance>(path, m_nextHandle++);
	return instance;
}

std::shared_ptr<FDD_TextureResource> DD_TextureMananger::GetTextureOrigin(const FDD_TextureInstance* target)
{
	if (!target)
		return nullptr;

	std::shared_ptr<FDD_TextureResource> origin;

	auto it = m_textureCache.find(target->filename);
	if (it != m_textureCache.end())
	{
		return it->second;
	}

	return nullptr;
}

void DD_TextureMananger::ReleaseTextureOrigin(const FDD_TextureResource* target)
{
	if (!target)
		return;

	if (target->srv)
		target->srv->Release();
}

std::wstring DD_TextureMananger::GetExtention(std::wstring str)
{
	size_t sz = str.rfind('.', str.length());
	if (sz != std::wstring::npos)
		return str.substr(sz + 1, str.length() - sz);
	return std::wstring();
}

HRESULT DD_TextureMananger::CreateSRV(const std::wstring& path, ID3D11ShaderResourceView** srvOut)
{
	HRESULT result = S_OK;

	std::wstring ext = GetExtention(path);
	DirectX::ScratchImage image;

	if (ext == L"dds")
		result = DirectX::LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE, nullptr, image);
	else
		result = DirectX::LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE, nullptr, image);

	if (FAILED(result)) return result;

	ComPtr<ID3D11Resource> resource;

	result = DirectX::CreateTexture(gDevice.GetDevice(), image.GetImages(), image.GetImageCount(), image.GetMetadata(), resource.GetAddressOf());

	if (FAILED(result)) return result;

	result = gDevice.GetDevice()->CreateShaderResourceView(resource.Get(), nullptr, srvOut);


	return result;
}

std::vector<std::wstring> DD_TextureMananger::GetOriginInfo()
{
	std::vector<std::wstring> info;
	for (const auto& texture : m_textureCache)
	{
		wchar_t buffer[256];
		swprintf_s(buffer, L"origin : %s, count : %ld", DD_DXHelper::ExtractFileName(texture.first).c_str(),
			texture.second->refCount);

		info.push_back(buffer);
	}
	return info;
}

DD_TextureMananger::DD_TextureMananger()
{
}

DD_TextureMananger::~DD_TextureMananger()
{
}

//struct FDD_TextureResource ------------------------------------------------------------------------------------------------

FDD_TextureResource::FDD_TextureResource(ID3D11ShaderResourceView* srv, std::wstring path) : srv(srv), path(path), refCount(0)
{
}

FDD_TextureResource::~FDD_TextureResource()
{
	if (DD_TextureMananger::GetInstance())
	{
		gTextureMng.ReleaseTextureOrigin(this);
	}
}

//struct FDD_TextureInstance ------------------------------------------------------------------------------------------------

FDD_TextureInstance::FDD_TextureInstance(std::wstring filename, int32_t handle) : filename(filename), handle(handle)
{
	std::shared_ptr<FDD_TextureResource> origin = GetOrgin();
	if (!origin)
		return;

	origin->refCount++;
}

FDD_TextureInstance::~FDD_TextureInstance()
{
	std::shared_ptr<FDD_TextureResource> origin = GetOrgin();
	if (!origin)
		return;

	origin->refCount--;
}

std::shared_ptr<FDD_TextureResource> FDD_TextureInstance::GetOrgin() const
{
	if (DD_TextureMananger::GetInstance())
	{
		return gTextureMng.GetTextureOrigin(this);
	}

	return nullptr;
}
