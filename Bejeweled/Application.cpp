#include <SDL.h>
#include <SDL_assert.h>
#include <SDL_log.h>
#include <SDL_timer.h>

#include "Direct3D11.hpp"
#include <DirectXMath.h>

#include <stdio.h>
#include <string>
#include <vector>

struct TransformsBuffer
{
    DirectXMath::XMMatr
};

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
    ComPtr<ID3D11Buffer> constantsBuffer;

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
        { -0.5f, -0.5f, 0.0f, 1.0f },
        { -0.5f,  0.5f, 0.0f, 1.0f },
        {  0.5f,  0.5f, 0.0f, 1.0f },
        {  0.5f,  0.5f, 0.0f, 1.0f },
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

    D3D11_BUFFER_DESC constantsBufferDesc = {};
    constantsBufferDesc.ByteWidth = sizeof(float) * 32;
    constantsBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    constantsBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    constantsBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D_OK(d3dDevice->CreateBuffer(&constantsBufferDesc, nullptr, constantsBuffer.GetAddressOf()));

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

        rotationAngle += 0.1f * ((float)elapsedMS / 1000.0f);

        DirectX::XMMATRIX viewproj = DirectX::XMMatrixOrthographicLH(1.0f, 1.0f, 0.0f, 1.0f);
        DirectX::XMMATRIX transform = DirectX::XMMatrixRotationZ(rotationAngle);

        d3d11.FrameStart(window);

        const auto d3dContext = d3d11.GetDeviceContext();
        d3dContext->VSSetShader(vertexShader.Get(), nullptr, 0);
        d3dContext->PSSetShader(pixelShader.Get(), nullptr, 0);
        d3dContext->IASetInputLayout(inputLayout.Get());
        d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        UINT strides[] = { sizeof(TriangleVertex) };
        UINT offsets[] = { 0 };
        d3dContext->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), strides, offsets);

        D3D11_MAPPED_SUBRESOURCE mappedConstantsBuffer;
        d3dContext->Map(constantsBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedConstantsBuffer);
        memcpy(mappedConstantsBuffer.pData, &transform, sizeof(transform));
        d3dContext->Unmap(constantsBuffer.Get(), 0);

        ID3D11Buffer* constantsBuffers[] = { constantsBuffer.Get() };
        d3dContext->VSSetConstantBuffers(0, 1, constantsBuffers);

        d3dContext->Draw(6, 0);
        
        d3d11.FrameEnd();
    }

    SDL_DestroyWindow(window);
    return 0;
}