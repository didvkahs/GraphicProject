#pragma once

struct Skeleton_s
{
	Joint_s* list = nullptr;
	int count = 0;
};

struct MeshList_s
{
	Mesh_s* list = nullptr;
	int count = 0;
};

struct TextureList_s
{
	Texture_s* list = nullptr;
	int count = 0;
};

struct MaterialList_s
{
	Material_s* list = nullptr;
	int count = 0;
};


struct JointPose_s
{
	DirectX::XMFLOAT3 translation = { 0, 0, 0 };
	DirectX::XMFLOAT4 rotation = { 0, 0, 0, 1 };
	float scale = 0;

	DirectX::XMMATRIX GetMat()
	{
		DirectX::XMMATRIX localMat;

		localMat = 
			DirectX::XMMatrixScaling(scale, scale, scale) *
			DirectX::XMMatrixRotationQuaternion(DirectX::XMLoadFloat4(&rotation)) *
			DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);

		return localMat;
	}
};

struct Joint_s
{
	DirectX::XMFLOAT4X4 inveBindPose;
	unsigned int parentIDX = 0;
	std::string jointName;
	std::string nodeId;
	std::string jointId;
	JointPose_s localPose;
};

struct Texture_s
{
	bool hasUri = false;
	WCHAR* fileName = nullptr;

	uint8_t* data = nullptr;
	int dataSize = 0;

	DirectX::ScratchImage imageData;
	DirectX::TexMetadata  imageMetaData;
};

struct ModelVertex_s
{
	DirectX::XMFLOAT4 position;
	DirectX::XMFLOAT4 normal;
	DirectX::XMFLOAT2 uv;

	uint32_t textureIDX;
	DirectX::XMUINT4 joints;
	DirectX::XMFLOAT4 weights;
};

struct Mesh_s
{
	uint32_t* indices = nullptr;
	ModelVertex_s* vertices = nullptr;
	int indexCount = 0;
	int vertexCount = 0;
};

struct Material_s
{
	WCHAR* name;

	float baseColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f};
	float metallicFactor = 0;
	float roughnessFactor = 0;
	float emissiveFactor[3] = { 1.0f, 1.0f, 1.0f };
};