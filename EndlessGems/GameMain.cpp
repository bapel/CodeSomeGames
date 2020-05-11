#include <SDL.h>
#include <SDL_assert.h>
#include <SDL_log.h>
#include <SDL_timer.h>
#include <SDL_image.h>

#include <Direct3D11.hpp>
#include <SimpleMath.hpp>

#include <stdio.h>
#include <string>
#include <vector>
#include <random>
#include <ctime>
#include <queue>

#include "SimpleMath.h"
using Vector2 = DirectX::SimpleMath::Vector2;

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
    Matrix model;
    Matrix viewProj;
};

const auto kQuadSize = 2 * 32.0f;

//float GemColToWorld(int c)
//{
//    return kQuadSize * (boardState.Cols - 1) * ((float)c / (boardState.Cols - 1) - 0.5f);
//}
//
//float GemRowToWorld(int r)
//{
//    return kQuadSize * (boardState.Rows - 1) * ((float)r / (boardState.Rows - 1) - 0.5f);
//}

#include "Game\Logic.hpp"

std::vector<Game::GemLocation::Coord> Game::Logic::m_RowsBuffer;

int main(int argc, char** argv)
{
    using namespace Game;

    Board board;
    BoardChangeQueue changeQueue;

    Logic::m_RowsBuffer.reserve(board.Count());

    std::mt19937 g(0);
    std::uniform_int_distribution<> d(1, (int)Game::GemColor::Count - 1);

    for (auto r = 0; r < board.Rows(); r++)
    {
        for (auto c = 0; c < board.Cols(); c++)
        {
            auto spawn = BoardChange();
            spawn.Type = BoardChangeType::Spawn;
            spawn.Spawn.Location = { r, c };
            spawn.Spawn.Color = (Game::GemColor)d(g);

            changeQueue.PushBack(spawn);
        }
    }

    while (changeQueue.NumRemaining() > 0)
        Logic::ProcessQueue(&board, &changeQueue);

    changeQueue.Clear();

    auto boardStr = board.ToString();
    printf("%s\n", boardStr.c_str());

    {
        auto clear = BoardChange();
        clear.Type = BoardChangeType::Clear;

        clear.Clear.ID = board[{ 2, 1 }].ID;
        changeQueue.PushBack(clear);

        clear.Clear.ID = board[{ 2, 2 }].ID;
        changeQueue.PushBack(clear);

        clear.Clear.ID = board[{ 2, 3 }].ID;
        changeQueue.PushBack(clear);
    }

    while (changeQueue.NumRemaining() > 0)
        Logic::ProcessQueue(&board, &changeQueue);

    boardStr = board.ToString();
    printf("%s\n", boardStr.c_str());

    {
        auto clear = BoardChange();
        clear.Type = BoardChangeType::Clear;

        clear.Clear.ID = board[{ 2, 2 }].ID;
        changeQueue.PushBack(clear);

        clear.Clear.ID = board[{ 3, 2 }].ID;
        changeQueue.PushBack(clear);

        clear.Clear.ID = board[{ 4, 2 }].ID;
        changeQueue.PushBack(clear);
    }

    Logic::ProcessQueue(&board, &changeQueue);

    while (changeQueue.NumRemaining() > 0)
        Logic::ProcessQueue(&board, &changeQueue);

    boardStr = board.ToString();
    printf("%s\n", boardStr.c_str());

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

    std::vector<QuadObject> quads;
    quads.reserve(64);
    
    Common::Direct3D11 d3d11;
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

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (SDL_QUIT == event.type)
            {
                quit = true;
                break;
            }
            else if (SDL_KEYDOWN == event.type)
            {
                switch (event.key.keysym.sym)
                {
                    case SDLK_SPACE:
                    {
                        break;
                    }
                }
            }
        }

        if (quit)
            break;

        // Main Loop.
        auto currentMS = SDL_GetTicks();
        auto elapsedMS = currentMS - lastMS;
        lastMS = currentMS;

        int w, h;
        float viewWidth, viewHeight;

        SDL_GetWindowSize(window, &w, &h);
        
        viewWidth = w;
        viewHeight = h;

        TransformsBufferData transformsBufferData = 
        {
            Matrix::CreateRotationZ(rotationAngle),
            Matrix::CreateOrthographic(viewWidth, viewHeight, 0.0f, 1.0f)
        };

        d3d11.FrameStart(window, Color(0x2a2b3eff));

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