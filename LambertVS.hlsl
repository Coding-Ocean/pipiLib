#include"Root.hlsli"

struct IN {
    float4 pos    : POSITION;
	float3 normal : NORMAL;
    float2 tex    : TEXCOORD;
};

struct OUT {
    float4 pos     : SV_POSITION;
	float4 diffuse : COLOR;
    float2 tex     : TEXCOORD;
};

OUT main( IN i ) {
    OUT o = (OUT)0;

	o.pos = mul( World, i.pos );
	o.pos = mul( View, o.pos );
    o.pos = mul( Proj, o.pos);

    float3 normal = normalize( mul( (float3x3) World, i.normal ) );
    o.diffuse = Diffuse * max(dot(normal, LightPos), Ambient );

	o.tex = i.tex;
    
    return o;
}
