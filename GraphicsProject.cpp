// GraphicsProject.cpp : Defines the entry point for the application.

#define NOMINMAX

#include <chrono>

#include "framework.h"
#include "GraphicsProject.h"
#include "D3DResources.h"
#include "Camera.h"

#include "GLTFTypes.h"
#include "GLTFReader.h"

#include "TestObject.h"


#ifdef _DEBUG
#pragma comment(linker, "/entry:wWinMainCRTStartup /subsystem:console")
#endif

#define MAX_LOADSTRING 100

// TODO : (IMP | LATER ) move this define to EntityResource Class 
#define MODEL_PATH ("C://Users//james//source//Models//chisa//scene.gltf")


typedef std::chrono::time_point<std::chrono::high_resolution_clock> timepoint_t;
using namespace DirectX;


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
HWND hWnd;

float g_DeltaTime = 0;
float g_RenderTime = 0;
timepoint_t g_LastTime;

D3DResources g_resource;
Camera* g_cam = nullptr;

GLTFReader g_Reader;

TestObject testObject;


XMMATRIX g_world;
XMMATRIX g_view;
XMMATRIX g_projection;

// TODO : (IMP | LATER ) move this define to EntityResource Class 
NodeList_s g_NodeList;
MeshList_s g_MeshList;
Skin_s g_Skin;




// Forward declarations of functions included in this code module:
bool InitObject(void);
void RenderFrame(void);
void CloseObjectHandles(void);
void UpdateDeltaTime(void);

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);




int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_GRAPHICSPROJECT, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_GRAPHICSPROJECT));

    MSG msg = { 0 };

    if (!InitObject())
    {
        goto LB_CLOSE;
    }    


    // Main message loop:
    while (WM_QUIT != msg.message)
    {
        UpdateDeltaTime();

        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        RenderFrame();
    }
    
LB_CLOSE:
    CloseObjectHandles();
    return (int) msg.wParam;
}



bool InitObject(void)
{
    if (!g_resource.Initialize(hWnd))
    {
        fprintf(stderr, "Resource Initialization failed \n");
        goto LB_FAILED_INIT_RESOURCE;
    }

    if (!g_Reader.Initialize(MODEL_PATH))
    {
        fprintf(stderr, "GLTFReader failed with error while reading %s\n", MODEL_PATH);
        goto LB_FAILED_READ_MODEL_GLTF;
    }

    if (!g_Reader.GetMeshes(g_MeshList))
    {
        fprintf(stderr, "GetMesh failed with error\n");
        goto LB_FAILED_GET_MESHLIST;
    }

    if (!g_Reader.GetNodes(g_NodeList))
    {
        fprintf(stderr, "GetNodes failed with error\n");
        goto LB_FAILED_GET_NODES;
    }


    { // Setup Camera 
        // TODO : (LATER) make camera system

        XMFLOAT4 eye = { 0.0f, 0.0f, -3.0f, 0.0f };
        XMFLOAT4 at  = { 0.0f, 0.0f, 0.0f, 0.0f };
        XMFLOAT4 up  = { 0.0f, 1.0f, 0.0f, 0.0f };
        g_cam = new Camera(eye, at, up);
        g_cam->Initialize(g_resource);
    }

    if (!testObject.Initialize(g_resource, g_MeshList, g_NodeList))
    {
        fprintf(stderr, "testObject initialization failed with error\n");
        goto LB_FAILED_TESTOBJ_INITIALIZE;
    }
    

    return true;

LB_FAILED_TESTOBJ_INITIALIZE:
    if (g_NodeList.count)
    {
        delete[] g_NodeList.list;
        g_NodeList.list = nullptr;
    }

LB_FAILED_GET_NODES:
    if (g_MeshList.count)
    {
        for (int i = 0; i < g_MeshList.count; ++i)
        {
            Mesh_s& mesh = g_MeshList.list[i];
            delete[] mesh.indices;
            delete[] mesh.vertices;
        }
        delete[] g_MeshList.list;
        g_MeshList.list = nullptr;
    }

LB_FAILED_GET_MESHLIST:

LB_FAILED_READ_MODEL_GLTF:

LB_FAILED_INIT_RESOURCE:

    return false;
}



void RenderFrame(void)
{
    const static float BGWP_COLOR[] = { 1.0f, 1.0f, 0.0f, 1.0f };

    ID3D11Device* device = g_resource.GetDevice();
    ID3D11DeviceContext* devContext = g_resource.GetContext();
    ID3D11RenderTargetView* rtView = g_resource.GetRTView();
    ID3D11DepthStencilView* dsView = g_resource.GetDepthStencilView();
    IDXGISwapChain* swapChain = g_resource.GetSwapChain();

    devContext->ClearRenderTargetView(rtView, BGWP_COLOR);
    devContext->ClearDepthStencilView(dsView, D3D11_CLEAR_DEPTH, 1.0f, 0);

    testObject.Draw(g_cam);

    swapChain->Present(1, 0);
}



void CloseObjectHandles(void)
{
    g_resource.CloseD3DHandles();
}



void UpdateDeltaTime(void)
{
    timepoint_t currentTime = std::chrono::high_resolution_clock::now();
    g_DeltaTime = std::chrono::duration<float>(currentTime - g_LastTime).count();

    if (g_DeltaTime > 0.1f)
    {
        g_DeltaTime = 0.1f;
    }

    g_LastTime = currentTime;
}

















//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_GRAPHICSPROJECT));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_GRAPHICSPROJECT);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_CHAR:
        switch(wParam)
        {
        case'w':
        case'W':

            break;
        case'a':
        case'A':

            break;

        case 's':
        case 'S':

            break;
    
        case'd':
        case'D':

            break;

        }

        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
