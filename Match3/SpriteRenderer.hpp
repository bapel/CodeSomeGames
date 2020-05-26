#pragma once

#include <Direct3D11.hpp>

class SpriteRenderer
{
private:
    __declspec(align(16))
    struct InstanceData
    {
        Vector2 Position;
        Vector2 Scale;
        float Rotation_Z;
        Color32 Tint;
        uint16_t SpriteId;
    };

    struct StaticBatch
    {
        uint32_t StartInstanceLocation;
        uint32_t InstanceCount;
    };

    const static uint16_t MaxNumSpriteIds = 8;

    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_DeviceContext;
    ComPtr<ID3D11VertexShader> m_VertexShader;
    ComPtr<ID3D11PixelShader> m_PixelShader;
    ComPtr<ID3D11InputLayout> m_InputLayout;
    ComPtr<ID3D11Buffer> m_QuadBuffer;

    ComPtr<ID3D11Buffer> m_DynamicInstancesBuffer;
    std::vector<InstanceData> m_SpriteInstances;

    ComPtr<ID3D11Buffer> m_StaticInstancesBuffer;
    std::vector<StaticBatch> m_StaticBatches;

public:
    inline uint32_t GetDynamicInstanceCapacity() const
    {
        D3D11_BUFFER_DESC desc = {};
        m_DynamicInstancesBuffer->GetDesc(&desc);
        return desc.ByteWidth / sizeof(InstanceData);
    }

    inline uint32_t GetStaticInstanceCapacity() const
    {
        D3D11_BUFFER_DESC desc = {};
        m_StaticInstancesBuffer->GetDesc(&desc);
        return desc.ByteWidth / sizeof(InstanceData);
    }

    void Init(const Common::Direct3D11& d3d11, const std::string& shadersBasePath, uint32_t numInstances = 200);
    void InitInstancesBuffer(uint32_t numInstances);

    uint32_t CreateAndBeginStaticBatch();
    void FinishStaticBatch(uint32_t batchId);
    void CommitStaticBatches();

    void Begin();
    void BeginStatic();
    void EndStatic() { }
    void End();

    inline void DrawStatic(uint32_t batchId)
    {
        auto d3dContext = m_DeviceContext.Get();
        const auto& batch = m_StaticBatches[batchId];
        d3dContext->DrawInstanced(4, batch.InstanceCount, 0, batch.StartInstanceLocation);
    }

    inline void Draw(Vector2 position, Vector2 scale, Color tint, uint16_t spriteId = 0)
    {
        assert(spriteId < MaxNumSpriteIds);
        InstanceData instance = { position, scale, 0.0f, tint.BGRA(), spriteId };
        m_SpriteInstances.push_back(instance);
    }

    inline void Draw(Vector2 position, Vector2 scale, float rotationZ, Color tint, uint16_t spriteId = 0)
    {
        assert(spriteId < MaxNumSpriteIds);
        InstanceData instance = { position, scale, rotationZ, tint.BGRA(), spriteId };
        m_SpriteInstances.push_back(instance);
    }
};