#pragma once

#include <Direct3D11.hpp>

class SpriteRenderer
{
private:
    ComPtr<ID3D11DeviceContext> m_DeviceContext;
    ComPtr<ID3D11VertexShader> m_VertexShader;
    ComPtr<ID3D11PixelShader> m_PixelShader;
    ComPtr<ID3D11InputLayout> m_InputLayout;
    ComPtr<ID3D11Buffer> m_QuadBuffer;
    ComPtr<ID3D11Buffer> m_InstancesBuffer;

    struct InstanceData
    {
        Vector2 Position;
        Vector2 Scale;
        Color Tint;
        uint16_t SpriteId;
    };

    const static uint16_t MaxNumSpriteIds = 8;
    std::vector<InstanceData> m_SpriteInstances;

public:
    void Init(const Common::Direct3D11& d3d11, const std::string& shadersBasePath, uint32_t numInstances = 100);

    void Begin();
    void End();

    inline void Draw(Vector2 position, Vector2 scale, Color tint, uint16_t spriteId = 0)
    {
        assert(spriteId < MaxNumSpriteIds);
        m_SpriteInstances.emplace_back(position, scale, tint, spriteId);
    }
};