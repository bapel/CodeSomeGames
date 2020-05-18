#pragma once

#include "ComPtr.hpp"
#include "VectorMath.hpp"

#include <dxgi.h>
#include <d3d11.h>
#include <vector>
#include <string>

// @Todo. Make not dependent on SDL_assert?
#ifdef _DEBUG
#include <SDL_assert.h>
#define D3D_OK(__call__)\
    SDL_assert(S_OK == (__call__))
#else
#define D3D_OK(__call__) __call__
#endif

struct SDL_Window;

namespace Common {

    class Direct3D11
    {
    private:
        ComPtr<ID3D11Device> m_Device;
        ComPtr<ID3D11Debug> m_Debug;
        ComPtr<ID3D11DeviceContext> m_DeviceContext;

        ComPtr<IDXGISwapChain> m_SwapChain;
        ComPtr<ID3D11RenderTargetView> m_BackBufferView;
        ComPtr<ID3D11DepthStencilView> m_DepthStencilView;
        
        D3D_FEATURE_LEVEL m_FeatureLevel = D3D_FEATURE_LEVEL_1_0_CORE;

        int m_WindowWidth = 0;
        int m_WindowHeight = 0;

    public:
        ~Direct3D11();

        void Init(SDL_Window* window, bool debug = true);
        void OnWindowResized(int w, int h);
        void FrameStart(SDL_Window* window, Color clearColor);
        void FrameEnd();

        inline const ComPtr<IDXGISwapChain>& GetSwapchain() const { return m_SwapChain; }
        inline const ComPtr<ID3D11Device>& GetDevice() const { return m_Device; }
        inline const ComPtr<ID3D11DeviceContext>& GetDeviceContext() const { return m_DeviceContext; }

        ComPtr<ID3D11VertexShader> CreateVertexShaderFromFile(const std::string& path, std::vector<char>& outByteCode) const;
        ComPtr<ID3D11PixelShader> CreatePixelShaderFromFile(const std::string& path) const;

        ComPtr<ID3D11Texture2D> CreateTextureFromFile(const std::string& path, ID3D11ShaderResourceView** outView) const;

        ComPtr<ID3D11Buffer> CreateConstantsBuffer(size_t structSize) const;
        void UpdateBufferData(const ComPtr<ID3D11Buffer>& buffer, const void* data, size_t size) const;

        template <class StructType> inline ComPtr<ID3D11Buffer> CreateConstantsBuffer() const;
        template <class StructType> inline void UpdateBufferData(const ComPtr<ID3D11Buffer>& buffer, const StructType& data);
    };

    template <class StructType> 
    inline ComPtr<ID3D11Buffer> Direct3D11::CreateConstantsBuffer() const
    {
        return CreateConstantsBuffer(sizeof(StructType));
    }

    template <class StructType> 
    inline void Direct3D11::UpdateBufferData(const ComPtr<ID3D11Buffer>& buffer, const StructType& data)
    {
        UpdateBufferData(buffer, &data, sizeof(StructType));
    }
}