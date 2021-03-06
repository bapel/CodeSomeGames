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

#define AppendElem__ D3D11_APPEND_ALIGNED_ELEMENT

const D3D11_INPUT_ELEMENT_DESC spriteElementDescs[] = 
{
    // Vertex.
    { "POSITION",     0, DXGI_FORMAT_R32G32_FLOAT,   0, AppendElem__, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,   0, AppendElem__, D3D11_INPUT_PER_VERTEX_DATA, 0 },

    // Instance.                                     
    { "SPRITE_POS",   0, DXGI_FORMAT_R32G32_FLOAT,   1, AppendElem__, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "SPRITE_SCALE", 0, DXGI_FORMAT_R32G32_FLOAT,   1, AppendElem__, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "SPRITE_ZROT",  0, DXGI_FORMAT_R32_FLOAT,      1, AppendElem__, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "SPRITE_TINT",  0, DXGI_FORMAT_B8G8R8A8_UNORM, 1, AppendElem__, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
    { "SPRITE_ID",    0, DXGI_FORMAT_R16_UINT,       1, AppendElem__, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
};

void SpriteRenderer::Init(const Common::Direct3D11& d3d11, const std::string& shadersBasePath, uint32_t numMaxSprites)
{
    std::vector<char> vsByteCode;

    m_Device = d3d11.GetDevice();
    m_DeviceContext = d3d11.GetDeviceContext();
    m_VertexShader = d3d11.CreateVertexShaderFromFile(shadersBasePath + "Sprite.vsh.cso", vsByteCode);
    m_PixelShader = d3d11.CreatePixelShaderFromFile(shadersBasePath + "Sprite.psh.cso");

    InitInstancesBuffer(numMaxSprites);

    const auto d3dDevice = d3d11.GetDevice();

    Direct3D_Ok__(d3dDevice->CreateInputLayout(
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

    Direct3D_Ok__(d3dDevice->CreateBuffer(
        &vertexBufferDesc, 
        &initialData, 
        m_QuadBuffer.GetAddressOf()));
}

void SpriteRenderer::InitInstancesBuffer(uint32_t numInstances)
{
    D3D11_BUFFER_DESC instanceBufferDesc = {};
    instanceBufferDesc.ByteWidth = (UINT)(sizeof(InstanceData) * numInstances);
    instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    Direct3D_Ok__(m_Device->CreateBuffer(
        &instanceBufferDesc, nullptr,
        m_DynamicInstancesBuffer.GetAddressOf()));
}

uint32_t SpriteRenderer::CreateAndBeginStaticBatch()
{
    if (m_StaticBatches.size() == 0)
        assert(m_SpriteInstances.size() == 0);

    StaticBatch batch = { (UINT)m_SpriteInstances.size(), 0 };
    m_StaticBatches.emplace_back(batch);

    return (UINT)m_StaticBatches.size() - 1U;
}

void SpriteRenderer::FinishStaticBatch(uint32_t batchId)
{
    assert(batchId == m_StaticBatches.size() - 1);

    auto& lastBatch = m_StaticBatches[m_StaticBatches.size() - 1];
    lastBatch.InstanceCount = (UINT)m_SpriteInstances.size() - lastBatch.StartInstanceLocation;
}

void SpriteRenderer::CommitStaticBatches()
{
    assert(m_StaticInstancesBuffer == nullptr);

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = (UINT)(m_SpriteInstances.size() * sizeof(InstanceData));
    desc.Usage = D3D11_USAGE_IMMUTABLE;
    desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA data = {};
    data.pSysMem = m_SpriteInstances.data();

    Direct3D_Ok__(m_Device->CreateBuffer(&desc, &data, m_StaticInstancesBuffer.GetAddressOf()));

    m_SpriteInstances.clear();
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

void SpriteRenderer::BeginStatic()
{
    auto d3dContext = m_DeviceContext.Get();

    ID3D11Buffer* buffers[] = { m_QuadBuffer.Get(), m_StaticInstancesBuffer.Get() };
    UINT strides[] = { sizeof(SpriteVertex), sizeof(InstanceData) };
    UINT offsets[] = { 0, 0 };
    const auto numBuffers = sizeof(strides) / sizeof(UINT);
    d3dContext->IASetVertexBuffers(0, numBuffers, buffers, strides, offsets);
}

void SpriteRenderer::End()
{
    if (m_SpriteInstances.empty())
        return;

    if (m_SpriteInstances.size() > GetDynamicInstanceCapacity())
    {
        m_DynamicInstancesBuffer->Release();
        InitInstancesBuffer((UINT)m_SpriteInstances.size());
    }

    auto d3dContext = m_DeviceContext.Get();

    D3D11_MAPPED_SUBRESOURCE mappedInstanceBuffer;
    d3dContext->Map(m_DynamicInstancesBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedInstanceBuffer);
    memcpy(mappedInstanceBuffer.pData, m_SpriteInstances.data(), m_SpriteInstances.size() * sizeof(InstanceData));
    d3dContext->Unmap(m_DynamicInstancesBuffer.Get(), 0);

    ID3D11Buffer* buffers[] = { m_QuadBuffer.Get(), m_DynamicInstancesBuffer.Get() };
    UINT strides[] = { sizeof(SpriteVertex), sizeof(InstanceData) };
    UINT offsets[] = { 0, 0 };
    const auto numBuffers = sizeof(strides) / sizeof(UINT);
    d3dContext->IASetVertexBuffers(0, numBuffers, buffers, strides, offsets);

    d3dContext->DrawInstanced(4, (UINT)m_SpriteInstances.size(), 0, 0);
}