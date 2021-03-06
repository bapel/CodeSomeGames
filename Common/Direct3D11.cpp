#include "Direct3D11.hpp"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxguid.lib")

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_rwops.h>

size_t ReadBinaryFile(const char* path, std::vector<char>& outBuffer);

Common::Direct3D11::~Direct3D11()
{
    if (m_Debug)
        m_Debug->ReportLiveDeviceObjects(D3D11_RLDO_IGNORE_INTERNAL);
}

void Common::Direct3D11::Init(SDL_Window* window, bool debug)
{
    SDL_assert(nullptr != window);

    m_Window = window;

    SDL_SysWMinfo info = {};
    SDL_GetVersion(&info.version);
    SDL_GetWindowWMInfo(window, &info);
    SDL_GetWindowSize(window, &m_WindowWidth, &m_WindowHeight);

    HWND hwnd = info.info.win.window;

    D3D_FEATURE_LEVEL featureLevels[] = 
    {
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
    if (debug)
        deviceCreateFlags = D3D11_CREATE_DEVICE_DEBUG;

    Direct3D_Ok__(D3D11CreateDeviceAndSwapChain(
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

    if (debug)
        Direct3D_Ok__(m_Device->QueryInterface(IID_PPV_ARGS(m_Debug.GetAddressOf())));

    ComPtr<ID3D11Texture2D> backBuffer;
    Direct3D_Ok__(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf()));
    Direct3D_Ok__(m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_BackBufferView.GetAddressOf()));

    SetDebugName(backBuffer.Get(), "SwapChain BackBuffer Texture");
    SetDebugName(m_BackBufferView.Get(), "SwapChain BackBuffer RTV");
}

void Common::Direct3D11::SetDebugName(ID3D11DeviceChild* child, const std::string& name) const
{
    if (m_Debug && (name.size() > 0))
        child->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)name.size(), name.c_str());
}

void Common::Direct3D11::OnWindowResized(int w, int h)
{
    m_DeviceContext->ClearState();
    m_BackBufferView->Release();
    // _depthStencilView->Release();

    Direct3D_Ok__(m_SwapChain->ResizeBuffers(2, w, h, DXGI_FORMAT_R8G8B8A8_UNORM, 0));

    ComPtr<ID3D11Texture2D> backBuffer;
    Direct3D_Ok__(m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)backBuffer.GetAddressOf()));
    Direct3D_Ok__(m_Device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_BackBufferView.GetAddressOf()));

    m_WindowWidth = w;
    m_WindowHeight = h;
}

void Common::Direct3D11::BeginFrame()
{
    int w, h;
    SDL_GetWindowSize(m_Window, &w, &h);
    if (w != m_WindowWidth || h != m_WindowHeight)
        OnWindowResized(w, h);
}

// @Todo. Except a render target view.
// @Todo. Except and use a depth stencil view.
void Common::Direct3D11::BeginRender()
{
    m_DeviceContext->OMSetRenderTargets(1, m_BackBufferView.GetAddressOf(), nullptr);

    D3D11_VIEWPORT viewport = {};
    viewport.Width = (float)m_WindowWidth;
    viewport.Height = (float)m_WindowHeight;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    m_DeviceContext->RSSetViewports(1, &viewport);
}

void Common::Direct3D11::ClearBackBuffer(Color clearColor)
{
    m_DeviceContext->ClearRenderTargetView(m_BackBufferView.Get(), clearColor);
}

// @Todo. deoth buffer clear.
// _context->ClearDepthStencilView(_depthStencilView.Get(), 0, 1.0f, 0);

void Common::Direct3D11::EndFrame()
{
    Direct3D_Ok__(m_SwapChain->Present(0, 0));
    m_DeviceContext->ClearState();
    m_DeviceContext->Flush();
}

ComPtr<ID3D11VertexShader> Common::Direct3D11::CreateVertexShaderFromFile(const std::string& path, std::vector<char>& outByteCode) const
{
    std::vector<char>& buffer = outByteCode;
    auto bytes = ReadBinaryFile(path.c_str(), buffer);
    
    if (bytes == 0)
    {
        SDL_Log("Failed to load Vertex Shader: %s", path);
        return nullptr;
    }

    ComPtr<ID3D11VertexShader> shader = nullptr;
    Direct3D_Ok__(m_Device->CreateVertexShader(buffer.data(), buffer.size(), nullptr, &shader));
    SetDebugName(shader, path);
    return shader;
}

ComPtr<ID3D11PixelShader> Common::Direct3D11::CreatePixelShaderFromFile(const std::string& path) const
{
    std::vector<char> buffer;
    auto bytes = ReadBinaryFile(path.c_str(), buffer);

    if (bytes == 0)
    {
        SDL_Log("Failed to load Pixel Shader: path");
        return nullptr;
    }

    ComPtr<ID3D11PixelShader> shader = nullptr;
    Direct3D_Ok__(m_Device->CreatePixelShader(buffer.data(), buffer.size(), nullptr, &shader));
    SetDebugName(shader, path);
    return shader;
}

ComPtr<ID3D11Texture2D> Common::Direct3D11::CreateTextureFromFile(const std::string& path) const
{
    // @Todo: Implement.
    return nullptr;
}

ComPtr<ID3D11ShaderResourceView> Common::Direct3D11::CreateShaderResourceView(const ComPtr<ID3D11Texture2D> texture) const
{
    ComPtr<ID3D11ShaderResourceView> view = nullptr;
    Direct3D_Ok__(m_Device->CreateShaderResourceView(texture.Get(), nullptr, view.GetAddressOf()));
    return view;
}

ComPtr<ID3D11Buffer> Common::Direct3D11::CreateConstantsBuffer(size_t structSize) const
{
    D3D11_BUFFER_DESC constantsBufferDesc = {};
    constantsBufferDesc.ByteWidth = (UINT)structSize;
    constantsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ComPtr<ID3D11Buffer> constantsBuffer;
    Direct3D_Ok__(m_Device->CreateBuffer(&constantsBufferDesc, nullptr, constantsBuffer.GetAddressOf()));
    return constantsBuffer;
}

void Common::Direct3D11::UpdateBufferData(const ComPtr<ID3D11Buffer>& buffer, const void* data, size_t size) const
{
    D3D11_MAPPED_SUBRESOURCE mappedConstantsBuffer;
    m_DeviceContext->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedConstantsBuffer);
    memcpy(mappedConstantsBuffer.pData, data, size);
    m_DeviceContext->Unmap(buffer.Get(), 0);
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