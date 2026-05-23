#include <cstdint>

#include "D3DResources.h"
#include "GLTFTypes.h"
#include "Camera.h"

#include "TestObject.h"

#include "EntityResource.h"

#include <iostream>

using namespace DirectX;


constexpr int IMG_WIDTH = 512;
constexpr int IMG_HEIGHT = 512;



TestObject::TestObject(void) {}
TestObject::~TestObject(void) {}

bool TestObject::Initialize(D3DResources& resource, EntityResource& entityResource)
{
	m_Resource = resource;
	
	int meshCount = entityResource.meshList.count;
	m_meshList.list = new Mesh_s[meshCount];
	m_meshList.count = meshCount;
	memcpy(m_meshList.list, entityResource.meshList.list, sizeof(Mesh_s) * meshCount);

	int nodeCount = entityResource.nodeList.count;
	m_nodeList.list = new Node_s[nodeCount];
	m_nodeList.count = nodeCount;
	memcpy(m_nodeList.list, entityResource.nodeList.list, sizeof(Node_s) * nodeCount);


	if (!createBuffer())
	{
		fprintf(stderr, "createBuffer failed\n");
		goto LB_FAILED_CREATE_BUFFER;
	}
	if (!createSRView())
	{
		fprintf(stderr, "createSRView failed\n");
		goto LB_FAILED_CREATE_SRVIEW;
	}
	setupD3D();
	
	return true;

LB_FAILED_CREATE_INPUTLAYOUT:
	for (int i = 0; i < VS_COUNT; ++i)
	{
		SAFE_RELEASE(m_SRVs[i])
		m_SRVs[i] = nullptr;
	}

LB_FAILED_CREATE_SRVIEW:
	SAFE_RELEASE(m_VertexBuffer);
	SAFE_RELEASE(m_IndexBuffer);
	delete[] m_GlobalVertices;
	m_GlobalVertices = nullptr;
	delete[] m_GlobalIndices;
	m_GlobalIndices = nullptr;

LB_FAILED_CREATE_BUFFER:

LB_FAILED_LOAD_MODEL:

	return false;
}

void TestObject::Update(void)
{

}

