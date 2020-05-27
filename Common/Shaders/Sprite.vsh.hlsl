#include "SpriteVSOutput.hlsli"

matrix rotation_z(float angle)
{
    float fCosAngle = cos(angle);
    float fSinAngle = sin(angle);

    matrix M;
    M[0][0] = fCosAngle;
    M[0][1] = fSinAngle;
    M[0][2] = 0.0f;
    M[0][3] = 0.0f;

    M[1][0] = -fSinAngle;
    M[1][1] = fCosAngle;
    M[1][2] = 0.0f;
    M[1][3] = 0.0f;

    M[2][0] = 0.0f;
    M[2][1] = 0.0f;
    M[2][2] = 1.0f;
    M[2][3] = 0.0f;

    M[3][0] = 0.0f;
    M[3][1] = 0.0f;
    M[3][2] = 0.0f;
    M[3][3] = 1.0f;
    return M;
}

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
    float SpriteRotation : SPRITE_ZROT;
    float4 SpriteTint : SPRITE_TINT;
    uint SpriteId : SPRITE_ID;
};

SpriteVSOutput main(SpriteVSInput input) 
{
    float4 outPosition = float4(0.0f, 0.0f, 0.0f, 1.0f);

    outPosition.xy = (input.SpriteScale * input.Position);
    outPosition = mul(rotation_z(input.SpriteRotation), outPosition);
    outPosition.xy += input.SpritePos;
    outPosition = mul(ViewProjection, outPosition);

    SpriteVSOutput output;

    output.Position = outPosition;
    output.TexCoord = input.TexCoord;
    output.SpriteTint = input.SpriteTint;
    output.SpriteId = input.SpriteId;

    return output;
}