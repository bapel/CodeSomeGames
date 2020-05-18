#pragma once

#include "SpriteRenderer.hpp"

class Match3BoardView
{
private:
    SpriteRenderer m_SpriteRenderer;
    ComPtr<ID3D11DeviceContext> m_DeviceContext;
    ComPtr<ID3D11ShaderResourceView> m_SpriteResourceView;
    ComPtr<ID3D11Texture2D> m_SpriteTexture;
    ComPtr<ID3D11SamplerState> m_PixellySamplerState;
    ComPtr<ID3D11BlendState> m_TransparentSpriteBlendState;
    
public:
    void Init(const Common::Direct3D11& d3d11, const std::string& shadersBasePath)
    {
        m_SpriteRenderer.Init(d3d11, shadersBasePath);
        m_DeviceContext = d3d11.GetDeviceContext();

        auto spritePath = "Sprites//gem_grey_square.png";
        m_SpriteTexture = d3d11.CreateTextureFromFile(spritePath);
        m_SpriteResourceView = d3d11.CreateShaderResourceView(m_SpriteTexture);
        d3d11.SetDebugName(m_SpriteResourceView, spritePath);

        const auto d3dDevice = d3d11.GetDevice().Get();

        m_PixellySamplerState = InitPixellySamplerState(d3dDevice);
        m_TransparentSpriteBlendState = InitTransparentSpriteBlendState(d3dDevice);
    }

    void Begin()
    {
        m_DeviceContext->OMSetBlendState(m_TransparentSpriteBlendState.Get(), nullptr, 0xffffffff);

        m_SpriteRenderer.Begin();

        ID3D11ShaderResourceView* shaderResourceViews[] = { m_SpriteResourceView.Get() };
        UINT numShaderResourceViews = sizeof(shaderResourceViews) / sizeof(ID3D11ShaderResourceView*);
        m_DeviceContext->PSSetShaderResources(0, numShaderResourceViews, shaderResourceViews);

        ID3D11SamplerState* samplerStates[] = { m_PixellySamplerState.Get() };
        UINT numSamplerStates = sizeof(samplerStates) / sizeof(ID3D11SamplerState*);
        m_DeviceContext->PSSetSamplers(0, numSamplerStates, samplerStates);

        m_SpriteRenderer.SetSampler(m_PixellySamplerState);
    }

    inline void RenderGem(Vector2 position, Vector2 size, Color tint)
    {
        m_SpriteRenderer.Draw(position, size, tint);
    }

    void End()
    {
        m_SpriteRenderer.End();
    }

private:
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
};