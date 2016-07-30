Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);
Texture2D GMap : register(t1);
SamplerState GMapSampler : register(s1);

struct SOutput
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};

struct SVsOut
{
	float4 position : SV_Position;
	float2 texCoord : TEXCOORD;
    float3 color : COLOR;
};

SOutput PS( SVsOut vertex ) : SV_Target
{
	float4 dif = CMap.Sample( CMapSampler, vertex.texCoord );
	float3 color = (vertex.color * dif.rgb) * dif.a;
	
	SOutput op;
	op.color1 = op.color0 = float4( color, 1 );
	return op;
}