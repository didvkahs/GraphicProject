
#include <iostream>

#include "typedef.h"
#include"BmpReader.h"
#include "D3DResources.h"

#define SAFE_RELEASE(p) {if(p) {p->Release(); p = nullptr;}}

using namespace eastl;
using namespace DirectX;


void* __cdecl operator new[](unsigned __int64 size, char const*, int, unsigned int, char const*, int)
{
	return new unsigned char[size];
}
void* __cdecl operator new[](unsigned __int64 size, unsigned __int64, unsigned __int64, char const*, int, unsigned int, char const*, int)
{
	return new unsigned char[size];
}
namespace eastl {

	void __cdecl AssertionFailure(void*, const char* expression) {
#ifdef _DEBUG
		OutputDebugStringA(expression);
		__debugbreak();
#else
#endif
	}
}


D3DResources::D3DResources(void) {}
D3DResources::~D3DResources(void) { CloseD3DHandles();}


bool D3DResources::Initialize(HWND hWnd)
{
	m_hWnd = hWnd;

	RECT rect;
	GetClientRect(hWnd, &rect);

	winW = rect.right - rect.left;
	winH = rect.bottom - rect.top;
	
	if (!Bmp::LoadRGBs(&m_rBuf, &m_gBuf, &m_bBuf, width, height))
	{
		fprintf(stderr, "LoadRGBs failed with error\n");
		if (m_rBuf) Bmp::FreeMem(&m_rBuf);
		if (m_gBuf) Bmp::FreeMem(&m_gBuf);
		if (m_bBuf) Bmp::FreeMem(&m_bBuf);
		goto LB_FAILED_LOAD_RGBS;
	}

	if (!getMaxVideoAdapter())
	{
		fprintf(stderr, "getMaxVideoAdapter failed with error \n");
		goto LB_FAILED_CREATE_ADAPTER;
	}

	if (!createDeviceAndSwapChain())
	{
		fprintf(stderr, "createDeviceAndSwapChain failed with error\n");
		goto LB_FAILED_CREATE_DEVICE_AND_SWAPCHAIN;
	}
	
	if (!createRenderTargets())
	{
		fprintf(stderr, "createRenderTargets failed with error \n");
		goto LB_FAILED_CREAETE_RENDER_TARGETS;
	}


	if (!createVSAndPSAndInputLayout())
	{
		fprintf(stderr, "createVSAndPSAndInputLayout failed\n");
		goto LB_FAILED_CREATE_VS_PS_INPUTLAYOUT;
	}
	
	{
		HRESULT hr = S_OK;
		
		D3D11_SAMPLER_DESC samd = {};
		samd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		samd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		samd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		samd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		samd.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samd.MinLOD = 0;
		samd.MaxLOD = D3D11_FLOAT32_MAX;

		hr = m_Device->CreateSamplerState(&samd, &m_SamplerLinear);

		if (FAILED(hr))
		{
			fprintf(stderr, "create sampler state failed\n");
			goto LB_FAILED_CREATE_SAMPLER;
		}
	}

	if (!createBmpTexture())
	{
		fprintf(stderr, "createBmpTexture failed \n");
		goto LB_FAILED_CREATE_BMP_TEXTURE;
	}

	createClientSizeViewPort();

return true;

LB_FAILED_CREATE_BMP_TEXTURE:
	SAFE_RELEASE(m_SamplerLinear)

LB_FAILED_CREATE_SAMPLER:
	SAFE_RELEASE(m_InputLayout)
	SAFE_RELEASE(m_PixelBuffer)
	SAFE_RELEASE(m_VertexBuffer)
	
	for (auto& ps : m_PixelShaders)
	{
		ps->Release();
	}
	m_PixelShaders.clear();
	m_PixelShaders.shrink_to_fit();

	for (auto& vs : m_VertexShaders)
	{
		vs->Release();
	}
	m_VertexShaders.clear();
	m_VertexShaders.shrink_to_fit();

LB_FAILED_CREATE_VS_PS_INPUTLAYOUT:
	SAFE_RELEASE(m_RTView)
	SAFE_RELEASE(m_DepthStencil)
	SAFE_RELEASE(m_DepthStencilView)

LB_FAILED_CREAETE_RENDER_TARGETS:
	SAFE_RELEASE(m_Device)
	SAFE_RELEASE(m_DevContext)
	SAFE_RELEASE(m_SwapChain)

LB_FAILED_CREATE_DEVICE_AND_SWAPCHAIN:
	SAFE_RELEASE(m_Adapter)

LB_FAILED_CREATE_ADAPTER:
	if (m_rBuf) Bmp::FreeMem(&m_rBuf);
	if (m_gBuf) Bmp::FreeMem(&m_gBuf);
	if (m_bBuf) Bmp::FreeMem(&m_bBuf);
LB_FAILED_LOAD_RGBS:
	return false;
}

