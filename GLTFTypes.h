#pragma once

#include <EASTL/string.h>
#include <DirectXTex.h>
#include <string>

struct JointPose_s
{
	DirectX::XMFLOAT3 translation = { 0, 0, 0 };
	DirectX::XMFLOAT4 rotation = { 0, 0, 0, 1 };
	float scale = 0;

	DirectX::XMMATRIX GetMat() const
	{
		DirectX::XMMATRIX localMat;

		localMat = 
			DirectX::XMMatrixScaling(scale, scale, scale) *
			DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation)) *
			DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);

		return localMat;
	}
};

struct Node_s
{
	int parentIDX = -1;
	
	eastl::string name;
	eastl::string skinIDX;
	eastl::string meshIDX;
	
	bool hasLocalPose = false;
	JointPose_s localPose;
};

struct Skin_s
{
	int jointNodeCount = 0;
	int* jointNodeIndices = nullptr;

	DirectX::XMFLOAT4X4* invBindPoses = nullptr;
};

struct ModelVertex_s
{
	DirectX::XMFLOAT4 position = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT4 normal = { 0.0f, 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT2 uv = { 0.0f, 0.0f };

	uint32_t textureIDX;
	DirectX::XMUINT4 joints = { 0, 0, 0, 1};
	DirectX::XMFLOAT4 weights = { 0.0f, 0.0f, 0.0f, 0.0f };
};

struct Mesh_s
{
	int indexCount = 0;
	int vertexCount = 0;
	eastl::string materialIDX;
	uint32_t* indices = nullptr;
	ModelVertex_s* vertices = nullptr;
};

struct NodeList_s
{
	Node_s* list = nullptr;
	int count = 0;
};

struct MeshList_s
{
	Mesh_s* list = nullptr;
	int count = 0;
};