#include"Root.hlsli"

struct IN {
    float4 pos     : SV_POSITION;
	float4 diffuse : COLOR;
    float2 tex     : TEXCOORD;
};

float4 main( IN i ) : SV_TARGET {
    return Texture.Sample(Sampler, i.tex) * i.diffuse;
}
