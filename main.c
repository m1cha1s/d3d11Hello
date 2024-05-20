/*

This is according to this guide:
https://antongerdelan.net/opengl/d3d11.html

*/

#include <stdio.h>
#include <stdbool.h>
#include <float.h>

#define WIN32_LEAN_AND_MEAN
#define COBJMACROS
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <assert.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// GLOBALS

// D3D11
ID3D11Device* devicePtr                     = NULL;
ID3D11DeviceContext* deviceContextPtr       = NULL;
IDXGISwapChain* swapChainPtr                = NULL;
ID3D11RenderTargetView* renderTargetViewPtr = NULL;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ULONGLONG getDeltaTime();

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    
    const wchar_t CLASS_NAME[] = L"Sample Window Class";
    
    WNDCLASS wc = { 0 };
    
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = (LPCSTR)CLASS_NAME;
    
    RegisterClass(&wc);
    
    HWND hwnd = CreateWindowEx(
        0,
        (LPCSTR)CLASS_NAME,
        (LPCSTR)L"d3d11Hello",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
        NULL,
        NULL,
        hInstance,
        NULL
    );
    
    if (hwnd == NULL) {
        return 0;
    }
    
    // Initialize D3D11
    DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
    swapChainDesc.BufferDesc.RefreshRate.Numerator    = 0;
    swapChainDesc.BufferDesc.RefreshRate.Denominator  = 1;
    swapChainDesc.BufferDesc.Format     = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.SampleDesc.Count      = 1;
    swapChainDesc.SampleDesc.Quality    = 0;
    swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.BufferCount           = 1;
    swapChainDesc.OutputWindow          = hwnd;
    swapChainDesc.Windowed              = true;
    
    D3D_FEATURE_LEVEL featureLevel;
    UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;
    #if defined( DEBUG ) || defined( _DEBUG )
    flags |= D3D11_CREATE_DEVICE_DEBUG;
    #endif
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        NULL,
        D3D_DRIVER_TYPE_HARDWARE,
        NULL,
        flags,
        NULL,
        0,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &swapChainPtr,
        &devicePtr,
        &featureLevel,
        &deviceContextPtr
    );
    assert(S_OK==hr && swapChainPtr && devicePtr && deviceContextPtr);
    
    ID3D11Texture2D* framebuffer;
    // hr = swapChainPtr->lpVtbl->GetBuffer(swapChainPtr,
    hr = IDXGISwapChain_GetBuffer(
        swapChainPtr,
        0,
        &IID_ID3D11Texture2D,
        (void**)&framebuffer
    );
    assert(SUCCEEDED(hr));
    
    hr = ID3D11Device_CreateRenderTargetView(
        devicePtr,
        (ID3D11Resource*)framebuffer,
        0,
        &renderTargetViewPtr
    );
    assert(SUCCEEDED(hr));
    
    flags = D3DCOMPILE_ENABLE_STRICTNESS;
    #if defined( DEBUG ) || defined( _DEBUG )
        flags |= D3DCOMPILE_DEBUG;
    #endif
    ID3DBlob *vsBlobPtr = NULL, *psBlobPtr = NULL, *errorBlobPtr = NULL;
    
    // Compile vertex shader
    hr = D3DCompileFromFile(
        L"C:\\Users\\msule\\Dev\\d3d11Hello\\shaders.hlsl",
        NULL,
        NULL,
        //D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "vsMain",
        "vs_5_0",
        flags,
        0,
        &vsBlobPtr,
        &errorBlobPtr
    );
    if (FAILED(hr)) {
        if (errorBlobPtr) {
            OutputDebugStringA((char*)ID3D10Blob_GetBufferPointer(errorBlobPtr));
            ID3D10Blob_Release(errorBlobPtr);
        }
        if (vsBlobPtr) ID3D10Blob_Release(vsBlobPtr);
        assert(false);
    }
    
    // Compile pixel shader
    hr = D3DCompileFromFile(
        L"C:\\Users\\msule\\Dev\\d3d11Hello\\shaders.hlsl",
        NULL,
        NULL,
        //D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "psMain",
        "ps_5_0",
        flags,
        0,
        &psBlobPtr,
        &errorBlobPtr
    );
    if (FAILED(hr)) {
        if (errorBlobPtr) {
            OutputDebugStringA((char*)ID3D10Blob_GetBufferPointer(errorBlobPtr));
            ID3D10Blob_Release(errorBlobPtr);
        }
        if (psBlobPtr) ID3D10Blob_Release(psBlobPtr);
        //if (vsBlobPtr) ID3D10Blob_Release(vsBlobPtr); // Not sure if supposed to do this.
        assert(false);
    }
    
    ID3D11VertexShader* vertexShaderPtr = NULL;
    ID3D11PixelShader* pixelShaderPtr   = NULL;
    
    hr = ID3D11Device_CreateVertexShader(
        devicePtr,
        ID3D10Blob_GetBufferPointer(vsBlobPtr),
        ID3D10Blob_GetBufferSize(vsBlobPtr),
        NULL,
        &vertexShaderPtr
    );
    assert(SUCCEEDED(hr));
    
    hr = ID3D11Device_CreatePixelShader(
        devicePtr,
        ID3D10Blob_GetBufferPointer(psBlobPtr),
        ID3D10Blob_GetBufferSize(psBlobPtr),
        NULL,
        &pixelShaderPtr
    );
    assert(SUCCEEDED(hr));
    
    ID3D11InputLayout* inputLayoutPtr = NULL;
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UV", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    hr = ID3D11Device_CreateInputLayout(
        devicePtr,
        inputElementDesc,
        ARRAYSIZE(inputElementDesc),
        ID3D10Blob_GetBufferPointer(vsBlobPtr),
        ID3D10Blob_GetBufferSize(vsBlobPtr),
        &inputLayoutPtr
    );
    assert(SUCCEEDED(hr));
    
    // Create vertex buffer
    float vertexDataArray[] = {
        -0.5f,  0.5f,  0.0f,  0.0f,  0.0f,
         0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.0f,  1.0f,  1.0f,
    };
    UINT vertexStride = 5*sizeof(float);
    UINT vertexOffset = 0;
    UINT vertexCount  = 6;
    
    ID3D11Buffer* vertexBufferPtr = NULL;
    
    D3D11_BUFFER_DESC vertexBuffDesc = { 0 };
    vertexBuffDesc.ByteWidth = sizeof(vertexDataArray);
    vertexBuffDesc.Usage = D3D11_USAGE_DYNAMIC;
    vertexBuffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBuffDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    D3D11_SUBRESOURCE_DATA srData = { 0 };
    srData.pSysMem = vertexDataArray;
    hr = ID3D11Device_CreateBuffer(
        devicePtr,
        &vertexBuffDesc,
        &srData,
        &vertexBufferPtr
    );
    assert(SUCCEEDED(hr));
    
    // Load and set texture.
    
    int x,y,n;
    unsigned char *bitmap = stbi_load("res/aaa.png", &x, &y, &n, 4);
    assert(bitmap); // Make sure the bitmap has been loaded.
    
    ID3D11Texture2D* texture = NULL;
    D3D11_TEXTURE2D_DESC textureDesc = { 0 };
    textureDesc.Width = x;
    textureDesc.Height = y;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DYNAMIC;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    // textureDesc.MiscFlags = 
    
    D3D11_SUBRESOURCE_DATA srTex = { 0 };
    srTex.pSysMem = bitmap;
    srTex.SysMemPitch = x*4;
    
    hr = ID3D11Device_CreateTexture2D(
        devicePtr,
        &textureDesc,
        &srTex,
        &texture
    );
    assert(SUCCEEDED(hr));
    
    stbi_image_free(bitmap);
    
    ID3D11ShaderResourceView *textureShaderResViewPtr = NULL;
    
    hr = ID3D11Device_CreateShaderResourceView(
        devicePtr,
        (ID3D11Resource *)texture,
        NULL,
        &textureShaderResViewPtr
    );
    assert(SUCCEEDED(hr));
    
    D3D11_SAMPLER_DESC textureSamplerDesc = { 0 };
    textureSamplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    textureSamplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    textureSamplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    textureSamplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    textureSamplerDesc.MinLOD = -FLT_MAX;
    textureSamplerDesc.MaxLOD = FLT_MAX;
    textureSamplerDesc.MipLODBias = 0.0f;
    textureSamplerDesc.MaxAnisotropy = 1;
    textureSamplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    textureSamplerDesc.BorderColor[0] = 1.0f;
    textureSamplerDesc.BorderColor[1] = 1.0f;
    textureSamplerDesc.BorderColor[2] = 1.0f;
    textureSamplerDesc.BorderColor[3] = 1.0f;
    
    ID3D11SamplerState* samplerStatePtr = NULL;
    
    ID3D11Device_CreateSamplerState(
        devicePtr,
        &textureSamplerDesc,
        &samplerStatePtr
    );
    
    ShowWindow(hwnd, nCmdShow);
    
    MSG msg = { 0 };
    bool shouldClose = false;
    
    (void)getDeltaTime();
    
    while(!shouldClose) {
        if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if(msg.message == WM_QUIT) break;
        
        vertexDataArray[0] += 0.01f;
        if (vertexDataArray[0]>=1.0f) vertexDataArray[0] = -1.0f;
        
        D3D11_MAPPED_SUBRESOURCE resource = { 0 };
        ID3D11DeviceContext_Map(
            deviceContextPtr,
            (ID3D11Resource *)vertexBufferPtr,
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &resource
        );
        memcpy(resource.pData, vertexDataArray, sizeof(vertexDataArray));
        ID3D11DeviceContext_Unmap(
            deviceContextPtr,
            (ID3D11Resource *)vertexBufferPtr,
            0,
        );
        
        // Clear background
        float backgroundColor[4] = {
            0x64 / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f
        };
        ID3D11DeviceContext_ClearRenderTargetView(
            deviceContextPtr,
            renderTargetViewPtr,
            backgroundColor
        );
        
        // Set the viewport
        RECT winRect;
        GetClientRect(hwnd, &winRect);
        D3D11_VIEWPORT viewport = {
            0.0f,
            0.0f,
            (FLOAT)(winRect.right-winRect.left),
            (FLOAT)(winRect.bottom-winRect.top),
            0.0f,
            1.0f
        };
        ID3D11DeviceContext_RSSetViewports(deviceContextPtr, 1,&viewport);
        
        // Set Output Merger
        ID3D11DeviceContext_OMSetRenderTargets(deviceContextPtr, 1, &renderTargetViewPtr, NULL);
        
        // Set Input Assembler
        ID3D11DeviceContext_IASetPrimitiveTopology(
            deviceContextPtr,
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
        );
        ID3D11DeviceContext_IASetInputLayout(
            deviceContextPtr,
            inputLayoutPtr
        );
        
        ID3D11DeviceContext_PSSetShaderResources(
            deviceContextPtr,
            0, 1,
            &textureShaderResViewPtr
        );
        ID3D11DeviceContext_PSSetSamplers(
            deviceContextPtr,
            0, 1,
            &samplerStatePtr
        );
        
        ID3D11DeviceContext_IASetVertexBuffers(
            deviceContextPtr,
            0,
            1,
            &vertexBufferPtr,
            &vertexStride,
            &vertexOffset
        );
        
        // Set Shaders
        ID3D11DeviceContext_VSSetShader(
            deviceContextPtr,
            vertexShaderPtr,
            NULL,
            0
        );
        ID3D11DeviceContext_PSSetShader(
            deviceContextPtr,
            pixelShaderPtr,
            NULL,
            0
        );
        
        // Finally draw the triangle
        ID3D11DeviceContext_Draw(
            deviceContextPtr,
            vertexCount,
            0
        );
        
        // Swap the Buffers(present the frame)
        IDXGISwapChain_Present(
            swapChainPtr,
            1,
            0
        );
        
        ULONGLONG dt = getDeltaTime()/10000;
        printf("%I64u ms\n", dt);
    }
    
    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch(uMsg) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    // case WM_PAINT: {
    //     PAINTSTRUCT ps;
    //     HDC hdc = BeginPaint(hwnd, &ps);
        
    //     FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
        
    //     EndPaint(hwnd, &ps);
    // } return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

ULONGLONG getDeltaTime() {
    static ULONGLONG prevTime = 0;
    
    SYSTEMTIME st = {0};
    GetSystemTime(&st);
    
    FILETIME ft = {0};
    SystemTimeToFileTime(&st, &ft);
    
    ULARGE_INTEGER ftl;
    memcpy(&(ftl), &(ft), sizeof(ft));
    
    ULONGLONG currTime = ftl.QuadPart;
    
    ULONGLONG dt = currTime - prevTime;
    prevTime = currTime;
    
    return dt;
}

