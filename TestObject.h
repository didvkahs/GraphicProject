#pragma once

#include "IEntity.h"

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

class TestObject : public IEntity
{
public:

	TestObject(void);
	~TestObject(void);

	bool Initialize(D3DResources& resource, EntityResource* entitiyResource) override;

	void Update(void) override;
	void Draw(Camera* cam) override;

	void CloseIEntityHandles(void) override;

private:
	bool setupD3D(void);
	bool createSRView(void);
	bool createBuffer(void);

private:

	struct TestVertex_s
	{
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT4 nor;
		DirectX::XMFLOAT2 tex;
	};

private:

	DirectX::XMFLOAT4X4 m_World;
	DirectX::XMFLOAT4X4 m_scale;

	int m_vsIDs[VS_COUNT];
	int m_psIDs[PS_COUNT];
	VertexShaderID_e m_currVSID = VS_DEFAULT;
	PixelShaderID_e m_currPSID = PS_DEFAULT;

	TestVertex_s* m_GlobalVertices = nullptr;
	int m_VertexCount = 0;
};