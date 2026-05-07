#pragma once


struct ID3D11Buffer;
class D3DResources;


enum MoveCam_e
{
	MOV_FORWARD,
	MOV_BACK,
	MOV_RIGHT,
	MOV_LEFT,
	MOV_UP,
	MOV_DOWN
};


class Camera
{
public:
	Camera(void);
	Camera(DirectX::XMFLOAT4 eye, DirectX::XMFLOAT4 at, DirectX::XMFLOAT4 up);

	bool Initialize(D3DResources& resource);

	void Update(const float& deltaTime, MoveCam_e type);

	ID3D11Buffer* GetCBWorld(void) const;
	ID3D11Buffer* GetCBView(void) const;
	ID3D11Buffer* GetCBProjection(void) const;

	void CloseCameraHandles(void);

private:

	void setWorld(void);
	void setView(void); // TODO : (IMP | RM) remove this function after render GLTF model
	void setProjection(void);
	bool createBuffer(void);

private:
	const float CAM_SPEED = 1.0f;
	
	D3DResources* m_Resource = nullptr;

	DirectX::XMFLOAT4X4 m_world;
	DirectX::XMFLOAT4X4 m_view;
	DirectX::XMFLOAT4X4 m_projection;

	ID3D11Buffer* m_CBWorld = nullptr;
	ID3D11Buffer* m_CBView = nullptr;
	ID3D11Buffer* m_CBProjection = nullptr;

	float m_Yaw = 0.0f;
	float m_Pitch = 0.0f;

	DirectX::XMFLOAT4 m_position;
	DirectX::XMFLOAT4 m_forward;
	DirectX::XMFLOAT4 m_right;
	DirectX::XMFLOAT4 m_up;
};