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
    float4 color : COLOR;
};

SOutput PS( SVsOut vertex ) : SV_Target
{
	float4 dif = CMap.Sample( CMapSampler, vertex.texCoord );
	
	SOutput op;
	op.color1 = op.color0 = dif * vertex.color;
	return op;
}