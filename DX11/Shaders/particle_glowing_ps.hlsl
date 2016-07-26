Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);
Texture2D GMap : register(t1);
SamplerState GMapSampler : register(s1);

struct SOutput
{
	float4 color0 : SV_Target0;
	float4 color1 : SV_Target1;
};

SOutput PS( float4 posH : SV_Position, float2 o_texCoord : TEXCOORD0 ) : SV_Target
{
	float4 dif = CMap.Sample( CMapSampler, o_texCoord );
    float4 gmap = GMap.Sample( GMapSampler, o_texCoord );
	
	SOutput op;
	op.color0 = dif;
	op.color1 = gmap;
	return op;
}