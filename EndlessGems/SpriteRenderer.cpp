#include "SpriteRenderer.hpp"
#include <SDL_assert.h>

struct SpriteVertex 
{ 
    float x, y, u, v; 
};

void SpriteRenderer::Init(const Common::Direct3D11* d3d11, const std::string& shadersBasePath, uint32_t numInstances)
{
    std::vector<char> vsByteCode;
    const auto d3dDevice = d3d11->GetDevice();

    m_VertexShader = d3d11->CreateVertexShaderFromFile(shadersBasePath + "SpriteVS.cso", vsByteCode);
    m_PixelShader = d3d11->CreatePixelShaderFromFile(shadersBasePath + "SpritePS.cso");

    D3D11_INPUT_ELEMENT_DESC elementDescs[] = 
    {
        // Vertex.
        { "POSITION",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },
                                                             
        // Instance.                                         
        { "SPRITE_POS",   0, DXGI_FORMAT_R32G32_FLOAT,       1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "SPRITE_SCALE", 0, DXGI_FORMAT_R32G32_FLOAT,       1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
        { "SPRITE_TINT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };

    const auto numElements = sizeof(elementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC);

    D3D_OK(d3dDevice->CreateInputLayout(
        elementDescs, 
        numElements, 
        vsByteCode.data(), 
        vsByteCode.size(), 
        m_InputLayout.GetAddressOf()));

    SpriteVertex vertices[] = 
    {
        { -0.5f, -0.5f, 0.0f, 1.0f },
        { -0.5f,  0.5f, 0.0f, 0.0f },
        {  0.5f, -0.5f, 1.0f, 1.0f },
        {  0.5f,  0.5f, 1.0f, 0.0f }
    };

    D3D11_BUFFER_DESC vertexBufferDesc = {};
    vertexBufferDesc.ByteWidth = (UINT)sizeof(vertices);
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA initialData = {};
    initialData.pSysMem = vertices;

    D3D_OK(d3dDevice->CreateBuffer(
        &vertexBufferDesc, 
        &initialData, 
        m_QuadBuffer.GetAddressOf()));

    D3D11_BUFFER_DESC instanceBufferDesc = {};
    instanceBufferDesc.ByteWidth = (UINT)(sizeof(Sprite) * numInstances);
    instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D_OK(d3dDevice->CreateBuffer(
        &instanceBufferDesc, 
        nullptr, 
        m_InstancesBuffer.GetAddressOf()));
}

void SpriteRenderer::Begin(ID3D11DeviceContext* d3dContext)
{
    m_Sprites.clear();

    d3dContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
    d3dContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);
    d3dContext->IASetInputLayout(m_InputLayout.Get());
    d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void SpriteRenderer::Draw(Vector2 position, Vector2 scale, Color tint)
{
    Sprite sprite;
    
    sprite.Position = position;
    sprite.Scale = scale;
    sprite.Tint = tint;
    
    m_Sprites.push_back(sprite);
}

void SpriteRenderer::End(ID3D11DeviceContext* d3dContext)
{
    D3D11_MAPPED_SUBRESOURCE mappedInstanceBuffer;
    d3dContext->Map(m_InstancesBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedInstanceBuffer);
    memcpy(mappedInstanceBuffer.pData, m_Sprites.data(), m_Sprites.size() * sizeof(Sprite));
    d3dContext->Unmap(m_InstancesBuffer.Get(), 0);

    ID3D11Buffer* buffers[] = { m_QuadBuffer.Get(), m_InstancesBuffer.Get() };
    UINT strides[] = { sizeof(SpriteVertex), sizeof(Sprite) };
    UINT offsets[] = { 0, 0 };
    const auto numBuffers = sizeof(strides) / sizeof(UINT);
    d3dContext->IASetVertexBuffers(0, numBuffers, buffers, strides, offsets);

    d3dContext->DrawInstanced(4, (UINT)m_Sprites.size(), 0, 0);
}