void D3DResources::CloseD3DHandles(void)
{
	if (m_DevContext) 
	{ 
		m_DevContext->ClearState();
	}

	for (auto& vshader : m_VertexShaders)
	{
		if (vshader) { vshader->Release(); }
	}
	m_VertexShaders.clear();
	m_VertexShaders.shrink_to_fit();

	for (auto& pshader : m_PixelShaders)
	{
		if (pshader) { pshader->Release(); }
	}
	m_PixelShaders.clear();
	m_PixelShaders.shrink_to_fit();

	for (auto& stagedtex : m_stagedTextures)
	{
		stagedtex.imageData.Release();
	}
	m_stagedTextures.clear();
	m_stagedTextures.shrink_to_fit();


	SAFE_RELEASE(m_InputLayout)
	SAFE_RELEASE(m_DepthStencilView)
	SAFE_RELEASE(m_DepthStencil)
	SAFE_RELEASE(m_RTView)
	SAFE_RELEASE(m_loadedTextureArray)
	SAFE_RELEASE(m_TextureRV)
	SAFE_RELEASE(m_SamplerLinear)

	SAFE_RELEASE(m_VertexBuffer)
	SAFE_RELEASE(m_PixelBuffer)
	SAFE_RELEASE(m_CBNeverChanges)
	SAFE_RELEASE(m_CBChangeOnResize)
	SAFE_RELEASE(m_CBChangesEveryFrame)

	if (m_DevContext)
	{
		m_DevContext->Flush();
	}
		

	SAFE_RELEASE(m_SwapChain)
		SAFE_RELEASE(m_DevContext)

#ifdef _DEBUG
	ID3D11Debug* debugDev = nullptr;
		
	if (m_Device && SUCCEEDED(m_Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&debugDev)))
	{
		// debugDev->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
	}
#endif

	SAFE_RELEASE(m_Device)
	SAFE_RELEASE(m_Adapter)

#ifdef _DEBUG
	if (debugDev)
	{
		debugDev->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
		debugDev->Release();

	}
#endif
}