void TestObject::Draw(Camera* cam)
{
	ID3D11DeviceContext* devcon = m_Resource.GetContext();
	ID3D11RenderTargetView* rtview = m_Resource.GetRTView();
	ID3D11DepthStencilView* depthStencilView = m_Resource.GetDepthStencilView();
	ID3D11SamplerState* sampler = m_Resource.GetSampler();

	UINT stride = sizeof(TestVertex_s);
	UINT offset = 0;

	devcon->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	devcon->IASetIndexBuffer(m_IndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	devcon->IASetInputLayout(m_InputLayout);
	devcon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11VertexShader* targetVShader = m_Resource.GetVShader(m_vsIDs[m_currVSID]);
	ID3D11PixelShader* targetPShader = m_Resource.GetPShader(m_psIDs[m_currPSID]);

	ID3D11Buffer* view = cam->GetCBView();
	devcon->VSSetConstantBuffers(0, 1, &view);

	ID3D11Buffer* projection = cam->GetCBProjection();
	devcon->VSSetConstantBuffers(1, 1, &projection);

	ID3D11Buffer* world = cam->GetCBWorld();
	devcon->VSSetConstantBuffers(2, 1, &world);

	devcon->VSSetShader(targetVShader, nullptr, 0);
	devcon->PSSetShader(targetPShader, nullptr, 0);
	devcon->PSSetShaderResources(0, 1, &m_SRVs[m_currVSID]);
	devcon->PSSetSamplers(0, 1, &sampler);

	devcon->DrawIndexed(m_IndexCount, 0, 0);
}

void TestObject::CloseObjectHandles(void)
{
	SAFE_RELEASE(m_InputLayout);

	for (int i = 0; i < VS_COUNT; ++i)
	{
		SAFE_RELEASE(m_SRVs[i])
		m_SRVs[i] = nullptr;
	}

	SAFE_RELEASE(m_VertexBuffer);
	SAFE_RELEASE(m_IndexBuffer);

	if (m_meshList.list)
	{
		delete[] m_meshList.list;
		m_meshList.list = nullptr;
		m_meshList.count = 0;
	}

	if (m_nodeList.list)
	{
		delete[] m_nodeList.list;
		m_nodeList.list = nullptr;
		m_nodeList.count = 0;
	}

	if (m_IndexCount) 
	{ 
		delete[] m_GlobalIndices;
		m_GlobalIndices = nullptr;
	}

	if (m_VertexCount)
	{
		delete[] m_GlobalVertices;
		m_GlobalVertices = nullptr;
	}
}




bool TestObject::setupD3D(void)
{
	ID3DBlob* vsBlob = nullptr;

	m_Resource.AddPixelShader(L"defaultShader.fx", "psMain", m_psIDs[PS_DEFAULT]);
	m_Resource.AddVertexShader(L"defaultShader.fx", "vsMain", m_vsIDs[VS_DEFAULT], vsBlob);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL"  , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	m_InputLayout = m_Resource.CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob);
	vsBlob->Release();

	return true;
}

bool TestObject::createBuffer()
{
	ID3D11Device* device = m_Resource.GetDevice();	
	MeshList_s meshList = m_meshList;

	for (int i = 0; i < meshList.count; ++i)
	{
		m_IndexCount += meshList.list[i].indexCount;
		m_VertexCount += meshList.list[i].vertexCount;
	}

	m_GlobalIndices = new uint32_t[m_IndexCount];
	m_GlobalVertices = new TestVertex_s[m_VertexCount];

	int indOffset = 0;
	int verOffset = 0;

	for (int i = 0; i < meshList.count; ++i)
	{
		Mesh_s& mesh = meshList.list[i];

		for (int j = 0; j < mesh.indexCount; ++j)
		{
			m_GlobalIndices[j + indOffset] = mesh.indices[j] + verOffset;
		}
		indOffset += mesh.indexCount;


		for (int j = 0; j < mesh.vertexCount; ++j)
		{
			ModelVertex_s& rawVertex = mesh.vertices[j];

			m_GlobalVertices[verOffset + j].nor = rawVertex.normal;
			m_GlobalVertices[verOffset + j].pos = rawVertex.position;
			m_GlobalVertices[verOffset + j].tex = rawVertex.uv;
		}

		verOffset += mesh.vertexCount;
	}

	HRESULT result = S_OK;

	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(TestVertex_s) * m_VertexCount;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA srd = {};
	srd.pSysMem = m_GlobalVertices;
	
	result = device->CreateBuffer(&bd, &srd, &m_VertexBuffer);
	if (FAILED(result))
	{
		fprintf(stderr, "(TESTOBJ) create vertexBuffer failed\n");
		return false;
	}

	bd.ByteWidth = sizeof(uint32_t) * m_IndexCount;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	srd.pSysMem = m_GlobalIndices;

	result = device->CreateBuffer(&bd, &srd, &m_IndexBuffer);
	if (FAILED(result))
	{
		fprintf(stderr, "(TESTOBJ) create indexBuffer failed\n");
		return false;
	}
	return true;
}

bool TestObject::createSRView(void)
{
	// TODO : (IMP | LATER) support 2D array texture later

	HRESULT result = S_OK;

	int meshCount = m_meshList.count;

	ID3D11Device* device = m_Resource.GetDevice();

	D3D11_TEXTURE2D_DESC texd = {};
	D3D11_SUBRESOURCE_DATA initData;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};

	ScratchImage image;
	TexMetadata metaData; 

	loadTexture(".//assets//seafloor.dds", image, metaData);
	const Image* img = image.GetImage(0, 0, 0);
	initData.pSysMem = img->pixels;
	initData.SysMemPitch = img->rowPitch;
	initData.SysMemSlicePitch = img->slicePitch;
	
	texd.Width = metaData.width;
	texd.Height = metaData.height;
	texd.MipLevels = metaData.mipLevels;
	texd.ArraySize = metaData.arraySize;
	texd.Format = metaData.format;
	texd.SampleDesc.Count = 1;
	texd.Usage = D3D11_USAGE_DEFAULT;
	texd.BindFlags = D3D11_BIND_SHADER_RESOURCE;


	ID3D11Texture2D* texture = nullptr;

	result = device->CreateTexture2D(&texd, &initData, &texture);
	if (FAILED(result))
	{
		fprintf(stderr, "createTexture2D failed with error \n");
		return false;
	}

	srvd.Format = texd.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvd.Texture2DArray.MostDetailedMip = 0;
	srvd.Texture2DArray.FirstArraySlice = 0;
	srvd.Texture2DArray.MipLevels = texd.MipLevels;
	srvd.Texture2DArray.ArraySize = texd.ArraySize;

	result = device->CreateShaderResourceView(texture, &srvd, &m_SRVs[VS_DEFAULT]);

	return true;
}


inline std::wstring stows(const std::string input);

bool TestObject::loadTexture(const char* inFileName, DirectX::ScratchImage& image, DirectX::TexMetadata& metaData)
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