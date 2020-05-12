#include "VsOut.hlsli"

cbuffer TransformsBuffer : register(b0)
{
    matrix Model;
    matrix ViewProjection;
}

/* vertex attributes go here to input to the vertex shader */
struct SpriteVSInput
{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD;
    float2 SpritePos : SPRITE_POS;
    float2 SpriteScale : SPRITE_SCALE;
    float4 SpriteTint : SPRITE_TINT;
};

SpriteVSOutput main(SpriteVSInput input) 
{
    float2 outPosition = (input.SpriteScale * input.Position) + input.SpritePos;

    SpriteVSOutput output;

    output.Position = mul(ViewProjection, float4(outPosition, 0.0, 1.0));
    output.TexCoord = input.TexCoord;
    output.SpriteTint = input.SpriteTint;

    return output;
}