void D3DResources::DrawBmp(void)
{
	float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

	m_DevContext->ClearRenderTargetView(m_RTView, clearColor);
	m_DevContext->ClearDepthStencilView(m_DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

	UINT stride = sizeof(Vertex_s);
	UINT offset = 0;

	m_DevContext->IASetVertexBuffers(0, 1, &m_VertexBuffer, &stride, &offset);
	m_DevContext->IASetInputLayout(m_InputLayout);
	m_DevContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11VertexShader* targetVShader = GetVShader(0);
	ID3D11PixelShader* targetPShader = GetPShader(0);

	m_DevContext->VSSetShader(targetVShader, nullptr, 0);
	m_DevContext->PSSetShader(targetPShader, nullptr, 0);
	m_DevContext->PSSetShaderResources(0, 1, &m_loadedTextureArray);
	m_DevContext->PSSetSamplers(0, 1, &m_SamplerLinear);

	m_DevContext->Draw(6, 0);
}


ID3D11Device* D3DResources::GetDevice(void)
{
	return m_Device;
}

ID3D11DeviceContext* D3DResources::GetContext(void)
{
	return m_DevContext;
}

ID3D11RenderTargetView* D3DResources::GetRTView(void)
{
	return m_RTView;
}

ID3D11VertexShader* D3DResources::GetVShader(const int id)
{
	return m_VertexShaders.at(id);
}

ID3D11PixelShader* D3DResources::GetPShader(const int id)
{
	return m_PixelShaders.at(id);
}

IDXGISwapChain* D3DResources::GetSwapChain(void)
{
	return m_SwapChain;
}

ID3D11InputLayout* D3DResources::GetInputLayout(void) const
{
	return m_InputLayout;
}

ID3D11DepthStencilView* D3DResources::GetDepthStencilView(void)
{
	return m_DepthStencilView;
}

ID3DBlob* D3DResources::BuildShaderBlob(const LPCWSTR fileName, LPCSTR shaderModel)
{
	HRESULT result = S_FALSE;
	ID3DBlob* shaderBlob = nullptr;

	const LPCSTR entryPoint = "main";
	result = compileShader(fileName, entryPoint, shaderModel, &shaderBlob);

	if (FAILED(result))
	{
		fprintf(stderr, "compileShader failed with error : INVALID_FILE_NAME | INVALID_SHADER_MODEL\n");
		SAFE_RELEASE(shaderBlob)
	}

	return shaderBlob;
}

ID3D11InputLayout* D3DResources::CreateInputLayout(const D3D11_INPUT_ELEMENT_DESC* layout,UINT numElement, ID3DBlob* vsBlob) const
{
	HRESULT result = S_FALSE;
	ID3D11InputLayout* outLayout = nullptr;

	result = m_Device->CreateInputLayout(layout, numElement, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &outLayout);
	if (FAILED(result))
	{
		fprintf(stderr, "createInputLayout failed with error : INVALID_LAYOUT | INVALID_NUM_ELEMENT | INVALID_BLOB\n");
		SAFE_RELEASE(outLayout)
	}

	return outLayout;
}











bool D3DResources::createBmpTexture(void)
{
	uint8_t* rgba = new uint8_t[width * height * 4];

	for (int i = 0; i < width * height; ++i)
	{
		rgba[i * 4 + 0] = 0; //m_rBuf->data[i];
		rgba[i * 4 + 1] = 0; //m_gBuf->data[i];
		rgba[i * 4 + 2] = m_bBuf->data[i];
		rgba[i * 4 + 3] = 255;
	}

	D3D11_TEXTURE2D_DESC texd = {};
	texd.Width = width;
	texd.Height = height;
	texd.MipLevels = 1;
	texd.ArraySize = 1;
	texd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texd.SampleDesc.Count = 1;
	texd.Usage = D3D11_USAGE_DEFAULT;
	texd.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = rgba;
	initData.SysMemPitch = width * 4;

	ID3D11Texture2D* texture = nullptr;

	HRESULT result;
	result = m_Device->CreateTexture2D(&texd, &initData, &texture);
	if (FAILED(result))
	{
		SAFE_RELEASE(texture)
		delete[] rgba;
		return false;
	}


	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = {};
	srvd.Format = texd.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvd.Texture2DArray.ArraySize = 1;
	srvd.Texture2DArray.FirstArraySlice = 0;
	srvd.Texture2DArray.MipLevels = 1;
	srvd.Texture2DArray.MostDetailedMip = 0;


	result = m_Device->CreateShaderResourceView(texture, &srvd, &m_loadedTextureArray);
	if (FAILED(result))
	{
		SAFE_RELEASE(m_loadedTextureArray)
		texture->Release();
		delete[] rgba;

		return false;
	}

	texture->Release();
	delete[] rgba;

	return true;
}

bool D3DResources::createVSAndPSAndInputLayout(void)
{
	HRESULT result = S_FALSE;

	Vertex_s vertices[] = {
		{ XMFLOAT2(-1.0f,  1.0f), XMFLOAT2(0.0f, 0.0f) },
		{ XMFLOAT2(1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT2(-1.0f, -1.0f), XMFLOAT2(0.0f, 1.0f) },
		{ XMFLOAT2(1.0f,  1.0f), XMFLOAT2(1.0f, 0.0f) },
		{ XMFLOAT2(1.0f, -1.0f), XMFLOAT2(1.0f, 1.0f) },
	};


	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(Vertex_s) * 6;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA initData = {};
	initData.pSysMem = vertices;

	result = m_Device->CreateBuffer(&bd, &initData, &m_VertexBuffer);

	if (FAILED(result))
	{
		fprintf(stderr, "creat vertexBuffer failed\n");
		goto LB_FAILED_CREATE_VERTEX_BUFFER;
	}

	{
		ID3D11VertexShader* basicVShader = nullptr;
		
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0}
		};

		ID3DBlob* vsBlob = nullptr;

		result = compileShader(L"defaultShader.fx", "vsMain", "vs_5_0", &vsBlob);
		if (FAILED(result))
		{
			if(vsBlob) vsBlob->Release();
			fprintf(stderr, "compile default vertexShader failed\n");
			goto LB_FAILED_COMPILE_VERTEX_SHADER;
		}

		result = m_Device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &basicVShader);
		if (FAILED(result))
		{
			vsBlob->Release();
			fprintf(stderr, "create Vertex shader failed \n");
			goto LB_FAILED_COMPILE_VERTEX_SHADER;
		}

		m_VertexShaders.push_back(std::move(basicVShader));

		result = m_Device->CreateInputLayout(layout, ARRAYSIZE(layout), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_InputLayout);
		if (FAILED(result))
		{
			vsBlob->Release();
			fprintf(stderr, "create inputLayout failed\n");


			goto LB_FAILED_CREATE_INPUT_LAYOUT;
		}

		vsBlob->Release();
	}


	{
		ID3DBlob* psBlob = nullptr;
		ID3D11PixelShader* basicPShader = nullptr;

		
		result = compileShader(L"defaultShader.fx", "psMain", "ps_5_0", &psBlob);
		if (FAILED(result))
		{
			if (psBlob) psBlob->Release();
			fprintf(stderr, "compile pixelShader failed\n");
			goto LB_FAILED_COMPILE_PIXEL_SHADER;
		}

		result = m_Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &basicPShader);
		if (FAILED(result))
		{
			psBlob->Release();
			fprintf(stderr, "create pixelShader failed\n");
			goto LB_FAILED_COMPILE_PIXEL_SHADER;
		}

		m_PixelShaders.push_back(std::move(basicPShader));
		psBlob->Release();
	}
	


	return true;

