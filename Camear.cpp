#include <DirectXMath.h>

#include "Camera.h"
#include "D3DResources.h"

using namespace DirectX;

Camera::Camera(void)
{
    XMVECTOR eye = { 0.0f, 0.0f, -3.0f, 0.0f };
    XMVECTOR at = { 0.0f, 0.0f, 0.0f, 0.0f };
    XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };

    XMStoreFloat4(&m_currPos, eye);
    XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
    XMStoreFloat4x4(&m_view, view);

    m_fov = FOV_PC;
}

Camera::Camera(FovSetting_e fov) : Camera()
{
    m_fov = fov;
}


bool Camera::Initialize(D3DResources& resource)
{
    m_Resource = &resource;
    ID3D11Device* device = m_Resource->GetDevice();
    
    setWorld();
    setProjection();

    if (!createBuffer())
    {
        return false;
    }

    return true;
}

void Camera::ProcessKeyboardInput(const float& deltaTime, MoveCam_e type)
{
    XMMATRIX rot = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);
    XMVECTOR vforward = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rot);
    XMVECTOR vright = XMVector3TransformNormal(XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), rot);

    XMVECTOR vup = XMVector3Cross(vforward, vright);
    vup = XMVector3Normalize(vup);

    XMVECTOR vpos = XMLoadFloat4(&m_targetPos); 

    float speed = CAM_SPEED * deltaTime;

    if (type == MOV_FORWARD) vpos += vforward * speed;
    if (type == MOV_BACK)    vpos -= vforward * speed; 
    if (type == MOV_RIGHT)   vpos += vright * speed;
    if (type == MOV_LEFT)    vpos -= vright * speed;
    if (type == MOV_UP)      vpos += vup * speed;
    if (type == MOV_DOWN)    vpos -= vup * speed;

    XMStoreFloat4(&m_targetPos, vpos);
}

void Camera::UpdateMatrix(const float& deltaTime)
{
    float interpolationFactor = 12.0f * deltaTime;
    if (interpolationFactor > 1.0f) interpolationFactor = 1.0f;

    m_currYaw = m_currYaw + (m_Yaw - m_currYaw) * interpolationFactor;
    m_currPitch = m_currPitch + (m_Pitch - m_currPitch) * interpolationFactor;

    XMVECTOR curPos = XMLoadFloat4(&m_currPos);
    XMVECTOR tarPos = XMLoadFloat4(&m_targetPos);
    curPos = XMVectorLerp(curPos, tarPos, interpolationFactor);
    XMStoreFloat4(&m_currPos, curPos);

    XMMATRIX rot = XMMatrixRotationRollPitchYaw(m_currPitch, m_currYaw, 0.0f);
    XMVECTOR vforward = XMVector3TransformNormal(XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f), rot);
    XMVECTOR vup = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    XMMATRIX view = XMMatrixLookAtLH(curPos, curPos + vforward, vup);
    view = XMMatrixTranspose(view);

    ID3D11DeviceContext* devcon = m_Resource->GetContext();
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (SUCCEEDED(devcon->Map(m_CBView, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
        XMStoreFloat4x4(static_cast<XMFLOAT4X4*>(mappedResource.pData), view);
        devcon->Unmap(m_CBView, 0);
    }
}

void Camera::UpdateYawPitch(const float& deltaTime, int yaw, int pitch)
{
    m_Yaw += yaw * deltaTime * MOUSE_SPEED;
    m_Pitch += pitch * deltaTime * MOUSE_SPEED;

    const float limit = XMConvertToRadians(89.0f);
    if (m_Pitch > limit)  m_Pitch = limit;
    if (m_Pitch < -limit) m_Pitch = -limit;
}


void Camera::SetFov(FovSetting_e fov)
{
    m_fov = fov;
}

XMMATRIX Camera::GetWorldMat(void) const
{
    return XMLoadFloat4x4(&m_world);
}

ID3D11Buffer* Camera::GetCBView(void) const
{
    return m_CBView;
}

ID3D11Buffer* Camera::GetCBProjection(void) const
{
    return m_CBProjection;
}

void Camera::CloseCameraHandles(void)
{
    if (m_CBView)
    {
        m_CBView->Release();
        m_CBView = nullptr;
    }

    if (m_CBProjection)
    {
        m_CBProjection->Release();
        m_CBProjection = nullptr;
    }
}


void Camera::setWorld(void)
{
    XMMATRIX scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);
    XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);
    XMMATRIX translation = XMMatrixTranslation(0.0f, 0.0f, 0.0f);

    XMMATRIX world = scale * rotation * translation;
    XMStoreFloat4x4(&m_world, world);
}

void Camera::setProjection(void)
{
    float fovAngleY = XMConvertToRadians((float)m_fov);
    float aspectRatio = static_cast<float>(m_Resource->GetWidth()) / static_cast<float>(m_Resource->GetHeight());

    float nearz = 0.1f;
    float farz = 1000.0f;

    XMMATRIX projection = XMMatrixPerspectiveFovLH(fovAngleY, aspectRatio, nearz, farz);
    XMStoreFloat4x4(&m_projection, projection);
}

bool Camera::createBuffer(void)
{
    ID3D11Device* device = m_Resource->GetDevice();

    HRESULT result = S_OK;

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.ByteWidth = sizeof(XMFLOAT4X4);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

    D3D11_SUBRESOURCE_DATA srd = {};

    XMMATRIX tView = XMLoadFloat4x4(&m_view);
    tView = XMMatrixTranspose(tView);

    XMMATRIX tProj = XMLoadFloat4x4(&m_projection);
    tProj = XMMatrixTranspose(tProj);


    XMFLOAT4X4 ftproj;
    XMStoreFloat4x4(&ftproj, tProj);
    srd.pSysMem = &ftproj;

    result = device->CreateBuffer(&bd, &srd, &m_CBProjection);
    if (FAILED(result))
    {
        fprintf(stderr, "create projection buffer failed with error\n");
        goto LB_FAILED_CREATE_CBPROJECTION;
    }

    bd.Usage = D3D11_USAGE_DYNAMIC;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;



    XMFLOAT4X4 ftview;
    XMStoreFloat4x4(&ftview, tView);
    srd.pSysMem = &ftview;

    result = device->CreateBuffer(&bd, &srd, &m_CBView);
    if (FAILED(result))
    {
        fprintf(stderr, "create view constBuffer failed with error\n");
        goto LB_FAILED_CREAET_CBVIEW;
    }

    return true;

LB_FAILED_CREAET_CBVIEW:
    m_CBProjection->Release();
    m_CBProjection = nullptr;

LB_FAILED_CREATE_CBPROJECTION:
    return false;
}