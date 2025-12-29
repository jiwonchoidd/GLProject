#pragma once
#include "framework.h"
#include <DirectXMath.h>
#include <d3dcompiler.h>
#include <d3dcommon.h>

using namespace DirectX;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

namespace DD_DXHelper
{
	bool FindDirectoryItem(const std::wstring& path, const std::wstring& extName, std::vector<std::wstring>& outResult);
	int32_t GetRandomNum(int32_t startNum, int32_t endNum);
	std::wstring ExtractFileName(const std::wstring fullFilePath);
	HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);
	std::wstring GetAssetsPath(const std::wstring postFix = L"");
};

