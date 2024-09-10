cbuffer b0 : register( b0 ) {
    row_major matrix Proj;
    row_major matrix View;
	float3 LightPos;
};
cbuffer b1 : register( b1 ) {
    row_major matrix World;
};
cbuffer b2 : register( b2 ) {
    float4 Diffuse;
    float4 Ambient;
    float4 Specular;
};
cbuffer b3 : register( b3 ){ 
	row_major matrix WorldArray[2]       : packoffset(c0);
	row_major matrix WorldArrayDmmy[253] : packoffset(c8);//4*2
	row_major matrix WorldArrayEnd       : packoffset(c1020);//4*(2+253)
}
Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);
