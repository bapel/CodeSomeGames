#include <dxgi.h>
#include <d3d11.h>
#include <wrl\client.h>

template <class T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

struct SDL_Window;

class Direct3D11
{
private:
    ComPtr<IDXGISwapChain> _swapchain;
    ComPtr<ID3D11Device> _device;
    D3D_FEATURE_LEVEL _featureLevel;
    ComPtr<ID3D11DeviceContext> _context;

public:
    bool Init(SDL_Window* window);

    IDXGISwapChain* GetSwapchain() const { return _swapchain.Get(); }
    ID3D11Device* GetDevice() const { return _device.Get(); }
    ID3D11DeviceContext* GetDeviceContext() const { return _context.Get(); }
};