LB_FAILED_COMPILE_PIXEL_SHADER:
	m_InputLayout->Release();
	m_InputLayout = nullptr;

LB_FAILED_CREATE_INPUT_LAYOUT:
	{
		ID3D11VertexShader* vs = m_VertexShaders.at(0);
		vs->Release();
		m_VertexShaders.clear();
		m_VertexShaders.shrink_to_fit();
	}
LB_FAILED_COMPILE_VERTEX_SHADER:
	if (m_VertexBuffer) m_VertexBuffer->Release();

LB_FAILED_CREATE_VERTEX_BUFFER:

	return false;
}

bool D3DResources::getMaxVideoAdapter(void)
{
	IDXGIFactory1* pFactory = nullptr;
	UINT adapterIdx = 0;

	if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)&pFactory)))
	{
		return false;
	}

	DXGI_ADAPTER_DESC1 adapterDesc;
	size_t maxVideoMemSize = 0;
	IDXGIAdapter1* adapter;
	for (UINT i = 0; pFactory->EnumAdapters1(i, &adapter) != DXGI_ERROR_NOT_FOUND; ++i)
	{
		adapter->GetDesc1(&adapterDesc);
		if (adapterDesc.DedicatedVideoMemory > maxVideoMemSize)
		{
			adapterIdx = i;
			maxVideoMemSize = adapterDesc.DedicatedVideoMemory;
		}
		adapter->Release();
	}

	HRESULT hr = pFactory->EnumAdapters1(adapterIdx, &m_Adapter);
	pFactory->Release();
	if (hr == DXGI_ERROR_NOT_FOUND)
	{
		return false;
	}
	return true;
}

