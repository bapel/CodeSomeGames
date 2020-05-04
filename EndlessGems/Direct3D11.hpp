#pragma once

#include <dxgi.h>
#include <d3d11.h>
#include <wrl\client.h>
#include <SDL_assert.h>
#include <string>
#include <vector>
#include <array>

#define D3D_OK(__call__)\
    SDL_assert(S_OK == (__call__))

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

struct SDL_Window;

union Color
{
    uint32_t Value;
    struct { uint8_t R, G, B, A; };

    Color() : Value(0x000000ff) {}

    Color(uint32_t hex) 
    {
        R = (hex & 0xff000000) >> 24;
        G = (hex & 0x00ff0000) >> 16;
        B = (hex & 0x0000ff00) >>  8;
        A = (hex & 0x000000ff);
    }

    Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : R(r), G(g), B(b), A(a) {}

    std::array<float, 4> ToFloats() const 
    {
        return
        {
            (float)R / 255.0f,
            (float)G / 255.0f,
            (float)B / 255.0f,
            (float)A / 255.0f
        };
    }
};

class Direct3D11
{
private:
    ComPtr<IDXGISwapChain> _swapChain;
    ComPtr<ID3D11Device> _device;
    D3D_FEATURE_LEVEL _featureLevel = D3D_FEATURE_LEVEL_1_0_CORE;
    ComPtr<ID3D11DeviceContext> _context;
    ComPtr<ID3D11RenderTargetView> _backBufferView;
    ComPtr<ID3D11DepthStencilView> _depthStencilView;

    int _windowWidth = 0;
    int _windowHeight = 0;

public:
    void Init(SDL_Window* window);
    void OnWindowResized(int w, int h);
    void FrameStart(SDL_Window* window, Color clearColor);
    void FrameEnd();

    IDXGISwapChain* GetSwapchain() const { return _swapChain.Get(); }
    ID3D11Device* GetDevice() const { return _device.Get(); }
    ID3D11DeviceContext* GetDeviceContext() const { return _context.Get(); }

    ComPtr<ID3D11VertexShader> CreateVertexShaderFromFile(const std::string& path, std::vector<char>& outByteCode);
    ComPtr<ID3D11PixelShader> CreatePixelShaderFromFile(const std::string& path);

    ComPtr<ID3D11Texture2D> CreateTextureFromFile(const std::string& path, ID3D11ShaderResourceView** outView);
};