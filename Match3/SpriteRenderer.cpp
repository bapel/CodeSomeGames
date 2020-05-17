#include "SpriteRenderer.hpp"

struct SpriteVertex 
{ 
    Vector2 Position;
    Vector2 TexCoord;
};

const SpriteVertex vertices[] = 
{
    { { -0.5f, -0.5f, }, { 0.0f, 1.0f } },
    { { -0.5f,  0.5f, }, { 0.0f, 0.0f } },
    { {  0.5f, -0.5f, }, { 1.0f, 1.0f } },
    { {  0.5f,  0.5f, }, { 1.0f, 0.0f } }
};

const D3D11_INPUT_ELEMENT_DESC spriteElementDescs[] = 
{
    // Vertex.
    { "POSITION",     0, DXGI_FORMAT_R32G32_FLOAT,       0, 0, D3D11_INPUT_PER_VERTEX_DATA,   0 },
    { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA,   0 },

    // Instance.                                         
    { "SPRITE_POS",   0, DXGI_FORMAT_R32G32_FLOAT,       1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "SPRITE_SCALE", 0, DXGI_FORMAT_R32G32_FLOAT,       1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "SPRITE_TINT",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
};

void SpriteRenderer::Init(const Common::Direct3D11& d3d11, const std::string& shadersBasePath, uint32_t numInstances)
{
    std::vector<char> vsByteCode;

    m_DeviceContext = d3d11.GetDeviceContext();
    m_VertexShader = d3d11.CreateVertexShaderFromFile(shadersBasePath + "SpriteVS.cso", vsByteCode);
    m_PixelShader = d3d11.CreatePixelShaderFromFile(shadersBasePath + "SpritePS.cso");

    const auto d3dDevice = d3d11.GetDevice();

    D3D_OK(d3dDevice->CreateInputLayout(
        spriteElementDescs, 
        sizeof(spriteElementDescs) / sizeof(D3D11_INPUT_ELEMENT_DESC), 
        vsByteCode.data(), 
        vsByteCode.size(), 
        m_InputLayout.GetAddressOf()));

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
    instanceBufferDesc.ByteWidth = (UINT)(sizeof(InstanceData) * numInstances);
    instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D3D_OK(d3dDevice->CreateBuffer(
        &instanceBufferDesc, 
        nullptr, 
        m_InstancesBuffer.GetAddressOf()));
}

void SpriteRenderer::Begin()
{
    auto d3dContext = m_DeviceContext.Get();

    m_SpriteInstances.clear();

    d3dContext->VSSetShader(m_VertexShader.Get(), nullptr, 0);
    d3dContext->PSSetShader(m_PixelShader.Get(), nullptr, 0);
    d3dContext->IASetInputLayout(m_InputLayout.Get());
    d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void SpriteRenderer::SetSampler(const ComPtr<ID3D11SamplerState> sampler)
{
    auto d3dContext = m_DeviceContext.Get();
    ID3D11SamplerState* samplerStates[] = { sampler.Get() };
    UINT numSamplerStates = sizeof(samplerStates) / sizeof(ID3D11SamplerState*);
    d3dContext->PSSetSamplers(0, numSamplerStates, samplerStates);
}

void SpriteRenderer::Draw(Vector2 position, Vector2 scale, Color tint)
{
    InstanceData sprite;

    sprite.Position = position;
    sprite.Scale = scale;
    sprite.Tint = tint;

    m_SpriteInstances.push_back(sprite);
}

void SpriteRenderer::End()
{
    if (m_SpriteInstances.empty())
        return;

    auto d3dContext = m_DeviceContext.Get();

    D3D11_MAPPED_SUBRESOURCE mappedInstanceBuffer;
    d3dContext->Map(m_InstancesBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedInstanceBuffer);
    memcpy(mappedInstanceBuffer.pData, m_SpriteInstances.data(), m_SpriteInstances.size() * sizeof(InstanceData));
    d3dContext->Unmap(m_InstancesBuffer.Get(), 0);

    ID3D11Buffer* buffers[] = { m_QuadBuffer.Get(), m_InstancesBuffer.Get() };
    UINT strides[] = { sizeof(SpriteVertex), sizeof(InstanceData) };
    UINT offsets[] = { 0, 0 };
    const auto numBuffers = sizeof(strides) / sizeof(UINT);
    d3dContext->IASetVertexBuffers(0, numBuffers, buffers, strides, offsets);

    d3dContext->DrawInstanced(4, (UINT)m_SpriteInstances.size(), 0, 0);
}