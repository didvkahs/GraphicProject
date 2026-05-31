#pragma once

// TODO : (IMP) remove world matrix



struct ID3D11Buffer;
class D3DResources;


enum MoveCam_e
{
	MOV_FORWARD,
	MOV_BACK,
	MOV_RIGHT,
	MOV_LEFT,
	MOV_UP,
	MOV_DOWN,
	MOV_MOUSE,
	MOV_COUNT
};

enum FovSetting_e
{
	FOV_PC = 75,
	FOV_CONSOLE = 65
};


class Camera
{
public:
	Camera(void);
	Camera(FovSetting_e fov);

	bool Initialize(D3DResources& resource);

	void Update(const float& deltaTime, MoveCam_e type);
	void UpdateYawPitch(const float& deltaTime, int yaw, int pitch);
	
	void SetFov(FovSetting_e fov);
	DirectX::XMMATRIX GetWorldMat(void) const;
	ID3D11Buffer* GetCBView(void) const;
	ID3D11Buffer* GetCBProjection(void) const;

	void CloseCameraHandles(void);

private:

	void setWorld(void);
	void setProjection(void);
	bool createBuffer(void);

private:
	const float CAM_SPEED = 0.6f;
	const float MOUSE_SPEED = 0.1f;
	
	D3DResources* m_Resource = nullptr;

	FovSetting_e m_fov;

	DirectX::XMFLOAT4X4 m_world;
	DirectX::XMFLOAT4X4 m_view;
	DirectX::XMFLOAT4X4 m_projection;

	ID3D11Buffer* m_CBView = nullptr;
	ID3D11Buffer* m_CBProjection = nullptr;

	float m_currYaw = 0.0f;
	float m_currPitch = 0.0f;
	float m_Yaw = 0.0f;
	float m_Pitch = 0.0f;

	DirectX::XMFLOAT4 m_position;
	DirectX::XMFLOAT4 m_forward;
	DirectX::XMFLOAT4 m_right;
	DirectX::XMFLOAT4 m_up;
};