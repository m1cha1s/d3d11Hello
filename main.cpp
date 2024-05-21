/*

This is according to this guide:
https://antongerdelan.net/opengl/d3d11.html

*/

#include <stdio.h>
#include <float.h>

#define WIN32_LEAN_AND_MEAN
// #define COBJMACROS
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <assert.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "types.h"

#define STDLIB_ALLOC_IMPLEMENTATION
#include "alloc.h"

#define STD_VS "vs_5_0"
#define STD_PS "ps_5_0"

#define MAX_SHADERS 256

// GLOBALS

// D3D11
ID3D11Device* devicePtr                     = nullptr;
ID3D11DeviceContext* deviceContextPtr       = nullptr;
IDXGISwapChain* swapChainPtr                = nullptr;
ID3D11RenderTargetView* renderTargetViewPtr = nullptr;

ID3D11VertexShader* vertexShaders[MAX_SHADERS] = {nullptr};
ID3D11InputLayout*  inputLayouts[MAX_SHADERS]  = {nullptr};
ShaderIdx vertexShaderCount                    = 0;

ID3D11PixelShader* pixelShaders[MAX_SHADERS] = {nullptr};
ShaderIdx pixelShaderCount                   = 0;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ULONGLONG getDeltaTime();

void InitGFX(HWND hwnd);

// Shader stuff.
ID3DBlob* CompileShader(wchar_t* path, char* mainFn, char* std);
ShaderIdx LoadVertexShader(wchar_t* path, char* mainFn, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, u64 inputElementCount);
ShaderIdx LoadPixelShader(wchar_t* path, char* mainFn);

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
    
    InitGFX(hwnd);
    
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "UV", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    
    ShaderIdx vsIdx = LoadVertexShader(L"shaders.hlsl", "vsMain", inputElementDesc, ARRAYSIZE(inputElementDesc));
    ShaderIdx psIdx = LoadPixelShader(L"shaders.hlsl", "psMain");
    
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
    HRESULT hr = devicePtr->CreateBuffer(
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
    
    hr = devicePtr->CreateTexture2D(
        &textureDesc,
        &srTex,
        &texture
    );
    assert(SUCCEEDED(hr));
    
    stbi_image_free(bitmap);
    
    ID3D11ShaderResourceView *textureShaderResViewPtr = NULL;
    
    hr = devicePtr->CreateShaderResourceView(
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
    
    devicePtr->CreateSamplerState(
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
        deviceContextPtr->Map(
            (ID3D11Resource *)vertexBufferPtr,
            0,
            D3D11_MAP_WRITE_DISCARD,
            0,
            &resource
        );
        memcpy(resource.pData, vertexDataArray, sizeof(vertexDataArray));
        deviceContextPtr->Unmap(
            (ID3D11Resource *)vertexBufferPtr,
            0
        );
        
        // Clear background
        float backgroundColor[4] = {
            0x64 / 255.0f, 0x95 / 255.0f, 0xED / 255.0f, 1.0f
        };
        deviceContextPtr->ClearRenderTargetView(
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
        deviceContextPtr->RSSetViewports(1,&viewport);
        
        // Set Output Merger
        deviceContextPtr->OMSetRenderTargets(1, &renderTargetViewPtr, NULL);
        
        // Set Input Assembler
        deviceContextPtr->IASetPrimitiveTopology(
            D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST
        );
        deviceContextPtr->IASetInputLayout(
            inputLayouts[vsIdx]
        );
        
        deviceContextPtr->PSSetShaderResources(
            0, 1,
            &textureShaderResViewPtr
        );
        deviceContextPtr->PSSetSamplers(
            0, 1,
            &samplerStatePtr
        );
        
        deviceContextPtr->IASetVertexBuffers(
            0,
            1,
            &vertexBufferPtr,
            &vertexStride,
            &vertexOffset
        );
        
        // Set Shaders
        deviceContextPtr->VSSetShader(
            vertexShaders[vsIdx],
            NULL,
            0
        );
        deviceContextPtr->PSSetShader(
            pixelShaders[psIdx],
            NULL,
            0
        );
        
        // Finally draw the triangle
        deviceContextPtr->Draw(
            vertexCount,
            0
        );
        
        // Swap the Buffers(present the frame)
        swapChainPtr->Present(
            1,
            0
        );
        
        ULONGLONG dt = getDeltaTime()/10000;
        printf("%I64u ms\n", dt); // TODO: Print it on screen. NOT in the console.
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

void InitGFX(HWND hwnd) {
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
    hr = swapChainPtr->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        (void**)&framebuffer
    );
    assert(SUCCEEDED(hr));
    
    hr = devicePtr->CreateRenderTargetView(
        (ID3D11Resource*)framebuffer,
        0,
        &renderTargetViewPtr
    );
    assert(SUCCEEDED(hr));
}

ID3DBlob* CompileShader(wchar_t* path, char* mainFn, char* std) {
    UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
    #if defined( DEBUG ) || defined( _DEBUG )
        flags |= D3DCOMPILE_DEBUG;
    #endif
    ID3DBlob *blobPtr = NULL, *errorBlobPtr = NULL;

    HRESULT hr = D3DCompileFromFile(
        path,
        NULL,
        NULL,
        //D3D_COMPILE_STANDARD_FILE_INCLUDE,
        mainFn,
        std,
        flags,
        0,
        &blobPtr,
        &errorBlobPtr
    );
    if (FAILED(hr)) {
        if (errorBlobPtr) {
            OutputDebugStringA((char*)errorBlobPtr->GetBufferPointer());
            errorBlobPtr->Release();
        }
        if (blobPtr) blobPtr->Release();
        assert(false);
    }
    if (errorBlobPtr) errorBlobPtr->Release();
    
    return blobPtr;
}

ShaderIdx LoadVertexShader(wchar_t* path, char* mainFn, D3D11_INPUT_ELEMENT_DESC* inputElementDesc, u64 inputElementCount) {
    if (vertexShaderCount>=MAX_SHADERS) assert("Too many shaders!!!" && false);
        
    ID3DBlob* vsBlobPtr = CompileShader(path, mainFn, STD_VS);
    
    HRESULT hr = devicePtr->CreateVertexShader(
        vsBlobPtr->GetBufferPointer(),
        vsBlobPtr->GetBufferSize(),
        NULL,
        &vertexShaders[vertexShaderCount]
    );
    assert("Failed to load Vertex Shader!!!" && SUCCEEDED(hr));
    
    hr = devicePtr->CreateInputLayout(
        inputElementDesc,
        inputElementCount,
        vsBlobPtr->GetBufferPointer(),
        vsBlobPtr->GetBufferSize(),
        &inputLayouts[vertexShaderCount++]
    );
    assert("Failed to create Input Layout!!!" && SUCCEEDED(hr));
    
    return vertexShaderCount-1;
}

ShaderIdx LoadPixelShader(wchar_t* path, char* mainFn) {
    if (pixelShaderCount>=MAX_SHADERS) assert("Too many shaders!!!" && false);
    
    ID3DBlob* psBlobPtr = CompileShader(path, mainFn, STD_PS);
    
     HRESULT hr = devicePtr->CreatePixelShader(
        psBlobPtr->GetBufferPointer(),
        psBlobPtr->GetBufferSize(),
        NULL,
        &pixelShaders[pixelShaderCount++]
    );
    assert("Failed to create Pixel Shader" && SUCCEEDED(hr));
    
    return pixelShaderCount-1;
}