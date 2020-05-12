#pragma once

#include <Direct3D11.hpp>
#include <SDL_assert.h>

ComPtr<ID3D11SamplerState> InitSamplerState(ID3D11Device* d3dDevice)
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

ComPtr<ID3D11RasterizerState> InitRasterizerState(ID3D11Device* d3dDevice)
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

ComPtr<ID3D11BlendState> InitBlendState(ID3D11Device* d3dDevice)
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
