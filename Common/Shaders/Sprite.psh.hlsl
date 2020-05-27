#include "SpriteVSOutput.hlsli"

Texture2D sprites[8];
SamplerState spriteSampler;

float4 main(SpriteVSOutput input) : SV_TARGET 
{
    float2 uv = input.TexCoord;
    float4 tint = input.SpriteTint;
    uint id = input.SpriteId;

    switch (id)
    {
        case 1: return sprites[1].Sample(spriteSampler, uv) * tint;
        case 2: return sprites[2].Sample(spriteSampler, uv) * tint;
        case 3: return sprites[3].Sample(spriteSampler, uv) * tint;
        case 4: return sprites[4].Sample(spriteSampler, uv) * tint;
        case 5: return sprites[5].Sample(spriteSampler, uv) * tint;
        case 6: return sprites[6].Sample(spriteSampler, uv) * tint;
        case 7: return sprites[7].Sample(spriteSampler, uv) * tint;
    }

    return sprites[0].Sample(spriteSampler, uv) * tint;
}