bool D3DResources::createDeviceAndSwapChain(void)
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = winW;
	sd.BufferDesc.Height = winH;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	UINT createDeviceFlag = 0;
#if defined(_DEBUG)
	createDeviceFlag |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	HRESULT hr = D3D11CreateDeviceAndSwapChain(m_Adapter, D3D_DRIVER_TYPE_UNKNOWN,
		NULL, createDeviceFlag, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
		&sd, &m_SwapChain, &m_Device, &m_featureLevel, &m_DevContext);

	if (FAILED(hr))
	{
		return false;
	}
	return true;
}

bool D3DResources::createRenderTargets(void)
{
	ID3D11Texture2D* pBackBuffer = nullptr;
	HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
	if (FAILED(hr))
	{
		return false;
	}

	hr = m_Device->CreateRenderTargetView(pBackBuffer, nullptr, &m_RTView);
	if (FAILED(hr))
	{
		pBackBuffer->Release();
		goto LB_FAILED_CREATE_RENDER_TARGET_VIEW;
	}
	pBackBuffer->Release();


	D3D11_TEXTURE2D_DESC descDepth;
	ZeroMemory(&descDepth, sizeof(descDepth));
	descDepth.Width = winW;
	descDepth.Height = winH;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	hr = m_Device->CreateTexture2D(&descDepth, nullptr, &m_DepthStencil);
	if (FAILED(hr))
	{
		fprintf(stderr, "create depthStencil failed\n");
		goto LB_FAILED_CREATE_DEPTH_STENCIL;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(descDSV));
	descDSV.Format = descDepth.Format;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	hr = m_Device->CreateDepthStencilView(m_DepthStencil, &descDSV, &m_DepthStencilView);
	if (FAILED(hr))
	{
		goto LB_FAILED_CREATE_DEPTH_STENCIL_VIEW;
	}
	m_DevContext->OMSetRenderTargets(1, &m_RTView, m_DepthStencilView);

	return true;
LB_FAILED_CREATE_DEPTH_STENCIL_VIEW:
	m_DepthStencil->Release();
LB_FAILED_CREATE_DEPTH_STENCIL:
	m_RTView->Release();
LB_FAILED_CREATE_RENDER_TARGET_VIEW:
	return false;
}

