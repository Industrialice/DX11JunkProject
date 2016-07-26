Texture2D CMap : register(t0);
SamplerState CMapSampler : register(s0);

float4 PS( float4 posH : SV_Position, float2 o_texCoord : TEXCOORD0 ) : SV_Target
{
	return CMap.Sample( CMapSampler, o_texCoord );
}