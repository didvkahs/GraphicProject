#include <DirectXMath.h>

#include "Camera.h"
#include "D3DResources.h"

using namespace DirectX;


Camera::Camera(void) {}

Camera::Camera(XMFLOAT4 eye, XMFLOAT4 at, XMFLOAT4 up)
{
    XMVECTOR veye = XMLoadFloat4(&eye);
    XMVECTOR vat = XMLoadFloat4(&at);
    XMVECTOR vup = XMLoadFloat4(&up);

    m_position = eye;

    XMVECTOR forward = XMVector3Normalize(XMVectorSubtract(vat, veye));
    XMStoreFloat4(&m_forward, forward);
   
    XMVECTOR right = XMVector3Normalize(XMVector3Normalize(XMVector3Cross(vup, forward)));
    XMStoreFloat4(&m_right, right);

    XMVECTOR trueUp = XMVector3Normalize(XMVector3Normalize(XMVector3Cross(forward, right)));
    XMStoreFloat4(&m_up, trueUp);
}


bool Camera::Initialize(D3DResources& resource)
{
    m_Resource = &resource;
    HRESULT result = S_OK;
    ID3D11Device* device = m_Resource->GetDevice();
    
    setWorld();
    setView(); // TODO : (IMP | RM) remove this function after render GLTF model
    setProjection();

    if (!createBuffer())
    {
        return false;
    }

    return true;
}

void Camera::Update(const float& deltaTime, MoveCam_e type)
{
    // NOTICE : (LATER) use this function after render GLTF model
    // TODO : (LATER) fix this function ( lots of error )


    XMMATRIX rot = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);
    
    XMVECTOR vforward = XMLoadFloat4(&m_forward);
    vforward = XMVector3TransformNormal(vforward, rot);
    XMStoreFloat4(&m_forward, vforward);

    XMVECTOR vright = XMLoadFloat4(&m_right);
    vright = XMVector3TransformNormal(vright, rot);
    XMStoreFloat4(&m_right, vright);

    XMVECTOR vup = XMVector3Cross(vforward, vright);
    vup = XMVector3Normalize(vup);
    XMStoreFloat4(&m_up, vup);
    
    XMVECTOR vpos = XMLoadFloat4(&m_position);

    vpos += vforward * CAM_SPEED * deltaTime;
    vpos += vright * CAM_SPEED * deltaTime;
    vpos += vup * CAM_SPEED * deltaTime;

    XMMATRIX view = XMMatrixLookAtLH(vpos, XMVectorAdd(vpos, vforward), vup);
    XMStoreFloat4x4(&m_view, view);
}


ID3D11Buffer* Camera::GetCBWorld(void) const
{
    return m_CBWorld;
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
    if (m_CBWorld)
    {
        m_CBWorld->Release();
        m_CBWorld = nullptr;
    }

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

void Camera::setView(void)
{
    XMMATRIX rot = XMMatrixRotationRollPitchYaw(m_Pitch, m_Yaw, 0.0f);

    XMVECTOR vforward = XMLoadFloat4(&m_forward);
    vforward = XMVector3TransformNormal(vforward, rot);
    XMStoreFloat4(&m_forward, vforward);

    XMVECTOR vright = XMLoadFloat4(&m_right);
    vright = XMVector3TransformNormal(vright, rot);
    XMStoreFloat4(&m_right, vright);

    XMVECTOR vup = XMVector3Cross(vforward, vright);
    vup = XMVector3Normalize(vup);
    XMStoreFloat4(&m_up, vup);

    XMVECTOR vpos = XMLoadFloat4(&m_position);

    XMMATRIX view = XMMatrixLookAtLH(vpos, XMVectorAdd(vpos, vforward), vup);
    XMStoreFloat4x4(&m_view, view);
}

void Camera::setProjection(void)
{
    float fovAngleY = XMConvertToRadians(45.0f);
    float aspectRatio = static_cast<float>(m_Resource->GetWidth()) / static_cast<float>(m_Resource->GetHeight());

    float nearz = 0.1f;
    float farz = 1000.0f;

    XMMATRIX projection = XMMatrixPerspectiveLH(fovAngleY, aspectRatio, nearz, farz);
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
    
    XMMATRIX tWorld= XMLoadFloat4x4(&m_world);
    tWorld = XMMatrixTranspose(tWorld);

    XMMATRIX tView = XMLoadFloat4x4(&m_view);
    tView = XMMatrixTranspose(tView);

    XMMATRIX tProj = XMLoadFloat4x4(&m_projection);
    tProj = XMMatrixTranspose(tProj);


    srd.pSysMem = &tWorld;
    
    result = device->CreateBuffer(&bd, &srd, &m_CBWorld);
    if (FAILED(result))
    {
        fprintf(stderr, "create world constBuffer failed with error\n");
        goto LB_FAILED_CREATE_CBWORLD;
    }

    // TODO: (LATER) move createView function to another place
    // The view matrix represents the camera's position and orientation
    // It defines where the user is looking, so it must be updated every frame

    srd.pSysMem = &tView;

    result = device->CreateBuffer(&bd, &srd, &m_CBView);
    if (FAILED(result))
    {
        fprintf(stderr, "create view constBuffer failed with error\n");
        goto LB_FAILED_CREAET_CBVIEW;
    }

    srd.pSysMem = &tProj;

    result = device->CreateBuffer(&bd, &srd, &m_CBProjection);
    if (FAILED(result))
    {
        fprintf(stderr, "create projection buffer failed with error\n");
        goto LB_FAILED_CREATE_CBPROJECTION;
    }
    
    return true;

LB_FAILED_CREATE_CBPROJECTION:
    m_CBView->Release();
    m_CBView = nullptr;

LB_FAILED_CREAET_CBVIEW:
    m_CBWorld->Release();
    m_CBWorld = nullptr;

LB_FAILED_CREATE_CBWORLD:
    return false;
}