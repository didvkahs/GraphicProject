#pragma once

class D3DResrouces;
class EntityResource;

enum VertexShaderID_e
{
	VS_DEFAULT,
	VS_COUNT,
	VS_NON
};

enum PixelShaderID_e
{
	PS_DEFAULT,
	PS_COUNT,
	PS_NON
};


class TestObject
{
public:

	TestObject(void);
	~TestObject(void);

	bool Initialize(D3DResources& resource, EntityResource& entitiyResource);

	void Update(void);
	void Draw(Camera* cam);

	void CloseObjectHandles(void);

private:

	struct TestVertex_s
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT4 nor;
		DirectX::XMFLOAT2 tex;
	};

private:
	bool setupD3D(void);
	bool createSRView(void);
	bool createBuffer(void);

	bool loadTexture(const char* inFileName, DirectX::ScratchImage& image, DirectX::TexMetadata& metaData);
private:

	D3DResources m_Resource;
	
	MeshList_s m_meshList;
	NodeList_s m_nodeList;

	ID3D11InputLayout* m_InputLayout = nullptr;
	ID3D11ShaderResourceView* m_SRVs[VS_COUNT];

	ID3D11Buffer* m_PixelBuffer = nullptr;
	ID3D11Buffer* m_VertexBuffer = nullptr;
	ID3D11Buffer* m_IndexBuffer = nullptr;

	TestVertex_s* m_GlobalVertices = nullptr;
	int m_VertexCount;

	uint32_t* m_GlobalIndices = nullptr;
	int m_IndexCount;

	DirectX::XMFLOAT4X4 m_World;

	int m_vsIDs[VS_COUNT];
	int m_psIDs[PS_COUNT];
	VertexShaderID_e m_currVSID = VS_DEFAULT;
	PixelShaderID_e m_currPSID = PS_DEFAULT;
};