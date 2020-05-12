#include "VsOut.hlsli"

Texture2D spriteTexture;
SamplerState spriteSampler;

float4 main(SpriteVSOutput input) : SV_TARGET 
{
    float2 uv = input.TexCoord;
    float4 tint = input.SpriteTint;

    return spriteTexture.Sample(spriteSampler, uv) * tint;
}