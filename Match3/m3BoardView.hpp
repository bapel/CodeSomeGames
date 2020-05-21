#pragma once

#include <WICTextureLoader.h>
#include "SpriteRenderer.hpp"

#include "m3Board.hpp"

namespace m3
{
    class BoardView
    {
    private:
        const float m_SpriteSize;

        SpriteRenderer m_SpriteRenderer;
        ComPtr<ID3D11DeviceContext> m_DeviceContext;
        ComPtr<ID3D11SamplerState> m_PixellySamplerState;
        ComPtr<ID3D11BlendState> m_TransparentSpriteBlendState;

        ComPtr<ID3D11Resource> m_GemSpriteTex2D;
        ComPtr<ID3D11ShaderResourceView> m_GemSpriteSRV;
        ComPtr<ID3D11Resource> m_TileSpriteTex2D;
        ComPtr<ID3D11ShaderResourceView> m_TileSpriteSRV;

    public:
        BoardView(float spriteSize) : m_SpriteSize(spriteSize) {}

        void Init(const Common::Direct3D11& d3d11, const std::string& shadersBasePath)
        {
            m_SpriteRenderer.Init(d3d11, shadersBasePath);
            m_DeviceContext = d3d11.GetDeviceContext();

            const auto d3dDevice = d3d11.GetDevice().Get();
            const auto d3dContext = m_DeviceContext.Get();

            DirectX::CreateWICTextureFromFile(d3dDevice, d3dContext, L"Sprites//gem_sprite.png", 
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

        void RenderBackground(int rows, int cols, float spriteScale)
        {
            const Vector2 scale = { spriteScale, spriteScale };
            const Vector2 origin = -0.5f * scale * Vector2((float)cols - 1, (float)rows - 1);
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
                auto position = origin + scale * Vector2((float)c, (float)r);
                auto tint = Color(1, 1, 1, 1);

                m_SpriteRenderer.Draw(position, scale, rotations[i % 4], tint, 1);
            }
        }

        inline Color ToColor(m3::gem_color_t col)
        {
            switch (col)
            {
                case 'B': return Color(0, 1, 0, 1); break;
                case 'R': return Color(1, 0, 0, 1); break;
                case 'O': return Color(1, 0.5f, 0, 1); break;
                case 'G': return Color(0, 0, 1, 1); break;
                case 'Y': return Color(1, 1, 0, 1); break;
            }

            return Color(0, 0, 0, 0);
        }

        template <class T>
        void RenderGems(const T& data, int rows, int cols)
        {
            for (auto i = 0; i < data.Count(); i++)
            {
                auto position = data.GetPosition(i);
                auto scale = Vector2 { data.GetScale(i), data.GetScale(i) };
                auto tint = ToColor(data.GetColor(i));

                m_SpriteRenderer.Draw(position, scale, tint, 0);
            }
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