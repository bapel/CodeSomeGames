#include "Direct3D11.hpp"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_image.h>
#include <SDL_rwops.h>

size_t ReadBinaryFile(const char* path, std::vector<char>& outBuffer);

void Common::Direct3D11::Init(SDL_Window* window)
{
    SDL_assert(nullptr != window);

    SDL_SysWMinfo info = {};
    SDL_GetVersion(&info.version);
    SDL_assert(SDL_GetWindowWMInfo(window, &info));
    SDL_GetWindowSize(window, &m_WindowWidth, &m_WindowHeight);

    HWND hwnd = info.info.win.window;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_12_0,
        D3D_FEATURE_LEVEL_12_1
    };

    const UINT numFeatureLevels = sizeof(featureLevels) / sizeof(D3D_FEATURE_LEVEL);

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = m_WindowWidth;
    swapChainDesc.BufferDesc.Height = m_WindowHeight;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 4;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

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
        m_SwapChain.GetAddressOf(),
        m_Device.GetAddressOf(),
        &m_FeatureLevel,
        m_DeviceContext.GetAddressOf()));

    ComPtr<ID3D11Texture2D> backBuffer;
    D3D_OK(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf()));
    D3D_OK(m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_BackBufferView.GetAddressOf()));
}

void Common::Direct3D11::OnWindowResized(int w, int h)
{
    m_DeviceContext->ClearState();
    m_BackBufferView->Release();
    // _depthStencilView->Release();

    D3D_OK(m_SwapChain->ResizeBuffers(2, w, h, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

    ComPtr<ID3D11Texture2D> backBuffer;
    D3D_OK(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf()));
    D3D_OK(m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_BackBufferView.GetAddressOf()));

    m_WindowWidth = w;
    m_WindowHeight = h;
}

void Common::Direct3D11::FrameStart(SDL_Window* window, Color clearColor)
{
    SDL_assert(nullptr != window);

    m_DeviceContext->ClearState();

    {
        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        if (w != m_WindowWidth || h != m_WindowHeight)
            OnWindowResized(w, h);
    }

    m_DeviceContext->ClearRenderTargetView(m_BackBufferView.Get(), clearColor);
    // _context->ClearDepthStencilView(_depthStencilView.Get(), 0, 1.0f, 0);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = (float)m_WindowWidth;
    viewport.Height = (float)m_WindowHeight;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_DeviceContext->RSSetViewports(1, &viewport);
    m_DeviceContext->OMSetRenderTargets(1, m_BackBufferView.GetAddressOf(), nullptr);
}

void Common::Direct3D11::FrameEnd()
{
    D3D_OK(m_SwapChain->Present(0, 0));
}

ComPtr<ID3D11VertexShader> Common::Direct3D11::CreateVertexShaderFromFile(const std::string& path, std::vector<char>& outByteCode) const
{
    std::vector<char>& buffer = outByteCode;
    SDL_assert(ReadBinaryFile(path.c_str(), buffer) > 0);

    ID3D11VertexShader* shader = nullptr;
    D3D_OK(m_Device->CreateVertexShader(buffer.data(), buffer.size(), nullptr, &shader));
    return shader;
}

ComPtr<ID3D11PixelShader> Common::Direct3D11::CreatePixelShaderFromFile(const std::string& path) const
{
    std::vector<char> buffer;
    SDL_assert(ReadBinaryFile(path.c_str(), buffer) > 0);

    ID3D11PixelShader* shader = nullptr;
    D3D_OK(m_Device->CreatePixelShader(buffer.data(), buffer.size(), nullptr, &shader));
    return shader;
}

ComPtr<ID3D11Texture2D> Common::Direct3D11::CreateTextureFromFile(const std::string& path, ID3D11ShaderResourceView** outView)
{
    auto sprite = IMG_Load(path.c_str());
    SDL_assert(nullptr != sprite);

    auto formatName = SDL_GetPixelFormatName(sprite->format->format);
    SDL_Log("%s: %s", path.c_str(), formatName);

    DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN;
    switch (sprite->format->format)
    {
    case SDL_PIXELFORMAT_ABGR8888:
        format = DXGI_FORMAT_R8G8B8A8_UNORM;
        break;
    }

    // Unsupported format.
    SDL_assert(format != DXGI_FORMAT_UNKNOWN);

    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width = sprite->w;
    desc.Height = sprite->h;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = format;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    desc.MiscFlags = 0;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = sprite->pixels;
    data.SysMemPitch = sprite->pitch;
    data.SysMemSlicePitch = sprite->w * sprite->h * 4;

    ID3D11Texture2D* texture = nullptr;
    D3D_OK(m_Device->CreateTexture2D(&desc, &data, &texture));
    SDL_FreeSurface(sprite);

    if (outView != nullptr)
        D3D_OK(m_Device->CreateShaderResourceView(texture, nullptr, outView));

    return texture;
}

size_t ReadBinaryFile(const char* path, std::vector<char>& outBuffer)
{
    SDL_RWops* file = SDL_RWFromFile(path, "rb");
    SDL_assert(nullptr != file);

    if (file != nullptr)
    {
        auto size = SDL_RWseek(file, 0, RW_SEEK_END);
        outBuffer.resize(size);
        SDL_RWseek(file, 0, RW_SEEK_SET);
        size_t readSize = SDL_RWread(file, outBuffer.data(), sizeof(char), size);
        SDL_RWclose(file);

        return readSize;
    }

    return 0;
}