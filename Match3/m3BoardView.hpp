#pragma once

#include <WICTextureLoader.h>
#include "SpriteRenderer.hpp"

#include "m3Board.hpp"

namespace m3
{
    class BoardView
    {
    private:
        SpriteRenderer m_SpriteRenderer;
        ComPtr<ID3D11DeviceContext> m_DeviceContext;
        ComPtr<ID3D11SamplerState> m_PixellySamplerState;
        ComPtr<ID3D11BlendState> m_TransparentSpriteBlendState;

        ComPtr<ID3D11Resource> m_GemSpriteTex2D;
        ComPtr<ID3D11ShaderResourceView> m_GemSpriteSRV;
        ComPtr<ID3D11Resource> m_TileSpriteTex2D;
        ComPtr<ID3D11ShaderResourceView> m_TileSpriteSRV;

    public:
        void Init(const Common::Direct3D11& d3d11, const std::string& shadersBasePath)
        {
            m_SpriteRenderer.Init(d3d11, shadersBasePath);
            m_DeviceContext = d3d11.GetDeviceContext();

            const auto d3dDevice = d3d11.GetDevice().Get();
            const auto d3dContext = m_DeviceContext.Get();

            DirectX::CreateWICTextureFromFile(d3dDevice, d3dContext, L"Sprites//gem_grey_square.png", 
                m_GemSpriteTex2D.GetAddressOf(), m_GemSpriteSRV.GetAddressOf());

            DirectX::CreateWICTextureFromFile(d3dDevice, d3dContext, L"Sprites//bg_tile.png", 
                m_TileSpriteTex2D.GetAddressOf(), m_TileSpriteSRV.GetAddressOf());

            m_PixellySamplerState = InitPixellySamplerState(d3dDevice);
            m_TransparentSpriteBlendState = InitTransparentSpriteBlendState(d3dDevice);
        }

        void BeginRender()
        {
            // Enable transparency.
            m_DeviceContext->OMSetBlendState(m_TransparentSpriteBlendState.Get(), nullptr, 0xffffffff);

            // "Pixel-Art" sampler.
            ID3D11SamplerState* samplerStates[] = { m_PixellySamplerState.Get() };
            UINT numSamplerStates = sizeof(samplerStates) / sizeof(ID3D11SamplerState*);
            m_DeviceContext->PSSetSamplers(0, numSamplerStates, samplerStates);

            ID3D11ShaderResourceView* shaderResourceViews[] = 
            { 
                m_GemSpriteSRV.Get(),
                m_TileSpriteSRV.Get()
            };

            UINT numSRVs = sizeof(shaderResourceViews) / sizeof(ID3D11ShaderResourceView*);
            m_DeviceContext->PSSetShaderResources(0, numSRVs, shaderResourceViews);   

            m_SpriteRenderer.Begin();
        }

        void RenderBackground(const int rows, const int cols)
        {
            const Vector2 scale = { 64.0f, 64.0f };
            const Vector2 origin = -0.5f * scale * Vector2(cols - 1, rows - 1);
            const float rotations[] = {
                TwoPi * 0.0f, 
                TwoPi * 0.25f, 
                TwoPi * 0.5f, 
                TwoPi * 0.0f
            };

            for (auto i = 0; i < rows * cols; i++)
            {
                auto r = i / cols;
                auto c = i % cols;
                auto position = origin + scale * Vector2(c, r);
                auto tint = Color(1, 1, 1, 1);

                m_SpriteRenderer.Draw(position, scale, rotations[i % 4], tint, 1);
            }
        }

        void RenderGems()
        {
            const Vector2 scale = { 64.0f, 64.0f };
            m_SpriteRenderer.Draw({ 0.0f, 0.0f }, scale, Color(1, 1, 1, 1), 0);
        }

        inline void EndRender()
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
}