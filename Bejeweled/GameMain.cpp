#include <SDL.h>
#include <SDL_assert.h>
#include <SDL_log.h>

#include "Direct3D11.hpp"

#include <stdio.h>
#include <string>
#include <vector>

int main(int argc, char** argv)
{
    SDL_assert(argc >= 1);
    const std::string kExePath(argv[0]);
    size_t offset = kExePath.find_last_of("\\");
    const std::string kBasePath = kExePath.substr(0, offset + 1);

    SDL_assert(0 == SDL_Init(0));

    SDL_Window* window = SDL_CreateWindow(
        "Bejeweled Clone",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1024, 768,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN);

    SDL_assert(nullptr != window);

    Direct3D11 d3d11;
    ComPtr<ID3D11VertexShader> vertexShader;
    ComPtr<ID3D11PixelShader> pixelShader;
    ComPtr<ID3D11InputLayout> inputLayout;
    ComPtr<ID3D11Buffer> vertexBuffer;

    d3d11.Init(window);

    std::vector<char> vsByteCode;
    vertexShader = d3d11.LoadVertexShader(kBasePath + "TriangleVertex.cso", vsByteCode);
    pixelShader = d3d11.LoadPixelShader(kBasePath + "TrianglePixel.cso");

    const auto d3dDevice = d3d11.GetDevice();

    D3D11_INPUT_ELEMENT_DESC elementDescs[] = {
        { "POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };

    D3D_OK(d3dDevice->CreateInputLayout(elementDescs, 1, vsByteCode.data(), vsByteCode.size(), inputLayout.GetAddressOf()));
    
    struct TriangleVertex
    {
        float x, y, z, w;
    };

    TriangleVertex vertices[] = 
    {
        {  0.0f,  0.5f, 0.0f, 1.0f },
        {  0.5f, -0.5f, 0.0f, 1.0f },
        { -0.5f, -0.5f, 0.0f, 1.0f }
    };

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = sizeof(vertices);
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = vertices;

    D3D_OK(d3dDevice->CreateBuffer(&vertexBufferDesc, &initialData, vertexBuffer.GetAddressOf()));

    bool quit = false;
    SDL_Event event = {};

    while (!quit)
    {
        while (SDL_PollEvent(&event))
        {
            if (SDL_QUIT == event.type)
            {
                quit = true;
                break;
            }
        }

        if (quit)
            break;

        // Main Loop.
        d3d11.FrameStart(window);

        const auto context = d3d11.GetDeviceContext();
        context->VSSetShader(vertexShader.Get(), nullptr, 0);
        context->PSSetShader(pixelShader.Get(), nullptr, 0);
        context->IASetInputLayout(inputLayout.Get());
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        UINT strides[] = { sizeof(TriangleVertex) };
        UINT offsets[] = { 0 };
        context->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), strides, offsets);

        context->Draw(3, 0);
        
        d3d11.FrameEnd();
    }

    SDL_DestroyWindow(window);
    return 0;
}