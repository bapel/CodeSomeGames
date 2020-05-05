#include <SDL.h>
#include <SDL_assert.h>
#include <SDL_log.h>
#include <SDL_timer.h>
#include <SDL_image.h>

#include "Direct3D11.hpp"
#include <DirectXMath.h>
using namespace DirectX;

#include <stdio.h>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <queue>

#include "GemsBoardView.hpp"

struct QuadObject
{
    float x, y, z, angle;
    Color color;
};

struct TriangleVertex
{
    float x, y, z, w;
    float u, v;
};

struct TransformsBufferData
{
    XMMATRIX model;
    XMMATRIX viewProj;
};

int main(int argc, char** argv)
{
    SDL_assert(argc >= 1);
    const std::string kExePath(argv[0]);
    size_t offset = kExePath.find_last_of("\\");
    const std::string kShadersBasePath = kExePath.substr(0, offset + 1);

    SDL_assert(0 == SDL_Init(0));

    auto flags = IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    SDL_assert(IMG_INIT_JPG == (flags & IMG_INIT_JPG));
    SDL_assert(IMG_INIT_PNG == (flags & IMG_INIT_PNG));

    SDL_Window* window = SDL_CreateWindow(
        "Bejeweled Clone",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 768,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    SDL_assert(nullptr != window);

    const auto kNumQuads = 256;
    const auto kQuadSize = 2 * 32.0f;

    GemsBoardState boardState;
    GemsBoardState boardState_1;

    std::vector<QuadObject> quads;
    quads.reserve(kNumQuads);
    
    //std::random_device randomDevice;
    //std::mt19937 generator(randomDevice());
    std::mt19937 generator(0);
    std::uniform_int_distribution<> gemColorDistribution(1, (int)GemColor::Count - 1);

    for (int r = 0; r < boardState.Rows; r++)
    {
        for (int c = 0; c < boardState.Cols; c++)
        {
            auto color = gemColorDistribution(generator);
            boardState[{ r, c }] = (GemColor)color;
        }
    }

    Direct3D11 d3d11;
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11Buffer> vertexBuffer;
    ComPtr<ID3D11Buffer> instanceBuffer;
    ComPtr<ID3D11Buffer> transformsBuffer;

    d3d11.Init(window);

    std::vector<char> vsByteCode;
    vertexShader = d3d11.CreateVertexShaderFromFile(kShadersBasePath + "TriangleVertex.cso", vsByteCode);
    pixelShader = d3d11.CreatePixelShaderFromFile(kShadersBasePath + "TrianglePixel.cso");

    const auto d3dDevice = d3d11.GetDevice();

    D3D11_INPUT_ELEMENT_DESC elementDescs[] = 
    {
        { "POSITION",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0,                 D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,       0, 4 * sizeof(float), D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "QUAD_DATA", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0,                 D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "COLOR",     0, DXGI_FORMAT_R8G8B8A8_UNORM,     1, 4 * sizeof(float), D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    };

    const auto numElements = sizeof(elementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC);

    D3D_OK(d3dDevice->CreateInputLayout(elementDescs, numElements, vsByteCode.data(), vsByteCode.size(), inputLayout.GetAddressOf()));

    TriangleVertex vertices[] = 
    {
        { -0.5f * kQuadSize, -0.5f * kQuadSize, 0.0f, 1.0f, 0.0f, 0.0f },
        { -0.5f * kQuadSize,  0.5f * kQuadSize, 0.0f, 1.0f, 0.0f, 1.0f },
        {  0.5f * kQuadSize,  0.5f * kQuadSize, 0.0f, 1.0f, 1.0f, 1.0f },
        {  0.5f * kQuadSize,  0.5f * kQuadSize, 0.0f, 1.0f, 1.0f, 1.0f },
        {  0.5f * kQuadSize, -0.5f * kQuadSize, 0.0f, 1.0f, 1.0f, 0.0f },
        { -0.5f * kQuadSize, -0.5f * kQuadSize, 0.0f, 1.0f, 0.0f, 0.0f }
    };

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = (UINT)sizeof(vertices);
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = vertices;

    D3D_OK(d3dDevice->CreateBuffer(&vertexBufferDesc, &initialData, vertexBuffer.GetAddressOf()));

    D3D11_BUFFER_DESC instanceBufferDesc = {};
    instanceBufferDesc.ByteWidth = (UINT)(sizeof(QuadObject) * 64);
    instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D_OK(d3dDevice->CreateBuffer(&instanceBufferDesc, nullptr, instanceBuffer.GetAddressOf()));

    D3D11_BUFFER_DESC constantsBufferDesc = {};
    constantsBufferDesc.ByteWidth = (UINT)sizeof(TransformsBufferData);
    constantsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D_OK(d3dDevice->CreateBuffer(&constantsBufferDesc, nullptr, transformsBuffer.GetAddressOf()));

    ComPtr<ID3D11ShaderResourceView> spriteResourceView;
    ComPtr<ID3D11Texture2D> spriteTexture = d3d11.CreateTextureFromFile("Sprites//gem_grey_square.png", spriteResourceView.GetAddressOf());

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;

    ComPtr<ID3D11SamplerState> samplerState;
    D3D_OK(d3dDevice->CreateSamplerState(&samplerDesc, samplerState.GetAddressOf()));

    D3D11_BLEND_DESC blendStateDesc = {};
    blendStateDesc.RenderTarget[0].BlendEnable = TRUE;
    blendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendStateDesc.RenderTarget[0].RenderTargetWriteMask = 0x0f;

    ComPtr<ID3D11BlendState> blendState;
    D3D_OK(d3dDevice->CreateBlendState(&blendStateDesc, blendState.GetAddressOf()));

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

    ComPtr<ID3D11RasterizerState> rasterizerState;
    D3D_OK(d3dDevice->CreateRasterizerState(&rasterizerDesc, rasterizerState.GetAddressOf()));

    bool quit = false;
    SDL_Event event = {};
    uint32_t lastMS = 0;
    float rotationAngle = 0.0f;

    std::vector<GemsBoardChange> changes;
    std::vector<GemsBoardChange> changesToForward;

    changes.reserve(64);
    changesToForward.reserve(64);

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (SDL_QUIT == event.type)
            {
                quit = true;
                break;
            }
            else if (SDL_WINDOWEVENT == event.type)
            {
                //switch (event.window.event) 
                //{
                //case SDL_WINDOWEVENT_RESIZED:
                //    auto w = event.window.data1;
                //    auto h = event.window.data2;
                //    d3d11.OnWindowResized(w, h);
                //    break;
                //}
            }
        }

        if (quit)
            break;

        // Main Loop.
        auto currentMS = SDL_GetTicks();
        auto elapsedMS = currentMS - lastMS;
        lastMS = currentMS;

        //std::uniform_int_distribution<> rowDistribution(0, boardState.Rows);
        //std::uniform_int_distribution<> colDistribution(0, boardState.Cols);

        GemsBoardChange clearChange = {};
        clearChange.Type = ChangeType::Clear;
        clearChange.Clear.From = { 3, 3 };
        clearChange.Clear.To = { 3, 6 };

        uint32_t n = 0;

        changes.clear();
        changes.push_back(clearChange);
        changesToForward.clear();

        while (changes.size() > 0)
        {
            auto change = changes[0];
            changes.erase(changes.begin());

            auto sideEffectsStartIdx = changes.size();
            auto numSideEffects = GemsLogic::GetSideEffects(boardState, change, changes);

            auto s0 = boardState.ToString();

            boardState_1 = boardState;
            GemsLogic::ApplyChange(boardState, change, boardState_1);
            boardState = boardState_1;
            changesToForward.push_back(change);

            auto s1 = boardState.ToString();
        }

        quads.clear();
        for (int r = 0; r < boardState.Rows; r++)
        {
            for (int c = 0; c < boardState.Cols; c++)
            {
                auto color = boardState[{ r, c }];
                if (color == GemColor::None)
                    continue;

                quads.emplace_back(QuadObject {
                    kQuadSize * (boardState.Cols - 1) * ((float)c / (boardState.Cols - 1) - 0.5f),
                    kQuadSize * (boardState.Rows - 1) * ((float)r / (boardState.Rows - 1) - 0.5f),
                    0.0f,
                    0.0f,//(float)positionDistribution(generator) * XM_PI,
                    GemsConstants::Colors[(uint32_t)color]
                });
            }
        }

        int w, h;
        float viewWidth, viewHeight;

        SDL_GetWindowSize(window, &w, &h);
        
        viewWidth = w;
        viewHeight = h;

        TransformsBufferData transformsBufferData = 
        {
            DirectX::XMMatrixRotationZ(rotationAngle),
            DirectX::XMMatrixOrthographicLH(viewWidth, viewHeight, 0.0f, 1.0f)
        };

        d3d11.FrameStart(window, 0x2a2b3eff);

        const auto d3dContext = d3d11.GetDeviceContext();

        d3dContext->OMSetBlendState(blendState.Get(), nullptr, 0xffffffff);
        d3dContext->RSSetState(rasterizerState.Get());

        D3D11_MAPPED_SUBRESOURCE mappedInstanceBuffer;
        d3dContext->Map(instanceBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedInstanceBuffer);
        memcpy(mappedInstanceBuffer.pData, quads.data(), quads.size() * sizeof(QuadObject));
        d3dContext->Unmap(instanceBuffer.Get(), 0);

        d3dContext->VSSetShader(vertexShader.Get(), nullptr, 0);
        d3dContext->PSSetShader(pixelShader.Get(), nullptr, 0);
        d3dContext->IASetInputLayout(inputLayout.Get());
        d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        D3D11_MAPPED_SUBRESOURCE mappedConstantsBuffer;
        d3dContext->Map(transformsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedConstantsBuffer);
        memcpy(mappedConstantsBuffer.pData, &transformsBufferData, sizeof(transformsBufferData));
        d3dContext->Unmap(transformsBuffer.Get(), 0);

        ID3D11Buffer* constantsBuffers[] = { transformsBuffer.Get() };
        d3dContext->VSSetConstantBuffers(0, 1, constantsBuffers);

        ID3D11Buffer* buffers[] = { vertexBuffer.Get(), instanceBuffer.Get() };
        UINT strides[] = { sizeof(TriangleVertex), sizeof(QuadObject) };
        UINT offsets[] = { 0, 0 };
        const auto numBuffers = sizeof(strides) / sizeof(UINT);
        d3dContext->IASetVertexBuffers(0, numBuffers, buffers, strides, offsets);

        ID3D11ShaderResourceView* shaderResourceViews[] = { spriteResourceView.Get() };
        UINT numShaderResourceViews = sizeof(shaderResourceViews) / sizeof(ID3D11ShaderResourceView*);
        d3dContext->PSSetShaderResources(0, numShaderResourceViews, shaderResourceViews);

        ID3D11SamplerState* samplerStates[] = { samplerState.Get() };
        UINT numSamplerStates = sizeof(samplerStates) / sizeof(ID3D11SamplerState*);
        d3dContext->PSSetSamplers(0, numSamplerStates, samplerStates);

        d3dContext->DrawInstanced(6, (UINT)quads.size(), 0, 0);
        
        d3d11.FrameEnd();
    }

    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
    return 0;
}