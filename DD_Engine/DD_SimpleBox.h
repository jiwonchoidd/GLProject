#pragma once
#include "framework.h"

using namespace DirectX;

class DD_SimpleBox
{
public:
	static bool CachePipline();
	static void ClearPipline();
public:
	void Create(const XMFLOAT3& pos, std::wstring texturePath = L"");
public:
	void Render();
public:
	void AddPos(const XMFLOAT3& addpos);
	void AddRot(const XMFLOAT3& addpos);
	void SetPos(const XMFLOAT3& pos) { m_position = pos; }
	XMFLOAT3 GetPos() const { return m_position; }
public:
	void SetTexture(std::shared_ptr<struct FDD_TextureInstance> textureInstance) { m_textureInstance = textureInstance; }
private:
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;
private:
	XMFLOAT3 m_rotation{};
	XMFLOAT3 m_position{};
	std::shared_ptr<struct FDD_TextureInstance> m_textureInstance;
public:
	DD_SimpleBox();
	virtual ~DD_SimpleBox();
};

