#include <DirectXMath.h>

#include "Camera.h"
#include "D3DResources.h"

using namespace DirectX;


Camera::Camera(void)
{
    XMVECTOR eye = { 0.0f, 0.0f, -3.0f, 0.0f };
    XMVECTOR at = { 0.0f, 0.0f, 0.0f, 0.0f };
    XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };

    XMStoreFloat4(&m_position, eye);
    XMMATRIX view = XMMatrixLookAtLH(eye, at, up);
    XMStoreFloat4x4(&m_view, view);
    
    XMVECTOR forward = XMVector4Normalize(at - eye);
    XMStoreFloat4(&m_forward, forward);
    m_right = { 1.0f, 0.0f, 0.0f, 0.0f };
    XMStoreFloat4(&m_up, up);
}


bool Camera::Initialize(D3DResources& resource)
{
    m_Resource = &resource;
    HRESULT result = S_OK;
    ID3D11Device* device = m_Resource->GetDevice();
    
    setWorld();
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
    
    XMVECTOR standardForward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
    XMVECTOR standardRight = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
    XMVECTOR worldUp = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);


    XMVECTOR vforward = XMVector3TransformNormal(standardForward, rot);
    XMVECTOR vright = XMVector3TransformNormal(standardRight, rot);

    XMVECTOR vup = XMVector3Cross(vforward, vright);
    vup = XMVector3Normalize(vup);
    
    XMVECTOR vpos = XMLoadFloat4(&m_position);
    
    float speed = CAM_SPEED * deltaTime;

    if (type == MOV_FORWARD) vpos += vforward * speed;
    if (type == MOV_BACK)    vpos -= vforward * speed;
    if (type == MOV_RIGHT)   vpos += vright * speed;
    if (type == MOV_LEFT)    vpos -= vright * speed;
    if (type == MOV_UP)      vpos += vup * speed; 
    if (type == MOV_DOWN)    vpos -= vup * speed;

    XMStoreFloat4(&m_position, vpos);
    XMStoreFloat4(&m_forward, vforward);
    XMStoreFloat4(&m_right, vright);
    XMStoreFloat4(&m_up, vup);

    XMMATRIX view = XMMatrixLookAtLH(vpos, XMVectorAdd(vpos, vforward), vup);
    view = XMMatrixTranspose(view);

    HRESULT hr = S_OK;
    ID3D11DeviceContext* devcon = m_Resource->GetContext();
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    if (FAILED(devcon->Map(m_CBView, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
    {
#ifdef _DEBUG
        __debugbreak();
#endif
        fprintf(stderr, "map cbview failed with error\n");
    }

    memcpy(mappedResource.pData, &view, sizeof(XMFLOAT4X4));

    devcon->Unmap(m_CBView, 0);
}

void Camera::UpdateYawPitch(const float& deltaTime, int yaw, int pitch)
{
    m_Yaw += yaw * deltaTime * MOUSE_SPEED;
    m_Pitch += pitch * deltaTime * MOUSE_SPEED;
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

void Camera::setProjection(void)
{
    float fovAngleY = XMConvertToRadians(45.0f);
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
    m_CBWorld->Release();
    m_CBWorld = nullptr;

LB_FAILED_CREATE_CBWORLD:
    return false;
}