#include "DD_SimpleBox.h"
#include "DD_Core.h"
#include "DD_Device.h"
#include "DD_TextureMananger.h"

using Microsoft::WRL::ComPtr;

struct SimpleVertex
{
    XMFLOAT3 Pos;
    XMFLOAT2 Tex;
};

struct CBObject
{
    XMMATRIX mWorld;
    XMFLOAT4 vMeshColor;
};

XMFLOAT4 g_vMeshColor(0.7f, 0.7f, 0.7f, 1.0f);

// 공유 파이프라인 리소스
namespace
{
    ComPtr<ID3D11InputLayout>  s_inputLayout;
    ComPtr<ID3D11VertexShader> s_vs;
    ComPtr<ID3D11PixelShader>  s_ps;
    ComPtr<ID3D11SamplerState> s_sampler;
    bool                       s_initialized = false;
}

bool DD_SimpleBox::CachePipline()
{
    if (s_initialized)
        return true;

    ComPtr<ID3DBlob> vsBlob;
    ComPtr<ID3DBlob> psBlob;
    HRESULT hr = DD_DXHelper::CompileShaderFromFile(DD_DXHelper::GetAssetsPath(L"Test.hlsl").c_str(),
        "VS", "vs_4_0", vsBlob.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = DD_DXHelper::CompileShaderFromFile(DD_DXHelper::GetAssetsPath(L"Test.hlsl").c_str(),
        "PS", "ps_4_0", psBlob.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = gDevice.GetDevice()->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, s_vs.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = gDevice.GetDevice()->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, s_ps.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_INPUT_ELEMENT_DESC layout[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    hr = gDevice.GetDevice()->CreateInputLayout(layout, _countof(layout),
        vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
        s_inputLayout.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_SAMPLER_DESC sampDesc;
    ZeroMemory(&sampDesc, sizeof(sampDesc));
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;

    hr = gDevice.GetDevice()->CreateSamplerState(&sampDesc, s_sampler.GetAddressOf());
    if (FAILED(hr)) return false;

    s_initialized = true;
    return true;
}

void DD_SimpleBox::ClearPipline()
{
    s_sampler.Reset();
    s_inputLayout.Reset();
    s_vs.Reset();
    s_ps.Reset();
    s_initialized = false;
}

void DD_SimpleBox::Create(const XMFLOAT3& pos, std::wstring texturePath /*= ""*/)
{
    m_position = pos;

    static SimpleVertex vertices[] =
    {
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f, -1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3( 1.0f,  1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },

        { XMFLOAT3(-1.0f, -1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
        { XMFLOAT3( 1.0f, -1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
        { XMFLOAT3( 1.0f,  1.0f,  1.0f), XMFLOAT2(1.0f, 1.0f) },
        { XMFLOAT3(-1.0f,  1.0f,  1.0f), XMFLOAT2(0.0f, 1.0f) },
    };

    m_textureInstance = gTextureMng.CreateTextureInstance(texturePath.length() > 0 ? texturePath : DD_DXHelper::GetAssetsPath(L"1.jpg"));

    // Vertex Buffer
    D3D11_BUFFER_DESC bd;
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(SimpleVertex) * 24;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initVB{};
    initVB.pSysMem = vertices;

    HRESULT hr = gDevice.GetDevice()->CreateBuffer(&bd, &initVB, &m_vertexBuffer);
    if (FAILED(hr)) return;

    // Index Buffer
    static WORD indices[] =
    {
        3,1,0,  2,1,3,
        6,4,5,  7,4,6,
        11,9,8, 10,9,11,
        14,12,13, 15,12,14,
        19,17,16, 18,17,19,
        22,20,21, 23,20,22
    };

    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(WORD) * 36;
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = 0;

    D3D11_SUBRESOURCE_DATA initIB{};
    initIB.pSysMem = indices;

    hr = gDevice.GetDevice()->CreateBuffer(&bd, &initIB, &m_indexBuffer);
    if (FAILED(hr)) return;

    // Constant Buffer (b2)
    ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bd.ByteWidth = sizeof(CBObject);
    hr = gDevice.GetDevice()->CreateBuffer(&bd, NULL, &m_constantBuffer);
}

void DD_SimpleBox::Render()
{
    auto* ctx = gDevice.GetDeviceContext();

    ctx->IASetInputLayout(s_inputLayout.Get());
    ctx->VSSetShader(s_vs.Get(), nullptr, 0);
    ctx->PSSetShader(s_ps.Get(), nullptr, 0);
    ctx->PSSetSamplers(0, 1, s_sampler.GetAddressOf());

    UINT stride = sizeof(SimpleVertex);
    UINT offset = 0;
    ctx->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);
    ctx->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R16_UINT, 0);

    if (m_textureInstance)
    {
        std::shared_ptr<FDD_TextureResource> origin = m_textureInstance->GetOrgin();
        ctx->PSSetShaderResources(0, 1, &origin->srv);
    }

    XMMATRIX S = XMMatrixScaling(1, 1, 1);
    XMMATRIX R = XMMatrixRotationY(m_rotation.y);
    XMVECTOR posVec = XMLoadFloat3(&m_position);
    XMMATRIX T = XMMatrixTranslationFromVector(posVec);
    XMMATRIX worldMat = XMMatrixMultiply(XMMatrixMultiply(S, R), T);

    CBObject cb;
    cb.mWorld = XMMatrixTranspose(worldMat);
    cb.vMeshColor = g_vMeshColor;

    ctx->UpdateSubresource(m_constantBuffer, 0, nullptr, &cb, 0, 0);
    ctx->VSSetConstantBuffers(2, 1, &m_constantBuffer);
    ctx->PSSetConstantBuffers(2, 1, &m_constantBuffer);

    // 드로우
    ctx->DrawIndexed(36, 0, 0);
}

void DD_SimpleBox::AddPos(const XMFLOAT3& addpos)
{
    XMVECTOR currentPos = XMLoadFloat3(&m_position);
    XMVECTOR deltaPos = XMLoadFloat3(&addpos);
    XMVECTOR newPos = XMVectorAdd(currentPos, deltaPos);
    XMStoreFloat3(&m_position, newPos);
}

void DD_SimpleBox::AddRot(const XMFLOAT3& addrot)
{
    XMVECTOR currentRot = XMLoadFloat3(&m_rotation);
    XMVECTOR deltaRot = XMLoadFloat3(&addrot);
    XMVECTOR newRot = XMVectorAdd(currentRot, deltaRot);
    XMStoreFloat3(&m_rotation, newRot);
}

DD_SimpleBox::DD_SimpleBox()
{
}

DD_SimpleBox::~DD_SimpleBox()
{
    if (m_constantBuffer) m_constantBuffer->Release();
    if (m_vertexBuffer) m_vertexBuffer->Release();
    if (m_indexBuffer) m_indexBuffer->Release();
}
