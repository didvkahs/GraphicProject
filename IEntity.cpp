#include "D3DResources.h"
#include "GLTFTypes.h"
#include "Camera.h"

#include "IEntity.h"

#include "EntityResource.h"

using namespace DirectX;


inline std::wstring stows(const std::string input);

bool IEntity::loadTexture(const char* inFileName, ScratchImage& image, TexMetadata& metaData)
{
	HRESULT result = S_OK;

	D3D11_SUBRESOURCE_DATA srd = {};
	ID3D11Texture2D* texture = nullptr;

	std::string input = inFileName;
	std::wstring fileName = stows(input);
	size_t pos = fileName.find(L".", 1);

	std::wstring fileType = fileName.substr(pos);

	if (L".dds" == fileType)
	{
		result = LoadFromDDSFile(fileName.c_str(), DDS_FLAGS_NONE, &metaData, image);
	}
	else if (L".tgs" == fileType)
	{
		result = LoadFromTGAFile(fileName.c_str(), &metaData, image);
	}
	else if (L".hdr" == fileType)
	{
		result = LoadFromHDRFile(fileName.c_str(), &metaData, image);
	}
	else
	{
		result = LoadFromWICFile(fileName.c_str(), WIC_FLAGS_NONE, &metaData, image);
	}

	if (FAILED(result))
	{
		fprintf(stderr, "loadTexture form %s failed", inFileName);
		return false;
	}

	return true;
}

inline std::wstring stows(const std::string input)
{
	int inputLen = MultiByteToWideChar(CP_UTF8, 0, input.c_str(), (int)input.size(), nullptr, 0);
	std::wstring wstr(inputLen, 0);
	MultiByteToWideChar(CP_UTF8, 0, input.c_str(), (int)input.size(), &wstr[0], inputLen);

	return wstr;
}