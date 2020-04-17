#include <dxgi.h>
#include <d3d11.h>
#include <wrl\client.h>

#include <SDL_assert.h>

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
    D3D_FEATURE_LEVEL _featureLevel;
    ComPtr<ID3D11DeviceContext> _context;
    ComPtr<ID3D11RenderTargetView> _backBufferView;
    ComPtr<ID3D11DepthStencilView> _depthStencilView;

public:
    void Init(SDL_Window* window);
    void FrameStart(SDL_Window* window);
    void FrameEnd();

    IDXGISwapChain* GetSwapchain() const { return _swapChain.Get(); }
    ID3D11Device* GetDevice() const { return _device.Get(); }
    ID3D11DeviceContext* GetDeviceContext() const { return _context.Get(); }
};