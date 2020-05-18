#include "SDLGame.hpp"
#include "SpriteRenderer.hpp"

struct CameraConstantsBuffer
{
    Matrix model;
    Matrix viewProj;
};

class Match3Game final : public SDLGame
{
    SpriteRenderer m_SpriteRenderer;
    ComPtr<ID3D11Buffer> m_CameraConstantsBuffer;
    ComPtr<ID3D11ShaderResourceView> m_SpriteResourceView;
    ComPtr<ID3D11Texture2D> m_SpriteTexture;
    ComPtr<ID3D11SamplerState> m_PixellySamplerState;
    ComPtr<ID3D11BlendState> m_TransparentSpriteBlendState;
    ComPtr<ID3D11RasterizerState> m_DefaultRasterizerState;
    
    void OnCreate() 
    {
        // Load resources.

        m_SpriteRenderer.Init(m_D3D11, m_ShadersPath);

        m_CameraConstantsBuffer = m_D3D11.CreateConstantsBuffer<CameraConstantsBuffer>();
        m_D3D11.SetDebugName(m_CameraConstantsBuffer.Get(), "CameraConstantsBuffer");

        auto spritePath = "Sprites//gem_grey_square.png";
        m_SpriteTexture = m_D3D11.CreateTextureFromFile(spritePath, m_SpriteResourceView);

        const auto d3dDevice = m_D3D11.GetDevice().Get();

        m_PixellySamplerState = InitPixellySamplerState(d3dDevice);
        m_TransparentSpriteBlendState = InitTransparentSpriteBlendState(d3dDevice);
        m_DefaultRasterizerState = InitDefaultRasterizerState(d3dDevice);

        // Init game.
        // @Todo.
    }

    void OnUpdate(double dtSeconds) { }

    void OnRender(int viewportWidth, int viewportHeight)
    {
        CameraConstantsBuffer cameraConstantBufferData =
        {
            Matrix::CreateRotationX(0), // @Todo. Matrix::Identity is undefined???
            Matrix::CreateOrthographic(viewportWidth, viewportHeight, 0.0f, 1.0f)
        };

        m_D3D11.UpdateBufferData(m_CameraConstantsBuffer.Get(), &cameraConstantBufferData);

        const auto d3dContext = m_D3D11.GetDeviceContext();

        m_D3D11.FrameStart(m_Window, Color(0x2a2b3eff));

        d3dContext->OMSetBlendState(m_TransparentSpriteBlendState.Get(), nullptr, 0xffffffff);
        d3dContext->RSSetState(m_DefaultRasterizerState.Get());

        m_SpriteRenderer.Begin();

        ID3D11Buffer* constantsBuffers[] = { m_CameraConstantsBuffer.Get() };
        d3dContext->VSSetConstantBuffers(0, 1, constantsBuffers);

        ID3D11ShaderResourceView* shaderResourceViews[] = { m_SpriteResourceView.Get() };
        UINT numShaderResourceViews = sizeof(shaderResourceViews) / sizeof(ID3D11ShaderResourceView*);
        d3dContext->PSSetShaderResources(0, numShaderResourceViews, shaderResourceViews);

        ID3D11SamplerState* samplerStates[] = { m_PixellySamplerState.Get() };
        UINT numSamplerStates = sizeof(samplerStates) / sizeof(ID3D11SamplerState*);
        d3dContext->PSSetSamplers(0, numSamplerStates, samplerStates);

        m_SpriteRenderer.SetSampler(m_PixellySamplerState);

        m_SpriteRenderer.End();
        m_D3D11.FrameEnd();
    }

    void OnDestroy() 
    { }

    ComPtr<ID3D11SamplerState> InitPixellySamplerState(ID3D11Device* d3dDevice)
    {
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
        samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

        ComPtr<ID3D11SamplerState> samplerState = nullptr;
        D3D_OK(d3dDevice->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf()));
        return samplerState;
    }

    ComPtr<ID3D11BlendState> InitTransparentSpriteBlendState(ID3D11Device* d3dDevice)
    {
        D3D11_BLEND_DESC blendStateDesc = {};
        blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
        blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

        ComPtr<ID3D11BlendState> blendState = nullptr;
        D3D_OK(d3dDevice->CreateBlendState(&blendStateDesc, blendState.GetAddressOf()));
        return blendState;
    }

    ComPtr<ID3D11RasterizerState> InitDefaultRasterizerState(ID3D11Device* d3dDevice)
    {
        D3D11_RASTERIZER_DESC rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_BACK;
        rasterizerDesc.FrontCounterClockwise = FALSE;
        rasterizerDesc.DepthBias = D3D11_DEFAULT_DEPTH_BIAS;
        rasterizerDesc.DepthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP;
        rasterizerDesc.SlopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        rasterizerDesc.DepthClipEnable = true;
        rasterizerDesc.ScissorEnable = false;
        rasterizerDesc.MultisampleEnable = true;
        rasterizerDesc.AntialiasedLineEnable = true;

        ComPtr<ID3D11RasterizerState> rasterizerState = nullptr;
        D3D_OK(d3dDevice->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf()));
        return rasterizerState;
    }
};

RunGame(Match3Game)