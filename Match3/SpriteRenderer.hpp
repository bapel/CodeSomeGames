#pragma once

#include <Direct3D11.hpp>

class SpriteRenderer
{
private:
    ComPtr<ID3D11Device> m_Device;
    ComPtr<ID3D11DeviceContext> m_DeviceContext;
    ComPtr<ID3D11VertexShader> m_VertexShader;
    ComPtr<ID3D11PixelShader> m_PixelShader;
    ComPtr<ID3D11InputLayout> m_InputLayout;
    ComPtr<ID3D11Buffer> m_QuadBuffer;
    ComPtr<ID3D11Buffer> m_InstancesBuffer;

    __declspec(align(16))
    struct InstanceData
    {
        Vector2 Position;
        Vector2 Scale;
        float Rotation_Z;
        Color32 Tint;
        uint16_t SpriteId;
    };

    const static uint16_t MaxNumSpriteIds = 8;
    std::vector<InstanceData> m_SpriteInstances;
    size_t m_MaxSprites = 100;

public:
    void Init(const Common::Direct3D11& d3d11, const std::string& shadersBasePath, size_t numInstances = 200);
    void InitInstancesBuffer(size_t numInstances);

    void Begin();
    void End();

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