HRESULT D3DResources::loadTexture(void)
{
	HRESULT result = S_OK;

	static const int FILE_TYPE_SIZE = 4;
	static const int IMAGE_WIDTH  = 512; // to update image at once
	static const int IMAGE_HEIGHT = 512;
	const int stagedCount = m_stagedTextures.size();
	
	int validTextureCount = 0;

	if (stagedCount == 0) return S_FALSE;
	
	D3D11_SUBRESOURCE_DATA* subData = new D3D11_SUBRESOURCE_DATA[stagedCount];
	ID3D11Texture2D* texture = nullptr;
	
	for (int i = 0; i < stagedCount; ++i)
	{
		Texture_s& temp = m_stagedTextures.at(i);

		if (temp.fileName == L"")
		{
			// TODO : (LATER) this texture is just alpha or r, g, b. So handle this texture manualy
			// TODO : (DESIGN_NOTE) create basic texture and set r, g, b value and a value  


			continue;
		}
		

		size_t pos = std::wstring(temp.fileName).find('.');
		std::wstring fileType = std::wstring(temp.fileName).substr(pos);

		if (fileType.size() > FILE_TYPE_SIZE || fileType.size() < FILE_TYPE_SIZE)
		{
			fprintf(stderr, "fileName argument error : (%ls) INVLAID_FILE_NAME | INVLIAD_FILE_TYPE \n", temp.fileName);
			goto LB_RETURN;
		}

		if (L".dds" == fileType)
		{
			result = LoadFromDDSFile(temp.fileName, DDS_FLAGS_NONE, &temp.imageMetaData, temp.imageData);
		}
		else
		{
			result = LoadFromWICFile(temp.fileName, WIC_FLAGS_NONE, &temp.imageMetaData, temp.imageData);
		}
		
		if (FAILED(result))
		{
			fprintf(stderr, "load frome file failed with error : INVALID_FILE_FORMAT\n");
			goto LB_RETURN;
		}

		if (temp.imageMetaData.mipLevels > 1) { temp.imageMetaData.mipLevels = 1; }

		result = Resize(temp.imageData.GetImages(), temp.imageData.GetImageCount(), temp.imageMetaData, IMAGE_WIDTH, IMAGE_HEIGHT, TEX_FILTER_DEFAULT, temp.imageData);
		if (FAILED(result))
		{
			fprintf(stderr, "resize image failed : INDEX(%d)\n", i);
			goto LB_RETURN;
		}


		const Image* img = temp.imageData.GetImage(0, 0, 0);
		
		subData[i].pSysMem = img->pixels;
		subData[i].SysMemPitch = img->rowPitch;
		subData[i].SysMemSlicePitch = img->slicePitch;
		validTextureCount++;
	}

	if (validTextureCount == 0) { goto LB_RETURN; }

	

	D3D11_TEXTURE2D_DESC texd;
	ZeroMemory(&texd, sizeof(texd));
	texd.Width = IMAGE_WIDTH;
	texd.Height = IMAGE_HEIGHT;
	texd.MipLevels = 1;
	texd.ArraySize = static_cast<UINT>(stagedCount);
	texd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texd.SampleDesc.Count = 1;
	texd.SampleDesc.Quality = 0;
	texd.Usage = D3D11_USAGE_DEFAULT;
	texd.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texd.CPUAccessFlags = 0;
	texd.MiscFlags = 0;

	result = m_Device->CreateTexture2D(&texd, subData, &texture);
	if (FAILED(result))
	{
		fprintf(stderr, "createTexture2D failed\n");
		goto LB_RETURN;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	srvd.Format = texd.Format;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
	srvd.Texture2DArray.ArraySize = texd.ArraySize;
	srvd.Texture2DArray.FirstArraySlice = 0;
	srvd.Texture2DArray.MipLevels = texd.MipLevels;
	srvd.Texture2DArray.MostDetailedMip = 0;

	result = m_Device->CreateShaderResourceView(texture, &srvd, &m_loadedTextureArray);
	if (FAILED(result))
	{
		fprintf(stderr, "createShaderResourceView failed\n");
		goto LB_RETURN;
	}

LB_RETURN:

	if (texture) { texture->Release(); }
	delete[] subData;

	return result;
}

HRESULT D3DResources::compileShader(LPCWSTR fileName, LPCSTR entryPoint, LPCSTR shaderModel, ID3DBlob** blob)
{
	HRESULT result = S_FALSE;
	ID3DBlob* errorBlob = nullptr;

	UINT shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
	shaderFlags |= D3DCOMPILE_DEBUG;
#endif

	result = D3DCompileFromFile(fileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint, shaderModel, shaderFlags, 0, blob, &errorBlob);

	if (FAILED(result))
	{
		fprintf(stderr, "complieFromFile failed with error : %s\n", (char*)errorBlob->GetBufferPointer());

		if (*blob) { (*blob)->Release();}
		goto LB_RETURN;
	}

	if (errorBlob) { errorBlob->Release(); }
	result = S_OK;

LB_RETURN:

	return result;
}

void D3DResources::createClientSizeViewPort(void)
{
	D3D11_VIEWPORT vp = {};
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	vp.Width = (FLOAT)winW;
	vp.Height = (FLOAT)winH;
	vp.MinDepth = 0;
	vp.MaxDepth = 1.0f;

	m_DevContext->RSSetViewports(1, &vp);
}