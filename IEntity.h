#pragma once


class D3DResources;
class EntityResource;
class Camera;


class IEntity
{
public:

	IEntity(void) {};
	~IEntity(void) {};
	
	virtual bool Initialize(D3DResources& resource, EntityResource* entityResource) = 0;

	virtual void Update(void) = 0;
	virtual void Draw(Camera* cam) = 0;

	virtual void CloseIEntityHandles(void) = 0;

protected:

	bool loadTexture(const char* inFileName, DirectX::ScratchImage& image, DirectX::TexMetadata& metaData);

protected:

	D3DResources m_Resource;

	MeshList_s m_MeshList;
	NodeList_s m_NodeList;

	ID3D11InputLayout* m_InputLayout = nullptr;
	ID3D11ShaderResourceView** m_SRVs = nullptr;

	ID3D11Buffer* m_PixelBuffer = nullptr;
	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11Buffer* m_IndexBuffer = nullptr;
	ID3D11Buffer* m_CBWorld = nullptr;


	uint32_t* m_GlobalIndices = nullptr;
	int m_IndexCount = 0;
};