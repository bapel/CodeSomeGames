#include "Direct3D11.hpp"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include <SDL.h>
#include <SDL_syswm.h>

void Direct3D11::Init(SDL_Window* window)
{
    SDL_assert(nullptr != window);

    SDL_SysWMinfo info = {};
    SDL_GetVersion(&info.version);
    SDL_assert(SDL_GetWindowWMInfo(window, &info));

    HWND hwnd = info.info.win.window;

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_12_1
    };

    const UINT numFeatureLevels = sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL);

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = windowWidth;
    swapChainDesc.BufferDesc.Height = windowHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

    UINT deviceCreateFlags = 0;
#if _DEBUG
    deviceCreateFlags = D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_OK(D3D11CreateDeviceAndSwapChain(
        0, // Default adapter
        D3D_DRIVER_TYPE_HARDWARE,
        0,
        deviceCreateFlags,
        featureLevels,
        numFeatureLevels,
        D3D11_SDK_VERSION,
        &swapChainDesc,
        _swapChain.GetAddressOf(),
        _device.GetAddressOf(),
        &_featureLevel,
        _context.GetAddressOf()));

    ComPtr<ID3D11Texture2D> backBuffer;
    D3D_OK(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf()));
    D3D_OK(_device->CreateRenderTargetView(backBuffer.Get(), nullptr, _backBufferView.GetAddressOf()));
}

void Direct3D11::FrameStart(SDL_Window* window)
{
    SDL_assert(nullptr != window);

    int windowWidth, windowHeight;
    SDL_GetWindowSize(window, &windowWidth, &windowHeight);

    FLOAT clearColor[] = { 1.0f, 0.75f, 0.5f, 1.0f };
    _context->ClearRenderTargetView(_backBufferView.Get(), clearColor);
    // _context->ClearDepthStencilView(_depthStencilView.Get(), 0, 1.0f, 0);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = windowWidth;
    viewport.Height = windowHeight;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    _context->RSSetViewports(1, &viewport);
    _context->OMSetRenderTargets(1, _backBufferView.GetAddressOf(), nullptr);
}

void Direct3D11::FrameEnd()
{
    D3D_OK(_swapChain->Present(0, 0));
}