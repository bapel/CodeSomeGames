uniform float4x4 transform;

/* vertex attributes go here to input to the vertex shader */
struct vs_in {
    float3 position_local : POS;
};

/* outputs from vertex shader go here. can be interpolated to pixel shader */
struct vs_out {
    float4 position_clip : SV_POSITION; // required output of VS
};

vs_out main(vs_in input) {
    vs_out output = (vs_out)0; // zero the memory first
    output.position_clip = mul(transform, float4(input.position_local, 1.0));
    return output;
}