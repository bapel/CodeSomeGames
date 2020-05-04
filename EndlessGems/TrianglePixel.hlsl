#include "VsOut.hlsli"

Texture2D spriteTexture;
SamplerState spriteSampler;

float4 main(vs_out input) : SV_TARGET {
    input.uv.y = 1.0 - input.uv.y;
    float4 color = spriteTexture.Sample(spriteSampler, input.uv);
    return color * input.color;//float4(input.uv, 0.0, 1.0);// + input.color; // must return an RGBA colour
}