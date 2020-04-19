#include "VsOut.hlsli"

float4 main(vs_out input) : SV_TARGET {
    return input.color; // must return an RGBA colour
}