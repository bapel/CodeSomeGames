/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
    float2 uv : TEXCOORD;
    float4 color : COLOR;
};

struct SpriteVSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    float4 SpriteTint : COLOR;
};