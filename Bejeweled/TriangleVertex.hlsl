#include "VsOut.hlsli"

cbuffer TransformsBuffer : register(b0)
{
    matrix model;
    matrix viewProj;
}

/* vertex attributes go here to input to the vertex shader */
struct vs_in {
    float4 position_local : POSITION;
    float2 uv : TEXCOORD;
    float4 quad_data : QUAD_DATA;
    float4 color : COLOR;
};

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

vs_out main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory first
    matrix r = rotation_z(input.quad_data.w);
    float4 rotated_position = mul(float4(input.position_local), r);
    output.position_clip = mul(rotated_position + float4(input.quad_data.xyz, 0.0), viewProj);
    output.uv = input.uv;
    output.color = input.color;
    return output;
}