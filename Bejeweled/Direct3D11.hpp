#include <dxgi.h>
#include <d3d11.h>
#include <wrl\client.h>
#include <SDL_assert.h>
#include <string>
#include <vector>

#define D3D_OK(__call__)\
    SDL_assert(S_OK == (__call__))

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

struct SDL_Window;

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
    void FrameStart(SDL_Window* window);
    void FrameEnd();

    IDXGISwapChain* GetSwapchain() const { return _swapChain.Get(); }
    ID3D11Device* GetDevice() const { return _device.Get(); }
    ID3D11DeviceContext* GetDeviceContext() const { return _context.Get(); }

    ComPtr<ID3D11VertexShader> CreateVertexShaderFromFile(const std::string& path, std::vector<char>& outByteCode);
    ComPtr<ID3D11PixelShader> CreatePixelShaderFromFile(const std::string& path);

    ComPtr<ID3D11Texture2D> CreateTextureFromFile(const std::string& path, ID3D11ShaderResourceView** outView);
};