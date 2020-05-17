#pragma once

#include <Direct3D11.hpp>

struct SpriteInstanceData;

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
    };

    std::vector<InstanceData> m_SpriteInstances;

public:
    void Init(const Common::Direct3D11& d3d11, const std::string& shadersBasePath, uint32_t numInstances = 100);

    void Begin();
    void SetSampler(const ComPtr<ID3D11SamplerState> sampler);
    void Draw(Vector2 position, Vector2 scale, Color tint);
    void End();
};