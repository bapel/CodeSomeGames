struct SpriteVSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD;
    nointerpolation float4 SpriteTint : COLOR0;
    nointerpolation uint SpriteId: COLOR1;
};