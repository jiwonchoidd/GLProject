#include "DD_DXHelper.h"
#include <io.h>

bool DD_DXHelper::FindDirectoryItem(const std::wstring& path, const std::wstring& extName, std::vector<std::wstring>& outResult)
{
	outResult.clear();

	wchar_t exePath[MAX_PATH]{};
	GetModuleFileNameW(nullptr, exePath, MAX_PATH);

	std::wstring folder = exePath;
	size_t pos = folder.find_last_of(L"\\/");
	if (pos != std::wstring::npos)
		folder = folder.substr(0, pos);

	std::wstring searchPath = folder + L"\\" + path;
	if (searchPath.back() != L'\\')
		searchPath += L'\\';

	// *.ext
	std::wstring searchCommand = searchPath + L"*" + extName;

	_wfinddata_t fd{};
	intptr_t handle = _wfindfirst(searchCommand.c_str(), &fd);
	if (handle == -1)
		return false;

	do
	{
		if (!(fd.attrib & _A_SUBDIR))
		{
			outResult.emplace_back(searchPath + fd.name);
		}
	} while (_wfindnext(handle, &fd) == 0);

	_findclose(handle);

	return !outResult.empty();
}

int32_t DD_DXHelper::GetRandomNum(int32_t startNum, int32_t endNum)
{
	if (startNum >= endNum)
		return 0;
	return startNum + (rand() % (endNum - startNum + 1));
}

std::wstring DD_DXHelper::ExtractFileName(const std::wstring fullFilePath)
{
	size_t pos = fullFilePath.find_last_of(L"\\/");
	if (pos == std::string::npos)
		return fullFilePath;
	return fullFilePath.substr(pos + 1);
}

HRESULT DD_DXHelper::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
	HRESULT hr = S_OK;

	DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	// Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
	// Setting this flag improves the shader debugging experience, but still allows 
	// the shaders to be optimized and to run exactly the way they will run in 
	// the release configuration of this program.
	dwShaderFlags |= D3DCOMPILE_DEBUG;
#endif

	ID3DBlob* pErrorBlob;

	hr = D3DX11CompileFromFile(szFileName, NULL, NULL, szEntryPoint, szShaderModel,
		dwShaderFlags, 0, NULL, ppBlobOut, &pErrorBlob, NULL);
	if (FAILED(hr))
	{
		if (pErrorBlob != NULL)
			OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
		if (pErrorBlob)
			pErrorBlob->Release();
		return hr;
	}
	if (pErrorBlob)
		pErrorBlob->Release();

	return S_OK;
}

std::wstring DD_DXHelper::GetAssetsPath(const std::wstring postFix /*= L""*/)
{
	WCHAR assetsPath[512];

	DWORD size = GetModuleFileName(nullptr, assetsPath, _countof(assetsPath));
	if (size == 0 || size == _countof(assetsPath))
	{
		throw std::exception();
	}

	WCHAR* lastSlash = wcsrchr(assetsPath, L'\\');
	if (lastSlash)
	{
		*(lastSlash + 1) = L'\0';
	}

	std::wstring value = assetsPath + postFix;
	return value;
}