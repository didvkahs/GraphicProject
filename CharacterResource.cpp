#include "GLTFTypes.h"
#include "GLTFReader.h"
#include "EntityResource.h"

#include "CharacterResource.h"

#include <cfloat>
#include <DirectXMath.h>

using namespace DirectX;

CharacterResource::CharacterResource(void){}

CharacterResource::CharacterResource(const std::string& filePath) : EntityResource(filePath)
{
#ifdef _DEBUG
	if (m_reader == nullptr) __debugbreak();
#endif

	m_reader->GetSkins(skins);

	normalizeHeight();
}


CharacterResource::~CharacterResource(void)
{
	if (skins.invBindPoses != nullptr)
	{
		delete[] skins.invBindPoses;
		skins.invBindPoses = nullptr;

		delete[] skins.jointNodeIndices;
		skins.jointNodeIndices = nullptr;
		skins.jointNodeCount = -1;
	}
}


void CharacterResource::normalizeHeight(void)
{
	if (meshList.count <= 0 || meshList.list == nullptr) return;

	float maxY = -FLT_MAX; 
	float minY = FLT_MAX; 

	for (int m = 0; m < meshList.count; ++m)
	{
		Mesh_s& mesh = meshList.list[m];
		if (mesh.vertices == nullptr) continue;

		for (int v = 0; v < mesh.vertexCount; ++v)
		{
			float yPos = mesh.vertices[v].position.y;

			if (yPos > maxY) maxY = yPos;
			if (yPos < minY) minY = yPos;
		}
	}

	float originalHeight = maxY - minY;

	if (originalHeight <= FLT_EPSILON)
	{
		XMMATRIX scaleMat = XMMatrixIdentity();
		XMStoreFloat4x4(&scale, scaleMat);
		return;
	}

	
	float targetHeight = (float)NORMALIZED_CHARACTER_HEIGHT;
	float normalScale = targetHeight / originalHeight;

	XMMATRIX scaleMat = XMMatrixScaling(normalScale, normalScale, normalScale);
	XMStoreFloat4x4(&scale, scaleMat);
}